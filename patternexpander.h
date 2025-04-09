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
#include <QCoreApplication>

class Track;

/************************************************
  %N  Number of tracks       %n  Track number
  %D  Number of discs        %d  Disc number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title            %C  Catalog
 ************************************************/
class PatternExpander
{
    Q_DECLARE_TR_FUNCTIONS(PatternExpander)
public:
    enum class Mode {
        Album,
        Track,
    };

public:
    PatternExpander();
    PatternExpander(const AlbumTags &albumTags, const TrackTags &trackTags, const TrackTags &firstTrackTags);
    PatternExpander(const Track &track);

    QString expand(const QString &pattern) const;

    static QString resultFileName(const QString &pattern, const Track *track, const QString &ext);

    static QString example(const QString &pattern);
    static QString toolTip();

    const AlbumTags &albumTags() const { return mAlbumTags; }
    AlbumTags       &albumTags() { return mAlbumTags; }

    const TrackTags trackTags() const { return mTrackTags; }
    TrackTags       trackTags() { return mTrackTags; }

protected:
    static int lastDirSeparattor(const QString &pattern);
    QString    expand(const QString &pattern, Mode mode) const;

private:
    AlbumTags mAlbumTags;
    TrackTags mTrackTags;
    TrackTags mFirstTrackTags;
};

#endif // PATTERNEXPANDER_H
