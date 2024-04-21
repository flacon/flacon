/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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

#ifndef PATTERNEXPANDER_H
#define PATTERNEXPANDER_H

#include "types.h"
#include "tags.h"
#include <QString>

class Track;

/************************************************
  %N  Number of tracks       %n  Track number
  %D  Number of discs        %d  Disc number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title
 ************************************************/
class PatternExpander
{
public:
    PatternExpander();
    PatternExpander(const Track &track);

    TrackCount trackCount() const { return mTrackCount; }
    void       setTrackCount(TrackCount value) { mTrackCount = value; }

    TrackNum trackNum() const { return mTrackNum; }
    void     setTrackNum(TrackNum value) { mTrackNum = value; }

    DiscCount discCount() const { return mDiscCount; }
    void      setDiscCount(DiscCount value) { mDiscCount = value; }

    DiscNum discNum() const { return mDiscNum; }
    void    setDiscNum(DiscNum value) { mDiscNum = value; }

    QString artist() const { return mArtist; }
    void    setArtist(const QString &value) { mArtist = value; }

    QString album() const { return mAlbum; }
    void    setAlbum(const QString &value) { mAlbum = value; }

    QString trackTitle() const { return mTrackTitle; }
    void    setTrackTtle(const QString &value) { mTrackTitle = value; }

    QString genre() const { return mGenre; }
    void    setGenre(const QString &value) { mGenre = value; }

    QString date() const { return mDate; }
    void    setDate(const QString &value) { mDate = value; }

    QString expand(const QString &pattern) const;

    static QString resultFileName(const QString &pattern, const Track *track, const QString &ext);

    static QString example(const QString &pattern);

protected:
    static int lastDirSeparattor(const QString &pattern);

private:
    TrackCount mTrackCount;
    TrackNum   mTrackNum;
    DiscCount  mDiscCount;
    DiscNum    mDiscNum;
    QString    mAlbum;
    QString    mTrackTitle;
    QString    mArtist;
    QString    mGenre;
    QString    mDate;
};

#endif // PATTERNEXPANDER_H
