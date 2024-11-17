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

/**************************************
 *
 **************************************/
Track::Track(Disc *disk, int index) :
    mDisk(disk),
    mIndex(index)
{
}

/**************************************
 *
 **************************************/
void Track::setAudioFile(const InputAudioFile &file)
{
    mAudiofile = file;
}

/**************************************
 *
 **************************************/
Duration Track::duration() const
{
    Cue::Track cur  = mDisk->cue().tracks().at(index());
    Cue::Track next = (index() < mDisk->tracks().count() - 1) ? mDisk->cue().tracks().at(index() + 1) : Cue::Track();

    Duration trackLen = 0;
    if (cur.cueIndex01().file() != next.cueIndex00().file()) {
        uint start = cur.cueIndex01().milliseconds();
        uint end   = audioFile().duration();
        trackLen   = end > start ? end - start : 0;
    }
    else {
        uint start = cur.cueIndex01().milliseconds();
        uint end   = next.cueIndex00().milliseconds();
        trackLen   = end > start ? end - start : 0;
    }

    Duration postGapLen = 0;
    if (next.cueIndex00().file() != next.cueIndex01().file()) {
        uint start = next.cueIndex00().milliseconds();
        uint end   = audioFile().duration();
        postGapLen = end > start ? end - start : 0;
    }
    else {
        uint start = next.cueIndex00().milliseconds();
        uint end   = next.cueIndex01().milliseconds();
        postGapLen = end > start ? end - start : 0;
    }

    /*
    qDebug() << "***************************************";
    qDebug() << "CUR:" << track.index();
    qDebug() << "  - 00 " << cur.cueIndex00.toString() << cur.cueIndex00.file();
    qDebug() << "  - 01 " << cur.cueIndex01.toString() << cur.cueIndex01.file();
    qDebug() << ".......................................";
    qDebug() << "NEXT:";
    qDebug() << "  - 00 " << next.cueIndex00.toString() << next.cueIndex00.file();
    qDebug() << "  - 01 " << next.cueIndex01.toString() << next.cueIndex01.file();
    qDebug() << ".......................................";
    qDebug() << "trackLen: " << trackLen;
    qDebug() << "postGapLen: " << postGapLen;
    qDebug() << "***************************************";
    */

    return trackLen + postGapLen;
}

/**************************************
 *
 **************************************/
bool Track::preEmphased() const
{
    return CueFlags(flagsTag()).preEmphasis;
}

/**************************************
 *
 **************************************/
QString Track::tag(TrackTags::TagId tagId) const
{
    return firstNotNullString(mUserTags.tag(tagId), mLoadedTags.tag(tagId));
}

/**************************************
 *
 **************************************/
void Track::setTag(TrackTags::TagId tagId, const QString &value)
{
    mUserTags.setTag(tagId, value);
}

/**************************************
 *
 **************************************/
TrackNum Track::trackNumTag() const
{
    return mUserTags.trackNum() > 0 ? mUserTags.trackNum() : mLoadedTags.trackNum();
}

/**************************************
 *
 **************************************/
void Track::setTrackNumTag(int value)
{
    mUserTags.setTrackNum(value);
}

/**************************************
 *
 **************************************/
QString Track::fileTag() const
{
    return mDisk->textCodec().decode(mDisk->cue().tracks().at(mIndex).fileTag());
}

/**************************************
 *
 **************************************/
TrackTags Track::toTags() const
{
    TrackTags res;
    res.setTrackNum(trackNumTag());

    for (TrackTags::TagId tagId : TrackTags::allTagId()) {
        res.setTag(tagId, this->tag(tagId));
    }

    return res;
}

/**************************************
 *
 **************************************/
QDebug operator<<(QDebug debug, const Track &track)
{
    QDebugStateSaver saver(debug);
    debug.nospace()
            << " Track {"
            << " trackNum: " << track.trackNumTag()
            << " trackCount:" << track.disk()->tracks().count()
            << " diskId:" << track.disc()->discIdTag()
            << " Artist:" << track.performerTag()
            << " Album:" << track.disc()->albumTag()
            << " Title:" << track.titleTag()
            << "}";
    return debug;
}

PregapTrack::PregapTrack(const Track &track) :
    Track(track)
{
    mLoadedTags.setTrackNum(0);
    setTrackNumTag(0);
    setTitleTag("(HTOA)");
}
