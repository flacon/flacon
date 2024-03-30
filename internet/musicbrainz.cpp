/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "musicbrainz.h"
#include <QDebug>
#include "../disc.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "MusicBrainzProvider")
}

/* Algorithm ************************************
I couldn't get the data in one request, so I'm using 2 steps.

Step 1:
  Requesting a list of release groups by artist name and album title.
  For each group, we take the ID and perform step 2

Step 2:
  Requesting a list of releases included in the group.
  Each release contains metadata including track titles.

Postprocessing:
  Since the disc is often released several times, but contains the same tracks,
  our results will contain duplicates. We delete the duplicates.

-------------------------------------------------
Example
  1) https://musicbrainz.org/ws/2/release-group/?fmt=json&query=artist:%22Aerosmith%22%20AND%20release:%22Permanent%20Vacation%22
  2) https://musicbrainz.org/ws/2/release/?fmt=json&inc=recordings+genres+artist-credits&release-group=c522e905-3986-3f09-89f3-5d8bb09cc93e


Documentation:
  https://musicbrainz.org/doc/MusicBrainz_API
  https://musicbrainz.org/doc/MusicBrainz_API/Search
*************************************************/

static constexpr auto SEARCH_URL = "https://musicbrainz.org/ws/2/release-group/?fmt=json&query=%1";
static constexpr auto LOOKUP_URL = "https://musicbrainz.org/ws/2/release/?fmt=json&inc=recordings+genres+artist-credits&release-group=%1";

/************************************************

 ************************************************/
bool MusicBrainz::canDownload(const Disc &disk)
{
    if (disk.isEmpty()) {
        return false;
    }

    const Track *track = disk.track(0);

    return !track->artist().isEmpty() && !track->album().isEmpty();
}

/************************************************

 ************************************************/
void MusicBrainz::start()
{
    if (mDisk.isEmpty()) {
        return;
    }

    const Track *track = mDisk.track(0);
    mRequestArtist     = track->artist();
    mRequestAlbum      = track->album();

    QStringList query;
    query << QString("artist:\"%1\"").arg(mRequestArtist.toHtmlEscaped());
    query << QString("release:\"%1\"").arg(mRequestAlbum.toHtmlEscaped());

    QUrl url = QString(SEARCH_URL).arg(query.join("%20AND%20"));

    QNetworkRequest request(url);

    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { releaseGroupsReady(reply); });
}

/************************************************

 ************************************************/
void MusicBrainz::releaseGroupsReady(QNetworkReply *reply)
{
    if (reply->error()) {
        QString err = reply->errorString();
        qCWarning(LOG) << err;
        error(err);
        emit finished({});
        return;
    }

    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(LOG) << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        error(parseError.errorString());
        emit finished({});
        return;
    }

    QJsonArray releaseGroups = doc["release-groups"].toArray();
    if (releaseGroups.isEmpty()) {
        emit finished({});
        return;
    }

    for (const QJsonValue r : releaseGroups) {
        QString id = r["id"].toString();
        if (id.isEmpty()) {
            continue;
        }

        if (r["title"].toString().toUpper() != mRequestAlbum.toUpper()) {
            continue;
        }

        QNetworkReply *reply = get(QNetworkRequest(QString(LOOKUP_URL).arg(id)));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() { releasesReady(reply); });
    }

    if (isFinished()) {
        emit finished({});
    }
}

/************************************************

 ************************************************/
void MusicBrainz::releasesReady(QNetworkReply *reply)
{
    if (reply->error()) {
        QString err = reply->errorString();
        qCWarning(LOG) << err;
        error(err);
        emit finished({});
        return;
    }

    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(LOG) << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        error(parseError.errorString());
        emit finished({});
        return;
    }

    QJsonArray releases = doc["releases"].toArray();
    if (releases.isEmpty()) {
        emit finished({});
        return;
    }

    QVector<Tracks> res;

    for (const QJsonValue r : releases) {
        QString album = r["title"].toString();

        for (const QJsonValue m : r["media"].toArray()) {
            if (m["track-count"].toInt() != mDisk.count()) {
                continue;
            }

            Tracks tracks = parseTracksJson(m["tracks"].toArray(), album);
            if (!tracks.isEmpty()) {
                res << tracks;
            }
        }
    }

    mResult << res;

    if (isFinished()) {
        processResults();
    }
}

/************************************************

 ************************************************/
QString getGenre(const QJsonValue &track)
{
    QString res;

    QJsonArray genreList = track["recording"]["genres"].toArray();
    int        cnt       = 0;
    for (const QJsonValue json : genreList) {
        if (json["count"].toInt() > cnt) {
            cnt = json["count"].toInt();
            res = json["name"].toString();
        }
    }

    return res;
}

/************************************************

 ************************************************/
QString getDate(const QJsonValue &track)
{
    QList<QRegExp> patterns;
    patterns << QRegExp("(\\d\\d\\d\\d)", Qt::CaseInsensitive, QRegExp::RegExp2);
    patterns << QRegExp("(\\d\\d\\d\\d)-\\d\\d-\\d\\d", Qt::CaseInsensitive, QRegExp::RegExp2);

    QString s = track["recording"]["first-release-date"].toString();
    foreach (const QRegExp &re, patterns) {
        if (re.exactMatch(s)) {
            return re.cap(1);
        }
    }

    return "";
}

/************************************************

 ************************************************/
Tracks MusicBrainz::parseTracksJson(const QJsonArray &tracks, const QString &album)
{
    Tracks res;
    res.resize(tracks.count());

    QJsonArray genreList = tracks[0].toObject()["genres"].toArray();

    int n = 0;
    for (const QJsonValue t : tracks) {

        int pos = t["position"].toInt(-1);
        if (pos < 0) {
            return {};
        }

        QString trackTitle = t["title"].toString();
        if (trackTitle.isEmpty()) {
            return {};
        }

        QString artist = t["artist-credit"][0]["name"].toString();

        Track &track = res[n++];
        track.setCodec(TextCodecUtf8());
        track.setTag(TagId::Date, getDate(t));
        track.setTag(TagId::Album, album);
        track.setTag(TagId::Artist, artist);
        track.setTag(TagId::Title, trackTitle);
        track.setTag(TagId::Genre, getGenre(t));

        track.setDiscCount(mDisk.discCount());
        track.setDiscNum(mDisk.discNum());
        track.setTrackNum(n);
        track.setTrackCount(tracks.size());

        track.setAlbumArtist(artist);
        track.setTag(TagId::SongWriter, artist);
    }

    return res;
}

/************************************************

 ************************************************/
void MusicBrainz::processResults()
{
    removeDuplicates();

    int n = 0;
    for (Tracks &t : mResult) {
        n++;
        t.setUri(QString("https://musicbrainz.org/artis=%1&album=%2&num=%3").arg(mRequestArtist, mRequestAlbum).arg(n));
        if (mResult.size() == 1) {
            t.setTitle(QString("%1 / %2   [ MusicBrainz ]").arg(mRequestArtist, mRequestAlbum));
        }
        else {
            t.setTitle(QString("%1 / %2   [ MusicBrainz %3 ]").arg(mRequestArtist, mRequestAlbum).arg(n));
        }
    }

    emit finished(mResult);
}
