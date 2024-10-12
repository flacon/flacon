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
TrackNum Track::trackNumTag() const
{
    return mUserTags.trackNum() > 0 ? mUserTags.trackNum() : mLoadedTags.trackNum();
}

/**************************************
 *
 **************************************/
QString Track::commentTag() const
{
    if (!mUserTags.comment().isNull()) {
        return mUserTags.comment();
    }

    if (!mLoadedTags.comment().isNull()) {
        return mLoadedTags.comment();
    }

    return mDisk->commentTag();
}

/**************************************
 *
 **************************************/
QString Track::dateTag() const
{
    if (!mUserTags.date().isNull()) {
        return mUserTags.date();
    }

    if (!mLoadedTags.date().isNull()) {
        return mLoadedTags.date();
    }

    return mDisk->dateTag();
}

/**************************************
 *
 **************************************/
QString Track::flagsTag() const
{
    return mLoadedTags.flagsTag();
}

/**************************************
 *
 **************************************/
QString Track::isrcTag() const
{
    if (!mUserTags.isrc().isNull()) {
        return mUserTags.isrc();
    }
    return mLoadedTags.isrc();
}

/**************************************
 *
 **************************************/
QString Track::titleTag() const
{
    if (!mUserTags.title().isNull()) {
        return mUserTags.title();
    }
    return mLoadedTags.title();
}

/**************************************
 *
 **************************************/
QString Track::performerTag() const
{
    return firstNotNullString(mUserTags.performer(), mLoadedTags.performer());
}

/**************************************
 *
 **************************************/
QString Track::songWriterTag() const
{
    if (!mUserTags.songWriter().isNull()) {
        return mUserTags.songWriter();
    }
    return mLoadedTags.songWriter();
}

/**************************************
 *
 **************************************/
void Track::setCommentTag(const QString &value)
{
    mUserTags.setComment(value);
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
void Track::setDateTag(const QString &value)
{
    mUserTags.setDate(value);
}

/**************************************
 *
 **************************************/
void Track::setIsrcTag(const QString &value)
{
    mUserTags.setIsrc(value);
}

/**************************************
 *
 **************************************/
void Track::setTitleTag(const QString &value)
{
    mUserTags.setTitle(value);
}

/**************************************
 *
 **************************************/
void Track::setPerformerTag(const QString &value)
{
    mUserTags.setPerformer(value);
}

/**************************************
 *
 **************************************/
void Track::setSongWriterTag(const QString &value)
{
    mUserTags.setSongWriter(value);
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

    res.setArtist(artistTag());
    res.setComment(commentTag());
    res.setDate(dateTag());
    res.setFlagsTag(flagsTag());
    res.setIsrc(isrcTag());
    res.setTitle(titleTag());
    res.setPerformer(performerTag());
    res.setSongWriter(songWriterTag());
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
