#ifndef METADATAWRITER_H
#define METADATAWRITER_H

#include <QString>
#include "track.h"
#include "coverimage.h"
#include <taglib/mp4file.h>

namespace TagLib {

namespace Ogg {
class XiphComment;
}

namespace APE {
class Tag;
}

}

class MetadataWriter
{
public:
    explicit MetadataWriter(const QString &filePath);
    virtual ~MetadataWriter() = default;

    virtual void save() = 0;

    virtual void setTags(const Track &track)            = 0;
    virtual void setEmbeddedCue(const QString &cue)     = 0;
    virtual void setCoverImage(const CoverImage &image) = 0;

protected:
    void setXiphTags(TagLib::Ogg::XiphComment *tags, const Track &track) const;
    void setXiphEmbeddedCue(TagLib::Ogg::XiphComment *tags, const QString &cue) const;
    void setXiphCoverImage(TagLib::Ogg::XiphComment *tags, const CoverImage &image) const;

    void setApeTags(TagLib::APE::Tag *tags, const Track &track) const;
    void setApeCoverImage(TagLib::APE::Tag *tags, const CoverImage &image) const;
};

class NullMetadataWriter : public MetadataWriter
{
public:
    NullMetadataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;
};

class Mp4MetaDataWriter : public MetadataWriter
{
public:
    Mp4MetaDataWriter(const QString &filePath);

    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

private:
    TagLib::MP4::File mFile;
};

#endif // METADATAWRITER_H
