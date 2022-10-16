#ifndef METADATAWRITER_H
#define METADATAWRITER_H

#include <QString>
#include "track.h"
#include "coverimage.h"

namespace TagLib {
namespace Ogg {
class XiphComment;
}
}

class MetadataWriter
{
public:
    virtual ~MetadataWriter() = default;

    const QString filePath() const { return mFilePath; }
    void          setFilePath(const QString &value);

    const Track &track() const { return mTrack; }
    void         setTrack(const Track &track);

    const QString &embeddedCue() const { return mEmbeddedCue; }
    void           setEmbeddedCue(const QString &value);

    const CoverImage &coverImage() const { return mCoverImage; }
    void              setCoverImage(const CoverImage &value);

    virtual void writeTags() const = 0;

protected:
    QString    mFilePath;
    Track      mTrack;
    QString    mEmbeddedCue;
    CoverImage mCoverImage;

    void writeXiphComments(TagLib::Ogg::XiphComment *comments) const;
};

class Mp4MetaDataWriter : public MetadataWriter
{
public:
    void writeTags() const override;
};

#endif // METADATAWRITER_H
