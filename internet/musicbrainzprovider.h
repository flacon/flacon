#ifndef MUSICBRAINZPROVIDER_H
#define MUSICBRAINZPROVIDER_H

#include "dataprovider.h"

class MusicBrainzProvider : public DataProvider
{
public:
    static bool canDownload(const Disc &disc);

    using DataProvider::DataProvider;
    void start() override;

private:
    void releaseGroupsReady(QNetworkReply *reply);

    void   releasesReady(QNetworkReply *reply);
    Tracks parseTracksJson(const QJsonArray &tracks, const QString &album);

    void processResults();

private:
    QString mRequestAlbum;
    QString mRequestArtist;
};

#endif // MUSICBRAINZPROVIDER_H
