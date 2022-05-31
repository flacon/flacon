#include "musicbrainzprovider.h"
#include <QDebug>
#include "../disc.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "MusicBrainzProvider")
}

static constexpr auto SEARCH_URL = "http://musicbrainz.org/ws/2/release-group/?fmt=json&query=%1";
static constexpr auto LOOKUP_URL = "https://musicbrainz.org/ws/2/release/?fmt=json&inc=recordings&release-group=%1";

bool MusicBrainzProvider::canDownload(const Disc &disk)
{
    if (disk.isEmpty()) {
        return false;
    }

    const Track *track = disk.track(0);

    return !track->artist().isEmpty() && !track->album().isEmpty();
}

void MusicBrainzProvider::start()
{
    if (disc().isEmpty()) {
        return;
    }

    const Track *track = disc().track(0);
    mArtist            = track->artist();
    mAlbum             = track->album();
    mTracksCount       = disc().count();

    QStringList query;
    query << QString("artist:\"%1\"").arg(mArtist.toHtmlEscaped());
    query << QString("release:\"%1\"").arg(mAlbum.toHtmlEscaped());

    QUrl url = QString(SEARCH_URL).arg(query.join("%20AND%20"));

    QNetworkRequest request(url);

    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { releaseGroupsReady(reply); });
}

QNetworkReply *MusicBrainzProvider::get(const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    req.setRawHeader("User-Agent", "Flacon/9.0 (https://github.com/flacon/flacon)");
    qCDebug(LOG).noquote() << req.url().toEncoded();
    return DataProvider::get(req);
}

void MusicBrainzProvider::releaseGroupsReady(QNetworkReply *reply)
{
    if (reply->error()) {
        QString err = reply->errorString();
        qCWarning(LOG) << err;
        error(err);
        return;
    }

    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(LOG) << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        error(parseError.errorString());
        return;
    }

    QJsonArray releaseGroups = doc["release-groups"].toArray();
    if (releaseGroups.isEmpty()) {
        emit finished();
        return;
    }

    for (const QJsonValue r : releaseGroups) {
        QString id = r["id"].toString();
        if (id.isEmpty()) {
            continue;
        }

        if (r["title"] != mAlbum) {
            continue;
        }

        QNetworkReply *reply = get(QNetworkRequest(QString(LOOKUP_URL).arg(id)));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() { releasesReady(reply); });
    }
}

void MusicBrainzProvider::releasesReady(QNetworkReply *reply)
{
    if (reply->error()) {
        QString err = reply->errorString();
        qCWarning(LOG) << err;
        error(err);
        return;
    }

    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(LOG) << "Parse error at" << parseError.offset << ":" << parseError.errorString();
        error(parseError.errorString());
        return;
    }

    QJsonArray releases = doc["releases"].toArray();
    if (releases.isEmpty()) {
        return;
    }

    QVector<Tracks> res;
    QString         artist = disc().track(0)->artist();

    for (const QJsonValue r : releases) {
        QString album = r["title"].toString();

        for (const QJsonValue m : r["media"].toArray()) {
            if (m["track-count"].toInt() != disc().count()) {
                continue;
            }

            Tracks tracks = parseTracksJson(m["tracks"].toArray(), artist, album);
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

Tracks MusicBrainzProvider::parseTracksJson(const QJsonArray &tracks, const QString &artist, const QString &album)
{
    Tracks res;
    res.resize(tracks.count());

    int n = 0;
    for (const QJsonValue t : tracks) {

        int pos = t["position"].toInt(-1);
        if (pos < 0) {
            return {};
        }

        QString title = t["title"].toString();
        if (title.isEmpty()) {
            return {};
        }

        QString date = t["first-release-date"].toString();

        Track &track = res[n++];
        track.setCodecName(disc().codecName());
        track.setTag(TagId::Date, date);
        // track.setTag(TagId::Genre, genre);
        track.setTag(TagId::Album, album);
        track.setTag(TagId::Artist, artist);
        track.setTag(TagId::Title, title);
    }

    return res;
}

void MusicBrainzProvider::processResults()
{
    removeDduplicates();

    int n = 0;
    for (Tracks &t : mResult) {
        n++;
        t.setUri(QString("https://musicbrainz.org/artis=%1&album=%2&num=%3").arg(mArtist, mAlbum).arg(n));
        if (mResult.size() == 1) {
            t.setTitle(QString("%1 / %2   [ MusicBrainz ]").arg(mArtist, mAlbum));
        }
        else {
            t.setTitle(QString("%1 / %2   [ MusicBrainz %3 ]").arg(mArtist, mAlbum).arg(n));
        }
    }

    emit ready(mResult);
    emit finished();
}
