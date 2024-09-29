/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2018
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

#include "types.h"
#include "tags.h"
#include <QDebug>

/************************************************
 *
 ************************************************/
QString Tags::albumTag(TagId tagId) const
{
    return mAlbumTags.value(tagId, "");
}

/************************************************
 *
 ************************************************/
void Tags::setAlbumTag(TagId tagId, const QString &value)
{
    mAlbumTags[tagId] = value;
}

/************************************************
 *
 ************************************************/
QString Tags::trackTag(int trackIndex, TagId tagId) const
{
    if (trackIndex < mTrackTags.size()) {
        return mTrackTags.at(trackIndex).value(tagId, "");
    }

    return "";
}

/************************************************
 *
 ************************************************/
void Tags::setTrackTag(int trackIndex, TagId tagId, const QString &value)
{
    if (trackIndex >= mTrackTags.size()) {
        mTrackTags.resize(trackIndex + 1);
    }

    mTrackTags[trackIndex][tagId] = value;
}

/************************************************
 *
 ************************************************/
bool Tags::containsTrackTag(int trackIndex, TagId tagId) const
{
    if (trackIndex < mTrackTags.size()) {
        return mTrackTags.at(trackIndex).contains(tagId);
    }

    return false;
}

/************************************************
 *
 ************************************************/
void Tags::resize(int size)
{
    mTrackTags.resize(size);
}

/************************************************
 *
 ************************************************/
bool Tags::compareTags(const Tags &other) const
{
    return mTrackTags == other.mTrackTags;
}

/************************************************
 *
 ************************************************/
QByteArray RawTags::trackTag(int trackIndex, TagId tagId) const
{
    if (trackIndex < mTrackTags.size()) {
        return mTrackTags.at(trackIndex).value(tagId, {});
    }

    return {};
}

/************************************************
 *
 ************************************************/
void RawTags::setTrackTag(int trackIndex, TagId tagId, const QByteArray &value)
{
    if (trackIndex >= mTrackTags.size()) {
        mTrackTags.resize(trackIndex + 1);
    }

    mTrackTags[trackIndex][tagId] = value;
}

/************************************************
 *
 ************************************************/
bool RawTags::containsTrackTag(int trackIndex, TagId tagId) const
{
    if (trackIndex < mTrackTags.size()) {
        return mTrackTags.at(trackIndex).contains(tagId);
    }

    return false;
}
