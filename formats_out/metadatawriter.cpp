#include "metadatawriter.h"
#include <taglib/tpropertymap.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4file.h>

/************************************************
 *
 ************************************************/
void MetadataWriter::setFilePath(const QString &value)
{
    mFilePath = value;
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setTrack(const Track &track)
{
    mTrack = track;
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setEmbeddedCue(const QString &value)
{
    mEmbeddedCue = value;
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setCoverImage(const CoverImage &value)
{
    mCoverImage = value;
}

/************************************************
 *
 ************************************************/
void MetadataWriter::writeXiphComments(TagLib::Ogg::XiphComment *comments) const
{
    auto writeStrTag = [comments](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            comments->addField(tagName.toStdString(), TagLib::String(value.toStdString(), TagLib::String::UTF8), true);
        }
    };

    auto writeIntTag = [comments](const QString &tagName, int value) {
        comments->addField(tagName.toStdString(), QString::number(value).toStdString(), true);
    };

    writeStrTag("ARTIST", track().artist());
    writeStrTag("ALBUM", track().album());
    writeStrTag("GENRE", track().genre());
    writeStrTag("DATE", track().date());
    writeStrTag("TITLE", track().title());
    writeStrTag("ALBUMARTIST", track().tag(TagId::AlbumArtist));
    writeStrTag("ALBUM_ARTIST", track().tag(TagId::AlbumArtist));
    writeStrTag("COMMENT", track().comment());
    writeStrTag("DISCID", track().discId());

    writeIntTag("TRACKNUMBER", track().trackNum());
    writeIntTag("TOTALTRACKS", track().trackCount());
    writeIntTag("TRACKTOTAL", track().trackCount());

    writeIntTag("DISC", track().discNum());
    writeIntTag("DISCNUMBER", track().discNum());
    writeIntTag("DISCTOTAL", track().discCount());

    if (!embeddedCue().isEmpty()) {
        writeStrTag("CUESHEET", embeddedCue());
    }
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
void Mp4MetaDataWriter::writeTags() const
{
    TagLib::MP4::File file(filePath().toLocal8Bit(), false);

    if (!file.isValid()) {
        throw FlaconError("Can't open file");
    }

    TagLib::PropertyMap props = file.properties();

    auto writeStrTag = [&props](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            props.replace(TagLib::String(tagName.toStdString(), TagLib::String::UTF8), TagLib::String(value.toStdString(), TagLib::String::UTF8));
        }
    };

    writeStrTag("ARTIST", track().artist());
    writeStrTag("ALBUM", track().album());
    writeStrTag("GENRE", track().genre());
    writeStrTag("DATE", track().date());
    writeStrTag("TITLE", track().title());
    writeStrTag("ALBUMARTIST", track().albumArtist());
    writeStrTag("COMMENT", track().comment());
    writeStrTag("TRACKNUMBER", QString("%1/%2").arg(track().trackNum()).arg(track().trackCount()));
    writeStrTag("DISCNUMBER", QString("%1/%2").arg(track().discNum()).arg(track().discCount()));

    file.setProperties(props);

    if (!coverImage().isEmpty()) {
        const CoverImage  &img = coverImage();
        TagLib::ByteVector data(img.data().data(), img.data().size());

        TagLib::MP4::CoverArt cover(coverFormatToTagLib(img.format()), data);
        file.tag()->setItem("covr", TagLib::MP4::CoverArtList().append(cover));
    }

    if (!file.save()) {
        throw FlaconError("Can't save file");
    }
}
