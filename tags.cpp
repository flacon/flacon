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
#include "disc.h"
#include <QDebug>
#include <QMetaEnum>

#define GET(FIELD) !mUserTags.FIELD().isNull() ? mUserTags.FIELD() : mLoadedTags.FIELD();

/**************************************
 * AlbumTags
 **************************************/
QList<AlbumTags::TagId> AlbumTags::allTagId()
{
    QMetaEnum e = QMetaEnum::fromType<TagId>();

    QList<TagId> res;
    for (int i = 0; i < e.keyCount(); i++) {
        res << TagId(e.value(i));
    }
    return res;
}

/**************************************
 *
 **************************************/
void AlbumTags::merge(const AlbumTags &other)
{
    // clang-format off
    if (other.discCount())  mDiscCount  = other.discCount();
    if (other.discNum())    mDiscNum    = other.discNum();
    if (other.trackCount()) mTrackCount = other.trackCount();
    // clang-format on

    for (TagId tagId : other.mTags.keys()) {
        setTag(tagId, other.tag(tagId));
    }
}

/**************************************
 * TrackTags
 **************************************/
QList<TrackTags::TagId> TrackTags::allTagId()
{
    QMetaEnum e = QMetaEnum::fromType<TagId>();

    QList<TrackTags::TagId> res;
    for (int i = 0; i < e.keyCount(); i++) {
        res << TagId(e.value(i));
    }
    return res;
}

/**************************************
 *
 **************************************/
void TrackTags::merge(const TrackTags &other)
{
    if (other.mTrackNum)
        mTrackNum = other.mTrackNum;

    for (TrackTags::TagId tagId : other.mTags.keys()) {
        setTag(tagId, other.tag(tagId));
    }
}

/**************************************
 * Tags
 **************************************/
void Tags::merge(const Tags &other)
{
    AlbumTags::merge(other);

    int i = -1;
    for (TrackTags &t : mTracks) {
        i++;
        t.merge(other.tracks().at(i));
    }
}

/************************************************
 *
 ************************************************/
void Tags::resize(int size)
{
    mTracks.resize(size);
}

/**************************************
 *
 **************************************/
QDebug operator<<(QDebug debug, const Tags::Track &track)
{

    QDebugStateSaver saver(debug);
    debug.nospace()
            << "TrackTags {"
            << " Title:" << track.title()
            << " Performer:" << track.performer()
            << " SongWriter:" << track.songWriter()
            << " Date: " << track.date()
            << " ISRC:" << track.isrc()
            << "}";
    return debug;
}
