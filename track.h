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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include "types.h"
#include "tags.h"
#include "cue.h"
#include "inputaudiofile.h"
#include <QDebug>

class Disc;

class Track
{
    friend class Disc;
    friend class PregapTrack;

public:
    using Getter = QString (Track::*)() const;
    using Setter = void (Track::*)(const QString &);

public:
    Track() = default;
    Track(Disc *disk, int index);
    Track(const Track &other) = default;
    Track &operator=(const Track &other) = default;

    ~Track() = default;

    const InputAudioFile &audioFile() const { return mAudiofile; }
    void                  setAudioFile(const InputAudioFile &file);
    QString               audioFileName() const { return mAudiofile.fileName(); }

    Duration duration() const;

    CueIndex cueIndex00() const { return mCueIndex00; }
    CueIndex cueIndex01() const { return mCueIndex01; }

    Disc *disk() const { return mDisk; }
    Disc *disc() const { return disk(); }

    int index() const { return mIndex; }

    bool preEmphased() const;

public:
    // Tags
    TrackNum trackNumTag() const;
    void     setTrackNumTag(int value);

    QString artistTag() const { return performerTag(); }
    QString commentTag() const;
    QString dateTag() const;
    QString genreTag() const;
    QString flagsTag() const;
    QString isrcTag() const;
    QString titleTag() const;
    QString performerTag() const;
    QString songWriterTag() const;

    void setArtistTag(const QString &value) { setPerformerTag(value); }
    void setCommentTag(const QString &value);
    void setDateTag(const QString &value);
    void setGenreTag(const QString &value);
    void setIsrcTag(const QString &value);
    void setTitleTag(const QString &value);
    void setPerformerTag(const QString &value);
    void setSongWriterTag(const QString &value);

    QString fileTag() const;

    TrackTags toTags() const;

protected:
    Tags::Track userTags() const { return mUserTags; }
    Tags::Track loadedTags() const { return mLoadedTags; }

    void setUserTags(const Tags::Track &tags) { mUserTags = tags; }
    void setLoadedTags(const Tags::Track &tags) { mLoadedTags = tags; }

private:
    Disc *mDisk  = nullptr;
    int   mIndex = -1;

    CueIndex mCueIndex00;
    CueIndex mCueIndex01;

    Tags::Track mUserTags;
    Tags::Track mLoadedTags;

    InputAudioFile mAudiofile;
};

class PregapTrack : public Track
{
public:
    PregapTrack(const Track &track);
};

// using Tracks     = QVector<Track>;
using TrackPtrList = QList<Track *>;

QDebug operator<<(QDebug debug, const Track &track);

#endif // TRACK_H
