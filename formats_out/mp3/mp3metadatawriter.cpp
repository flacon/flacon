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
        tags->setArtist(TagLib::String(track.artist().toUtf8().data(), TagLib::String::UTF8));

    if (!track.album().isEmpty())
        tags->setAlbum(TagLib::String(track.album().toUtf8().data(), TagLib::String::UTF8));

    if (!track.genre().isEmpty())
        tags->setGenre(TagLib::String(track.genre().toUtf8().data(), TagLib::String::UTF8));

    if (!track.title().isEmpty())
        tags->setTitle(TagLib::String(track.title().toUtf8().data(), TagLib::String::UTF8));

    if (!track.comment().isEmpty())
        tags->setComment(TagLib::String(track.comment().toUtf8().data(), TagLib::String::UTF8));

    {
        int year = track.date().toInt();
        if (year)
            tags->setYear(year);
    }

    if (!track.tag(TagId::AlbumArtist).isEmpty()) {
        addFrame(tags, "TPE2")->setText(TagLib::String(track.tag(TagId::AlbumArtist).toUtf8().data(), TagLib::String::UTF8));
    }

    addFrame(tags, "TRCK")->setText(QStringLiteral("%1/%2").arg(track.trackNum()).arg(track.trackCount()).toStdString());
    addFrame(tags, "TPOS")->setText(QStringLiteral("%1/%2").arg(track.discNum()).arg(track.discCount()).toStdString());
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

/************************************************
 *
 ************************************************/
void Mp3MetaDataWriter::setTrackReplayGain(float gain, float peak)
{
    using namespace TagLib::ID3v2;
    Tag *tags = mFile.ID3v2Tag(true);

    {
        UserTextIdentificationFrame *frame = new UserTextIdentificationFrame();
        frame->setDescription("replaygain_track_gain");
        frame->setText(gainToString(gain).toStdString());
        tags->addFrame(frame);
    }

    {
        UserTextIdentificationFrame *frame = new UserTextIdentificationFrame();
        frame->setDescription("replaygain_track_peak");
        frame->setText(gainToString(peak).toStdString());
        tags->addFrame(frame);
    }
}

/************************************************
 *
 ************************************************/
void Mp3MetaDataWriter::setAlbumReplayGain(float gain, float peak)
{
    using namespace TagLib::ID3v2;
    Tag *tags = mFile.ID3v2Tag(true);

    {
        UserTextIdentificationFrame *frame = new UserTextIdentificationFrame();
        frame->setDescription("replaygain_album_gain");
        frame->setText(gainToString(gain).toStdString());
        tags->addFrame(frame);
    }

    {
        UserTextIdentificationFrame *frame = new UserTextIdentificationFrame();
        frame->setDescription("replaygain_album_peak");
        frame->setText(gainToString(peak).toStdString());
        tags->addFrame(frame);
    }
}
