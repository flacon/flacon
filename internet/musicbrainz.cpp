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
#include <QRegularExpression>

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

    return !disk.performerTag().isEmpty() && !disk.albumTag().isEmpty();
}

/************************************************

 ************************************************/
void MusicBrainz::start()
{
    if (mDisk.isEmpty()) {
        return;
    }

    mRequestArtist = mDisk.performerTag();
    mRequestAlbum  = mDisk.albumTag();

    QStringList query;
    query << QStringLiteral("artist:\"%1\"").arg(mRequestArtist.toHtmlEscaped());
    query << QStringLiteral("release:\"%1\"").arg(mRequestAlbum.toHtmlEscaped());

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

    QVector<InternetTags> res;

    for (const QJsonValue r : releases) {
        QString album = r["title"].toString();

        for (const QJsonValue m : r["media"].toArray()) {
            if (m["track-count"].toInt() != mDisk.tracks().count()) {
                continue;
            }

            InternetTags tags = parseTracksJson(m["tracks"].toArray(), album);
            if (!tags.isEmpty()) {
                res << tags;
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
static QString getGenre(const QJsonValue &track)
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
static QString getDate(const QJsonValue &track)
{
    static const QList<QRegularExpression> patterns = {
        QRegularExpression(QRegularExpression::anchoredPattern("(\\d\\d\\d\\d)"), QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QRegularExpression::anchoredPattern("(\\d\\d\\d\\d)-\\d\\d-\\d\\d"), QRegularExpression::CaseInsensitiveOption),
    };

    QString s = track["recording"]["first-release-date"].toString();
    foreach (const QRegularExpression &re, patterns) {
        QRegularExpressionMatch match = re.match(s);

        if (match.hasMatch()) {
            return match.captured(1);
        }
    }

    return "";
}

/************************************************

 ************************************************/
InternetTags MusicBrainz::parseTracksJson(const QJsonArray &tracks, const QString &album)
{
    InternetTags res;
    res.tracks().resize(tracks.count());

    int n = -1;
    for (const QJsonValue &t : tracks) {
        n++;

        int pos = t["position"].toInt(-1);
        if (pos < 0) {
            return {};
        }

        QString trackTitle = t["title"].toString();
        if (trackTitle.isEmpty()) {
            return {};
        }

        QString artist = t["artist-credit"][0]["name"].toString();
        res.setDate(getDate(t));
        res.setAlbum(album);
        res.setArtist(artist);
        res.setGenre(getGenre(t));

        InternetTags::Track &track = res.tracks()[n];

        track.setTitle(trackTitle);
        track.setTrackNum(n);
    }

    return res;
}

/************************************************

 ************************************************/
void MusicBrainz::processResults()
{
    removeDuplicates();

    int n = 0;
    for (InternetTags &t : mResult) {
        n++;
        TagsId tagsId;
        tagsId.uri = QStringLiteral("https://musicbrainz.org/artis=%1&album=%2&num=%3").arg(mRequestArtist, mRequestAlbum).arg(n);
        if (mResult.size() == 1) {
            tagsId.title = QStringLiteral("%1 / %2   [ MusicBrainz ]").arg(mRequestArtist, mRequestAlbum);
        }
        else {
            tagsId.title = QStringLiteral("%1 / %2   [ MusicBrainz %3 ]").arg(mRequestArtist, mRequestAlbum).arg(n);
        }

        t.setTagsId(tagsId);
    }

    emit finished(mResult);
}
