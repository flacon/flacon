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

#include "flacmetadatawriter.h"
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "FlacMetadataWriter")
}

/************************************************

 ************************************************/
FlacMetadataWriter::FlacMetadataWriter(const Profile &profile, const QString &filePath) :
    MetadataWriter(profile, filePath),
    mFile(filePath.toLocal8Bit(), false)
{
    if (!mFile.isValid()) {
        qCWarning(LOG) << Q_FUNC_INFO << "file is invalid";
        throw FlaconError("Can't open file");
    }
}

/************************************************

 ************************************************/
void FlacMetadataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setTags(const Track &track)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphTags(tags, track);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setEmbeddedCue(const QString &cue)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphEmbeddedCue(tags, cue);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setCoverImage(const CoverImage &image)
{
    if (!image.isEmpty()) {
        TagLib::ByteVector dt(image.data().data(), image.data().size());

        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setData(dt);
        pic->setMimeType(image.mimeType().toStdString());
        pic->setWidth(image.size().width());
        pic->setHeight(image.size().height());
        pic->setColorDepth(image.depth());

        mFile.addPicture(pic);
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setTrackReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphTrackReplayGain(tags, gain, peak);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setAlbumReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphAlbumReplayGain(tags, gain, peak);
}
