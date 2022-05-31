#ifndef MUSICBRAINZPROVIDER_H
#define MUSICBRAINZPROVIDER_H

#include "dataprovider.h"

class MusicBrainzProvider : public DataProvider
{
public:
    static bool canDownload(const Disc &disc);

    using DataProvider::DataProvider;
    void start() override;

protected:
    QNetworkReply *get(const QNetworkRequest &request) override;

    void releaseGroupsReady(QNetworkReply *reply);
    void releasesReady(QNetworkReply *reply);

    Tracks parseTracksJson(const QJsonArray &tracks, const QString &artist, const QString &album);
    void   processResults();

private:
    QString mAlbum;
    QString mArtist;
    int     mTracksCount = 0;
};

#endif // MUSICBRAINZPROVIDER_H
