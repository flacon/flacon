#ifndef OPUSMETADATAWRITER_H
#define OPUSMETADATAWRITER_H

#include "../metadatawriter.h"
#include <taglib/opusfile.h>

class OpusMetadataWriter : public MetadataWriter
{
public:
    OpusMetadataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

    void setTrackReplayGain(float gain, float peak) override;
    void setAlbumReplayGain(float gain, float peak) override;

private:
    TagLib::Ogg::Opus::File mFile;
};

#endif // OPUSMETADATAWRITER_H
