#ifndef MP3METADATAWRITER_H
#define MP3METADATAWRITER_H

#include "../metadatawriter.h"
#include <taglib/mpegfile.h>

class Mp3MetaDataWriter : public MetadataWriter
{
public:
    Mp3MetaDataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

    void setTrackReplayGain(float gain, float peak) override;
    void setAlbumReplayGain(float gain, float peak) override;

private:
    TagLib::MPEG::File mFile;
};

#endif // MP3METADATAWRITER_H
