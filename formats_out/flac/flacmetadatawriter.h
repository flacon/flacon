#ifndef FLACMETADATAWRITER_H
#define FLACMETADATAWRITER_H

#include "../metadatawriter.h"
#include <taglib/flacfile.h>

class FlacMetadataWriter : public MetadataWriter
{
public:
    FlacMetadataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

private:
    TagLib::FLAC::File mFile;
};

#endif // FLACMETADATAWRITER_H
