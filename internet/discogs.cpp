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

#include "discogs.h"
#include "../disc.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>
/* Algorithm ************************************
I couldn't get the data in one request, so I'm using 2 steps.

Step 1:
  Search a list of "masters" by artist name and album title.
  For each result, we take the "master_id" and perform step 2

Step 2:
  Requesting a metadata of "master" including track titles.

Postprocessing:
  Since the disc is often released several times, but contains the same tracks,
  our results will contain duplicates. We delete the duplicates.


Example:
  1. https://api.discogs.com/database/search?artist=Aerosmith&release_title=%22Permanent%20Vacation%22&token=nBycgGNNMBbfTpqyMkGvfGJRxEnsvXegKxqKtVLx
  2. https://api.discogs.com/masters/37029

Documentation:
  https://www.discogs.com/developers
*************************************************/
namespace {
Q_LOGGING_CATEGORY(LOG, "MusicBrainzProvider")
}

static constexpr auto TOKEN      = "nBycgGNNMBbfTpqyMkGvfGJRxEnsvXegKxqKtVLx";
static constexpr auto SEARCH_URL = "https://api.discogs.com/database/search?%1";
static constexpr auto LOOKUP_URL = "https://api.discogs.com/masters/%1";

bool Discogs::canDownload(const Disc &disk)
{
    if (disk.isEmpty()) {
        return false;
    }

    const Track *track = disk.track(0);

    return !track->artist().isEmpty() && !track->album().isEmpty();
}

void Discogs::start()
{
    if (mDisk.isEmpty()) {
        return;
    }

    const Track *track = mDisk.track(0);
    mRequestArtist     = track->artist();
    mRequestAlbum      = track->album();

    QStringList query;
    query << QString("artist=\"%1\"").arg(mRequestArtist.toHtmlEscaped());
    query << QString("release_title=\"%1\"").arg(mRequestAlbum.toHtmlEscaped());

    QUrl url = QString(SEARCH_URL).arg(query.join("&"));

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Discogs token=%1").arg(TOKEN).toUtf8().data());

    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { searchReady(reply); });
}

void Discogs::searchReady(QNetworkReply *reply)
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

    QSet<int> ids;
    for (const QJsonValue &r : doc["results"].toArray()) {
        ids << r["master_id"].toInt();
    }

    for (int id : ids) {
        if (!id) {
            continue;
        }

        QNetworkReply *reply = get(QNetworkRequest(QString(LOOKUP_URL).arg(id)));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() { masterReady(reply); });
    }

    if (isFinished()) {
        emit finished({});
    }
}

void Discogs::masterReady(QNetworkReply *reply)
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

    int        year      = doc["year"].toInt();
    QString    album     = doc["title"].toString();
    QString    artist    = doc["artists"][0]["name"].toString();
    QString    genre     = doc["genres"][0].toString();
    QJsonArray tracklist = doc["tracklist"].toArray();

    bool skip = false;

    skip = skip || tracklist.size() != mDisk.count();
    skip = skip || album.toUpper().simplified() != mRequestAlbum.toUpper().simplified();
    skip = skip || artist.toUpper().simplified() != mRequestArtist.toUpper().simplified();

    if (!skip) {
        InternetTags res;
        int          n = -1;
        for (const QJsonValue &t : tracklist) {
            n++;

            res.setTrackTag(n, TagId::Date, year > 0 ? QString::number(year) : "");
            res.setTrackTag(n, TagId::Album, album);
            res.setTrackTag(n, TagId::Artist, artist);
            res.setTrackTag(n, TagId::Title, t["title"].toString());
            res.setTrackTag(n, TagId::Genre, genre);
            res.setTrackTag(n, TagId::TrackNum, t["position"].toString());
            res.setTrackTag(n, TagId::TrackCount, QString::number(tracklist.size()));

            res.setAlbumTag(TagId::Artist, artist);
            res.setTrackTag(n, TagId::SongWriter, artist);
        }

        mResult << res;
    }

    if (isFinished()) {
        processResults();
    }
}

void Discogs::processResults()
{
    removeDuplicates();

    int n = 0;
    for (InternetTags &t : mResult) {
        n++;
        t.setUri(QString("https://discogs.com/Artist=%1&Album=%2&num=%3").arg(mRequestArtist, mRequestAlbum).arg(n));
        if (mResult.size() == 1) {
            t.setName(QString("%1 / %2   [ Discogs ]").arg(mRequestArtist, mRequestAlbum));
        }
        else {
            t.setName(QString("%1 / %2   [ Discogs %3 ]").arg(mRequestArtist, mRequestAlbum).arg(n));
        }
    }

    emit finished(mResult);
}
