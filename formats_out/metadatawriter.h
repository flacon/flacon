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

    virtual void setTrackReplayGain(float gain, float peak) = 0;
    virtual void setAlbumReplayGain(float gain, float peak) = 0;

protected:
    QString gainToString(float &gain) const;
    QString peakToString(float &peak) const;

    void setXiphTag(TagLib::Ogg::XiphComment *tags, const QString &key, const QString &value) const;
    void setXiphTags(TagLib::Ogg::XiphComment *tags, const Track &track) const;
    void setXiphEmbeddedCue(TagLib::Ogg::XiphComment *tags, const QString &cue) const;
    void setXiphCoverImage(TagLib::Ogg::XiphComment *tags, const CoverImage &image) const;
    void setXiphTrackReplayGain(TagLib::Ogg::XiphComment *tags, float gain, float peak) const;
    void setXiphAlbumReplayGain(TagLib::Ogg::XiphComment *tags, float gain, float peak) const;

    void setApeTags(TagLib::APE::Tag *tags, const Track &track) const;
    void setApeCoverImage(TagLib::APE::Tag *tags, const CoverImage &image) const;
    void setApeTrackReplayGain(TagLib::APE::Tag *tags, float gain, float peak) const;
    void setApeAlbumReplayGain(TagLib::APE::Tag *tags, float gain, float peak) const;
};

class NullMetadataWriter : public MetadataWriter
{
public:
    NullMetadataWriter(const QString &filePath);
    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

    void setTrackReplayGain(float gain, float peak) override;
    void setAlbumReplayGain(float gain, float peak) override;
};

class Mp4MetaDataWriter : public MetadataWriter
{
public:
    Mp4MetaDataWriter(const QString &filePath);

    void save() override;

    void setTags(const Track &track) override;
    void setEmbeddedCue(const QString &cue) override;
    void setCoverImage(const CoverImage &image) override;

    void setTrackReplayGain(float gain, float peak) override;
    void setAlbumReplayGain(float gain, float peak) override;

private:
    TagLib::MP4::File mFile;
};

#endif // METADATAWRITER_H
