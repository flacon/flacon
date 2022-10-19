#include "metadatawriter.h"
#include <taglib/tpropertymap.h>
#include <taglib/xiphcomment.h>
#include <taglib/apetag.h>

/************************************************
 *
 ************************************************/
MetadataWriter::MetadataWriter(const QString &)
{
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setXiphTags(TagLib::Ogg::XiphComment *tags, const Track &track) const
{
    auto writeStrTag = [tags](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            tags->addField(tagName.toStdString(), TagLib::String(value.toStdString(), TagLib::String::UTF8), true);
        }
    };

    auto writeIntTag = [tags](const QString &tagName, int value) {
        tags->addField(tagName.toStdString(), QString::number(value).toStdString(), true);
    };

    writeStrTag("ARTIST", track.artist());
    writeStrTag("ALBUM", track.album());
    writeStrTag("GENRE", track.genre());
    writeStrTag("DATE", track.date());
    writeStrTag("TITLE", track.title());
    writeStrTag("ALBUMARTIST", track.tag(TagId::AlbumArtist));
    writeStrTag("COMMENT", track.comment());
    writeStrTag("DISCID", track.discId());

    writeIntTag("TRACKNUMBER", track.trackNum());
    writeIntTag("TOTALTRACKS", track.trackCount());
    writeIntTag("TRACKTOTAL", track.trackCount());

    writeIntTag("DISC", track.discNum());
    writeIntTag("DISCNUMBER", track.discNum());
    writeIntTag("DISCTOTAL", track.discCount());

    //    if (!embeddedCue().isEmpty()) {
    //        writeStrTag("CUESHEET", embeddedCue());
    //    }

    //    if (!coverImage().isEmpty()) {
    //        const CoverImage  &img = coverImage();
    //        TagLib::ByteVector dt(img.data().data(), img.data().size());

    //        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
    //        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
    //        pic->setData(dt);
    //        pic->setMimeType(img.mimeType().toStdString());
    //        pic->setWidth(img.size().width());
    //        pic->setHeight(img.size().height());
    //        pic->setColorDepth(img.depth());

    //        TagLib::ByteVector block = pic->render();
    //        QByteArray         data(block.data(), block.size());
    //        tags->addField("METADATA_BLOCK_PICTURE", data.toBase64().toStdString(), true);
    //    }
}

void MetadataWriter::setXiphEmbeddedCue(TagLib::Ogg::XiphComment *tags, const QString &cue) const
{
    tags->addField("CUESHEET", TagLib::String(cue.toStdString(), TagLib::String::UTF8), true);
}

void MetadataWriter::setXiphCoverImage(TagLib::Ogg::XiphComment *tags, const CoverImage &image) const
{
    TagLib::ByteVector dt(image.data().data(), image.data().size());

    TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
    pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
    pic->setData(dt);
    pic->setMimeType(image.mimeType().toStdString());
    pic->setWidth(image.size().width());
    pic->setHeight(image.size().height());
    pic->setColorDepth(image.depth());

    TagLib::ByteVector block = pic->render();
    QByteArray         data(block.data(), block.size());
    tags->addField("METADATA_BLOCK_PICTURE", data.toBase64().toStdString(), true);
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

    writeStrTag("ARTIST", track.artist());
    writeStrTag("ALBUM", track.album());
    writeStrTag("GENRE", track.genre());
    writeStrTag("YEAR", track.date());
    writeStrTag("TITLE", track.title());
    writeStrTag("ALBUM ARTIST", track.tag(TagId::AlbumArtist));
    writeStrTag("COMMENT", track.comment());
    writeStrTag("DISCID", track.discId());

    writeStrTag("TRACK", QString("%1/%2").arg(track.trackNum()).arg(track.trackCount()));
    writeStrTag("PART", QString("%1").arg(track.discNum()));
}

/************************************************
 *
 ************************************************/
void MetadataWriter::setApeCoverImage(TagLib::APE::Tag *tags, const CoverImage &image) const
{
    TagLib::ByteVector imgData(image.data().data(), image.data().size());

    TagLib::ByteVector data;
    data.append(QString("Cover Art (Front).%1").arg(image.fileExt()).toUtf8().data());
    data.append(TagLib::ByteVector(1, 0));
    data.append(imgData);
    tags->setItem("Cover Art (Front)", TagLib::APE::Item("Cover Art (Front)", data, true));
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

///************************************************
// *
// ************************************************/
// void Mp4MetaDataWriter::writeTags() const
//{
//    TagLib::MP4::File file(filePath().toLocal8Bit(), false);

//    if (!file.isValid()) {
//        throw FlaconError("Can't open file");
//    }

//    TagLib::PropertyMap props = file.properties();

//    auto writeStrTag = [&props](const QString &tagName, const QString &value) {
//        if (!value.isEmpty()) {
//            props.replace(TagLib::String(tagName.toStdString(), TagLib::String::UTF8), TagLib::String(value.toStdString(), TagLib::String::UTF8));
//        }
//    };

//    writeStrTag("ARTIST", track().artist());
//    writeStrTag("ALBUM", track().album());
//    writeStrTag("GENRE", track().genre());
//    writeStrTag("DATE", track().date());
//    writeStrTag("TITLE", track().title());
//    writeStrTag("ALBUMARTIST", track().albumArtist());
//    writeStrTag("COMMENT", track().comment());
//    writeStrTag("TRACKNUMBER", QString("%1/%2").arg(track().trackNum()).arg(track().trackCount()));
//    writeStrTag("DISCNUMBER", QString("%1/%2").arg(track().discNum()).arg(track().discCount()));

//    file.setProperties(props);

//    if (!coverImage().isEmpty()) {
//        const CoverImage  &img = coverImage();
//        TagLib::ByteVector data(img.data().data(), img.data().size());

//        TagLib::MP4::CoverArt cover(coverFormatToTagLib(img.format()), data);
//        file.tag()->setItem("covr", TagLib::MP4::CoverArtList().append(cover));
//    }

//    if (!file.save()) {
//        throw FlaconError("Can't save file");
//    }
//}

Mp4MetaDataWriter::Mp4MetaDataWriter(const QString &filePath) :
    MetadataWriter(filePath),
    mFile(filePath.toLocal8Bit(), false)
{
    if (!mFile.isValid()) {
        throw FlaconError("Can't open file");
    }
}

void Mp4MetaDataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

void Mp4MetaDataWriter::setTags(const Track &track)
{
    TagLib::PropertyMap props = mFile.properties();

    auto writeStrTag = [&props](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            props.replace(TagLib::String(tagName.toStdString(), TagLib::String::UTF8), TagLib::String(value.toStdString(), TagLib::String::UTF8));
        }
    };

    writeStrTag("ARTIST", track.artist());
    writeStrTag("ALBUM", track.album());
    writeStrTag("GENRE", track.genre());
    writeStrTag("DATE", track.date());
    writeStrTag("TITLE", track.title());
    writeStrTag("ALBUMARTIST", track.albumArtist());
    writeStrTag("COMMENT", track.comment());
    writeStrTag("TRACKNUMBER", QString("%1/%2").arg(track.trackNum()).arg(track.trackCount()));
    writeStrTag("DISCNUMBER", QString("%1/%2").arg(track.discNum()).arg(track.discCount()));

    mFile.setProperties(props);
}

void Mp4MetaDataWriter::setEmbeddedCue(const QString &)
{
}

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
NullMetadataWriter::NullMetadataWriter(const QString &filePath) :
    MetadataWriter(filePath)
{
}

/************************************************
 *
 ************************************************/
void NullMetadataWriter::save()
{
}

/************************************************
 *
 ************************************************/
void NullMetadataWriter::setTags(const Track &)
{
}

/************************************************
 *
 ************************************************/
void NullMetadataWriter::setEmbeddedCue(const QString &)
{
}

/************************************************
 *
 ************************************************/
void NullMetadataWriter::setCoverImage(const CoverImage &)
{
}
