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

#include "wvmetadatawriter.h"
#include <taglib/apetag.h>

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(LOG, "WvMetadataWriter")
}

/************************************************

 ************************************************/
WvMetadataWriter::WvMetadataWriter(const QString &filePath) :
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
void WvMetadataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setTags(const Track &track)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeTags(tags, track);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setEmbeddedCue(const QString &)
{
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setCoverImage(const CoverImage &image)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeCoverImage(tags, image);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setTrackReplayGain(float gain, float peak)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeTrackReplayGain(tags, gain, peak);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setAlbumReplayGain(float gain, float peak)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeAlbumReplayGain(tags, gain, peak);
}
