#ifndef OGGMETADATAWRITER_H
#define OGGMETADATAWRITER_H

#include "../metadatawriter.h"
#include <taglib/vorbisfile.h>

class OggMetaDataWriter : public MetadataWriter
{
public:
    OggMetaDataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

private:
    TagLib::Ogg::Vorbis::File mFile;
};

#endif // OGGMETADATAWRITER_H
