#ifndef WVMETADATAWRITER_H
#define WVMETADATAWRITER_H

#include "../metadatawriter.h"
#include <taglib/wavpackfile.h>

class WvMetadataWriter : public MetadataWriter
{
public:
    WvMetadataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

private:
    TagLib::WavPack::File mFile;
};

#endif // WVMETADATAWRITER_H
