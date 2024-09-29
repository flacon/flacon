/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#include "track.h"
#include "disc.h"

/************************************************

************************************************/
Track::Track(Disc *disc, int index) :
    mDisc(disc),
    mIndex(index)
{
}

/************************************************

 ************************************************/
void Track::setAudioFile(const InputAudioFile &file)
{
    mAudiofile = file;
}

/************************************************
 *
 ************************************************/
QString Track::tag(const TagId &tagId) const
{
    return mDisc->trackTag(mIndex, tagId);
}

/************************************************
 *
 ************************************************/
// QByteArray Track::tagData(const TagId &tagId) const
// {
//     return mTags.tagData(tagId);
// }

/************************************************
 *
 ************************************************/
// TagValue Track::tagValue(TagId tagId) const
// {
//     return mTags.tagValue(tagId);
// }

/************************************************
 *
 ************************************************/
void Track::setTag(const TagId &tagId, const QString &value)
{
    mDisc->setTrackTag(mIndex, tagId, value);
}

/************************************************
 *
 ************************************************/
// void Track::setTag(TagId tagId, const TagValue &value)
// {
//     mTags.setTag(tagId, value);
//     if (mDisc) {
//         mDisc->trackChanged(tagId);
//     }
// }

/************************************************
 *
 ************************************************/
// bool Track::operator==(const Track &other) const
// {
//     // clang-format off
//     return
//         mDisc           == other.mDisc &&
//         mAudiofile      == other.mAudiofile &&
//         mTags           == other.mTags;
//     // clang-format on
// }

/************************************************
 *
 ************************************************/
TrackNum Track::trackNum() const
{
    bool ok;
    int  res = tag(TagId::TrackNum).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setTrackNum(TrackNum value)
{
    setTag(TagId::TrackNum, QString::number(value));
}

/************************************************
 *
 ************************************************/
TrackNum Track::trackCount() const
{
    bool ok;
    int  res = tag(TagId::TrackCount).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setTrackCount(TrackNum value)
{
    setTag(TagId::TrackCount, QString::number(value));
}

/************************************************
 *
 ************************************************/
DiscNum Track::discNum() const
{
    bool ok;
    int  res = tag(TagId::DiscNum).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setDiscNum(DiscNum value)
{
    setTag(TagId::DiscNum, QString::number(value));
}

/************************************************
 *
 ************************************************/
DiscNum Track::discCount() const
{
    bool ok;
    int  res = tag(TagId::DiscCount).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setDiscCount(DiscNum value)
{
    setTag(TagId::DiscCount, QString::number(value));
}

/************************************************
 *
 ************************************************/
Duration Track::duration() const
{
    return mDisc ? mDisc->trackDuration(*this) : 0;
}

/************************************************
 *
 ************************************************/
CueIndex Track::cueIndex00() const
{
    return mDisc->trackCueIndex00(mIndex);
}

/************************************************
 *
 ************************************************/
CueIndex Track::cueIndex01() const
{
    return mDisc->trackCueIndex01(mIndex);
}

/************************************************
 *
 ************************************************/
bool Track::preEmphased() const
{
    return CueFlags(tag(TagId::Flags)).preEmphasis;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug debug, const Track &track)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << " Track {"
                    << " trackNum: " << track.trackNum()
                    << " trackCount:" << track.trackCount()
                    << " diskId:" << track.discId()
                    << " Artist:" << track.artist()
                    << " Album:" << track.album()
                    << " Title:" << track.tags().title()
                    << "}";
    return debug;
}

/**************************************
 * Track::Tags
 **************************************/
Track::Tags::Tags(Track *track) :
    mTrack(track)
{
}

QString Track::Tags::artist() const
{
    if (!mArtist.isNull()) {
        return mArtist;
    }

    return mTrack->disc()->tags().artist();
}

QString Track::Tags::title() const
{
    return mTitle;
}

void Track::Tags::setTitle(const QString &value)
{
    mTitle        = value;
    mTitleChanged = true;
}

void Track::Tags::setTrackNum(int value)
{
    mTrackNum        = value;
    mTrackNumChanged = value;
}

void Track::Tags::initFromInternetTags(const InternetTags::Track &tags)
{
    // clang-format off
    if (!mTitleChanged) mTitle    = tags.title();
    if (!mTrackNum)     mTrackNum = tags.trackNum();
    // clang-format on
}

void Track::Tags::initFromCue(const Cue::Track &cue, const TextCodec &textCodec)
{
    // clang-format off
    if (!mTitleChanged) mTitle    = textCodec.decode(cue.title());
    if (!mTrackNum)     mTrackNum = cue.trackNum;
    // clang-format on
}
