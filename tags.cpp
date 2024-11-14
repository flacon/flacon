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

#define GET(FIELD) !mUserTags.FIELD().isNull() ? mUserTags.FIELD() : mLoadedTags.FIELD();

/**************************************
 * AlbumTags
 **************************************/
void AlbumTags::merge(const AlbumTags &other)
{
    // clang-format off
    if (other.discCount())  mDiscCount  = other.discCount();
    if (other.discNum())    mDiscNum    = other.discNum();
    if (other.trackCount()) mTrackCount = other.trackCount();

    if (!other.mAlbum.isNull())             mAlbum          = other.mAlbum;
    if (!other.mCatalog.isNull())           mCatalog        = other.mCatalog;
    if (!other.mCdTextfile.isNull())        mCdTextfile     = other.mCdTextfile;
    if (!other.mDiscId.isNull())            mDiscId         = other.mDiscId;
    if (!other.mAlbumPerformer.isNull())    mAlbumPerformer = other.mAlbumPerformer;
    // clang-format on
}

/**************************************
 * TrackTags
 **************************************/
void TrackTags::merge(const TrackTags &other)
{
    // clang-format off
    if (other.mTrackNum)    mTrackNum = other.mTrackNum;

    if (!other.mComment.isNull())    mComment    = other.mComment;
    if (!other.mFlagsTag.isNull())   mFlagsTag   = other.mFlagsTag;
    if (!other.mDate.isNull())       mDate       = other.mDate;
    if (!other.mGenre.isNull())      mGenre      = other.mGenre;
    if (!other.mIsrc.isNull())       mIsrc       = other.mIsrc;
    if (!other.mPerformer.isNull())  mPerformer  = other.mPerformer;
    if (!other.mSongWriter.isNull()) mSongWriter = other.mSongWriter;
    if (!other.mTitle.isNull())      mTitle      = other.mTitle;
    // clang-format on
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
