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

#include "metadatawriter.h"
#include <taglib/tpropertymap.h>
#include <taglib/xiphcomment.h>
#include <taglib/apetag.h>
#include <cmath>
#include "disc.h"

/************************************************
 * Some useful code can be viewed in the project
 *  https://github.com/Moonbase59/loudgain/blob/master/src/tag.cc
 ************************************************/

/************************************************
 *
 ************************************************/
MetadataWriter::MetadataWriter(const Profile &profile, const QString &) :
    mProfile(profile)
{
}

/************************************************
 *
 ************************************************/
QString MetadataWriter::gainToString(float &gain) const
{
    return QStringLiteral("%1 dB").arg(gain, 0, 'f', 2);
}

/************************************************
 *
 ************************************************/
QString MetadataWriter::peakToString(float &peak) const
{
    return QStringLiteral("%1").arg(peak, 0, 'f', 6);
}

/************************************************
 *
 ************************************************/
QString MetadataWriter::artistTag(const Track &track) const
{
    return track.artistTag();
}

/************************************************
 *
 ************************************************/
QString MetadataWriter::commentTag(const Track &track) const
{
    return track.commentTag();
}

/************************************************
 *
 ************************************************/
QString MetadataWriter::dateTag(const Track &track) const
{
    return track.dateTag();
}

/**************************************
 *
 **************************************/
QString MetadataWriter::genreTag(const Track &track) const
{
    return track.genreTag();
}

/**************************************
 *
 **************************************/
bool MetadataWriter::needWriteDiskNumTags(const Track &track) const
{
    return (track.disk()->discCountTag() > 1 || profile().isWriteSingleDiskNum());
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphTag(TagLib::Ogg::XiphComment *tags, const QString &key, const QString &value) const
{
    if (!value.isEmpty()) {
        tags->addField(key.toStdString(), TagLib::String(value.toStdString(), TagLib::String::UTF8), true);
    }
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphTags(TagLib::Ogg::XiphComment *tags, const Track &track) const
{
    auto writeIntTag = [tags](const QString &tagName, int value) {
        tags->addField(tagName.toStdString(), QString::number(value).toStdString(), true);
    };

    Disk *disk = track.disk();

    setXiphTag(tags, "ARTIST", this->artistTag(track));
    setXiphTag(tags, "ALBUM", disk->albumTag());
    setXiphTag(tags, "COMPOSER", track.songWriterTag());
    setXiphTag(tags, "GENRE", this->genreTag(track));
    setXiphTag(tags, "DATE", this->dateTag(track));
    setXiphTag(tags, "TITLE", track.titleTag());
    setXiphTag(tags, "ALBUMARTIST", disk->albumPerformerTag());
    setXiphTag(tags, "COMMENT", this->commentTag(track));
    setXiphTag(tags, "DISCID", disk->discIdTag());
    setXiphTag(tags, "CATALOGNUMBER", disk->catalogTag());

    writeIntTag("TRACKNUMBER", track.trackNumTag());
    writeIntTag("TOTALTRACKS", disk->tracks().count());
    writeIntTag("TRACKTOTAL", disk->tracks().count());

    if (needWriteDiskNumTags(track)) {
        writeIntTag("DISC", disk->discNumTag());
        writeIntTag("DISCNUMBER", disk->discNumTag());
        writeIntTag("DISCTOTAL", disk->discCountTag());
    }
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphEmbeddedCue(TagLib::Ogg::XiphComment *tags, const QString &cue) const
{
    tags->addField("CUESHEET", TagLib::String(cue.toStdString(), TagLib::String::UTF8), true);
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphCoverImage(TagLib::Ogg::XiphComment *tags, const CoverImage &image) const
{
    TagLib::ByteVector dt(image.data().data(), image.data().size());

    TagLib::FLAC::Picture pic;
    pic.setType(TagLib::FLAC::Picture::Type::FrontCover);
    pic.setData(dt);
    pic.setMimeType(image.mimeType().toStdString());
    pic.setWidth(image.size().width());
    pic.setHeight(image.size().height());
    pic.setColorDepth(image.depth());

    TagLib::ByteVector block = pic.render();
    QByteArray         data(block.data(), block.size());
    tags->addField("METADATA_BLOCK_PICTURE", data.toBase64().toStdString(), true);
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphTrackReplayGain(TagLib::Ogg::XiphComment *tags, float gain, float peak) const
{
    setXiphTag(tags, "REPLAYGAIN_TRACK_GAIN", gainToString(gain));
    setXiphTag(tags, "REPLAYGAIN_TRACK_PEAK", peakToString(peak));
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphAlbumReplayGain(TagLib::Ogg::XiphComment *tags, float gain, float peak) const
{
    setXiphTag(tags, "REPLAYGAIN_ALBUM_GAIN", gainToString(gain));
    setXiphTag(tags, "REPLAYGAIN_ALBUM_PEAK", peakToString(peak));
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setApeTags(TagLib::APE::Tag *tags, const Track &track) const
{
    auto writeStrTag = [tags](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            tags->addValue(tagName.toStdString(), TagLib::String(value.toStdString(), TagLib::String::UTF8), true);
        }
    };

    Disk *disk = track.disk();

    writeStrTag("ARTIST", this->artistTag(track));
    writeStrTag("ALBUM", disk->albumTag());
    writeStrTag("GENRE", this->genreTag(track));
    writeStrTag("YEAR", this->dateTag(track));
    writeStrTag("TITLE", track.titleTag());
    writeStrTag("ALBUM ARTIST", disk->albumPerformerTag());
    writeStrTag("COMMENT", this->commentTag(track));
    writeStrTag("DISCID", disk->discIdTag());
    writeStrTag("COMPOSER", track.songWriterTag());
    writeStrTag("CATALOGNUMBER", disk->catalogTag());

    writeStrTag("TRACK", QStringLiteral("%1/%2").arg(track.trackNumTag()).arg(disk->tracks().count()));
    if (needWriteDiskNumTags(track)) {
        writeStrTag("PART", QStringLiteral("%1").arg(disk->discNumTag()));
    }
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setApeCoverImage(TagLib::APE::Tag *tags, const CoverImage &image) const
{
    TagLib::ByteVector imgData(image.data().data(), image.data().size());

    TagLib::ByteVector data;
    data.append(QStringLiteral("Cover Art (Front).%1").arg(image.fileExt()).toUtf8().data());
    data.append(TagLib::ByteVector(1, 0));
    data.append(imgData);
    tags->setItem("Cover Art (Front)", TagLib::APE::Item("Cover Art (Front)", data, true));
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setApeTrackReplayGain(TagLib::APE::Tag *tags, float gain, float peak) const
{
    tags->addValue("REPLAYGAIN_TRACK_GAIN", gainToString(gain).toStdString());
    tags->addValue("REPLAYGAIN_TRACK_PEAK", peakToString(peak).toStdString());
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setApeAlbumReplayGain(TagLib::APE::Tag *tags, float gain, float peak) const
{
    tags->addValue("REPLAYGAIN_ALBUM_GAIN", gainToString(gain).toStdString());
    tags->addValue("REPLAYGAIN_ALBUM_PEAK", peakToString(peak).toStdString());
}

/************************************************
 *
 ************************************************/
static TagLib::MP4::CoverArt::Format coverFormatToTagLib(const CoverImage::Format fmt)
{
    // clang-format off
    switch (fmt) {
        case CoverImage::Format::JPG: return TagLib::MP4::CoverArt::Format::JPEG;
        case CoverImage::Format::PNG: return TagLib::MP4::CoverArt::Format::PNG;
        case CoverImage::Format::BMP: return TagLib::MP4::CoverArt::Format::BMP;
        case CoverImage::Format::GIF: return TagLib::MP4::CoverArt::Format::GIF;
        default:                      return TagLib::MP4::CoverArt::Format::Unknown;
    }
    // clang-format on
}

/************************************************
 *
 ************************************************/
Mp4MetaDataWriter::Mp4MetaDataWriter(const Profile &profile, const QString &filePath) :
    MetadataWriter(profile, filePath),
    mFile(filePath.toLocal8Bit(), false)
{
    if (!mFile.isValid()) {
        throw FlaconError("Can't open file");
    }
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::setTags(const Track &track)
{
    TagLib::PropertyMap props = mFile.properties();

    auto writeStrTag = [&props](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            props.replace(TagLib::String(tagName.toStdString(), TagLib::String::UTF8), TagLib::String(value.toStdString(), TagLib::String::UTF8));
        }
    };

    Disk *disk = track.disk();

    writeStrTag("ARTIST", this->artistTag(track));
    writeStrTag("ALBUM", disk->albumTag());
    writeStrTag("GENRE", this->genreTag(track));
    writeStrTag("DATE", this->dateTag(track));
    writeStrTag("TITLE", track.titleTag());
    writeStrTag("ALBUMARTIST", disk->albumPerformerTag());
    writeStrTag("COMMENT", this->commentTag(track));
    writeStrTag("COMPOSER", track.songWriterTag());
    writeStrTag("CATALOGNUMBER", disk->catalogTag());

    writeStrTag("TRACKNUMBER", QStringLiteral("%1/%2").arg(track.trackNumTag()).arg(disk->tracks().count()));
    if (needWriteDiskNumTags(track)) {
        writeStrTag("DISCNUMBER", QStringLiteral("%1/%2").arg(disk->discNumTag()).arg(disk->discCountTag()));
    }

    mFile.setProperties(props);
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::setEmbeddedCue(const QString &)
{
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::setCoverImage(const CoverImage &image)
{
    if (!image.isEmpty()) {
        TagLib::ByteVector data(image.data().data(), image.data().size());

        TagLib::MP4::CoverArt cover(coverFormatToTagLib(image.format()), data);
        mFile.tag()->setItem("covr", TagLib::MP4::CoverArtList().append(cover));
    }
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::setTrackReplayGain(float gain, float peak)
{
    TagLib::MP4::Tag *tags = mFile.tag();
    tags->setItem("----:com.apple.iTunes:replaygain_track_gain", TagLib::StringList(QString("%1 dB").arg(gain, 0, 'f', 2).toStdString()));
    tags->setItem("----:com.apple.iTunes:replaygain_track_peak", TagLib::StringList(QString("%1").arg(peak, 0, 'f', 6).toStdString()));
}

/************************************************
 *
 ************************************************/
void Mp4MetaDataWriter::setAlbumReplayGain(float gain, float peak)
{
    TagLib::MP4::Tag *tags = mFile.tag();
    tags->setItem("----:com.apple.iTunes:replaygain_album_gain", TagLib::StringList(QString("%1 dB").arg(gain, 0, 'f', 2).toStdString()));
    tags->setItem("----:com.apple.iTunes:replaygain_album_peak", TagLib::StringList(QString("%1").arg(peak, 0, 'f', 6).toStdString()));
}

/************************************************
 *
 ************************************************/
NullMetadataWriter::NullMetadataWriter(const Profile &profile, const QString &filePath) :
    MetadataWriter(profile, filePath) { }
void NullMetadataWriter::save() { }
void NullMetadataWriter::setTags(const Track &) { }
void NullMetadataWriter::setEmbeddedCue(const QString &) { }
void NullMetadataWriter::setCoverImage(const CoverImage &) { }
void NullMetadataWriter::setTrackReplayGain(float, float) { }
void NullMetadataWriter::setAlbumReplayGain(float, float) { }
