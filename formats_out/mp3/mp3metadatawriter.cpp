#include "mp3metadatawriter.h"
#include <QLoggingCategory>
#include <taglib/id3v2tag.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>

namespace {
Q_LOGGING_CATEGORY(LOG, "Mp3MetaDataWriter")
}

/************************************************

 ************************************************/
Mp3MetaDataWriter::Mp3MetaDataWriter(const QString &filePath) :
    MetadataWriter(filePath),
    mFile(filePath.toLocal8Bit(), false)
{
    if (!mFile.isValid()) {
        qCWarning(LOG) << Q_FUNC_INFO << "file is invalid";
        throw FlaconError("Can't open file");
    }
}

/************************************************

 ************************************************/
void Mp3MetaDataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************

 ************************************************/
static TagLib::ID3v2::Frame *addFrame(TagLib::ID3v2::Tag *tags, const TagLib::ByteVector &frameId)
{
    if (!tags->frameList(frameId).isEmpty()) {
        return tags->frameList(frameId).front();
    }

    TagLib::ID3v2::TextIdentificationFrame *frame = new TagLib::ID3v2::TextIdentificationFrame(frameId, TagLib::String::UTF8);
    tags->addFrame(frame);
    return frame;
}

/************************************************

 ************************************************/
void Mp3MetaDataWriter::setTags(const Track &track)
{
    TagLib::ID3v2::Tag *tags = mFile.ID3v2Tag(true);

    if (!track.artist().isEmpty())
        tags->setArtist(track.artist().toStdString());

    if (!track.album().isEmpty())
        tags->setAlbum(track.album().toStdString());

    if (!track.genre().isEmpty())
        tags->setGenre(track.genre().toStdString());

    if (!track.title().isEmpty())
        tags->setTitle(track.title().toStdString());

    if (!track.comment().isEmpty())
        tags->setComment(track.comment().toStdString());

    {
        int year = track.date().toInt();
        if (year)
            tags->setYear(year);
    }

    if (!track.tag(TagId::AlbumArtist).isEmpty()) {
        addFrame(tags, "TPE2")->setText(track.tag(TagId::AlbumArtist).toStdString());
    }

    addFrame(tags, "TRCK")->setText(QString("%1 / %2").arg(track.trackNum()).arg(track.trackCount()).toStdString());
    addFrame(tags, "TPOS")->setText(QString("%1 / %2").arg(track.discNum()).arg(track.discCount()).toStdString());
}

/************************************************

 ************************************************/
void Mp3MetaDataWriter::setEmbeddedCue(const QString &)
{
}

/************************************************

 ************************************************/
void Mp3MetaDataWriter::setCoverImage(const CoverImage &image)
{
    TagLib::ID3v2::Tag                  *tags = mFile.ID3v2Tag(true);
    TagLib::ID3v2::AttachedPictureFrame *apic = new TagLib::ID3v2::AttachedPictureFrame();

    TagLib::ByteVector img(image.data().data(), image.data().size());

    apic->setPicture(img);
    apic->setMimeType(image.mimeType().toStdString());
    apic->setType(TagLib::ID3v2::AttachedPictureFrame::Type::FrontCover);
    // apic->setDescription("Front Cover");
    tags->addFrame(apic);
}
