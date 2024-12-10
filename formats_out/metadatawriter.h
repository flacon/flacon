#ifndef METADATAWRITER_H
#define METADATAWRITER_H

/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QString>
#include "track.h"
#include "coverimage.h"
#include <taglib/mp4file.h>
#include "../profiles.h"

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
    explicit MetadataWriter(const Profile &profile, const QString &filePath);
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

    QString artistTag(const Track &track) const;
    QString commentTag(const Track &track) const;
    QString dateTag(const Track &track) const;
    QString genreTag(const Track &track) const;

    const Profile &profile() const { return mProfile; }

    bool needWriteDiskNumTags(const Track &track) const;

private:
    Profile mProfile;
};

class NullMetadataWriter : public MetadataWriter
{
public:
    NullMetadataWriter(const Profile &profile, const QString &filePath);
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
    Mp4MetaDataWriter(const Profile &profile, const QString &filePath);

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
