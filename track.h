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
#include "inputaudiofile.h"
#include <QDebug>

class Disc;

class Track
{
    friend class Disc;

public:
    Track() = default;
    Track(Disc *disc, int index);
    Track(const Track &other) = default;
    Track &operator=(const Track &other) = default;

    ~Track() = default;

    const InputAudioFile &audioFile() const { return mAudiofile; }
    void                  setAudioFile(const InputAudioFile &file);
    QString               audioFileName() const { return mAudiofile.fileName(); }

    QString tag(const TagId &tagId) const;
    // QByteArray tagData(const TagId &tagId) const;
    // TagValue   tagValue(TagId tagId) const;
    void setTag(const TagId &tagId, const QString &value);

    // void setTag(TagId tagId, const TagValue &value);

    // bool operator==(const Track &other) const;

    QString artist() const { return tag(TagId::Artist); }
    void    setArtist(const QString &value) { setTag(TagId::Artist, value); }

    QString album() const { return tag(TagId::Album); }
    void    setAlbum(const QString &value) { setTag(TagId::Album, value); }

    QString albumArtist() const { return tag(TagId::AlbumArtist); }
    void    setAlbumArtist(const QString &value) { setTag(TagId::AlbumArtist, value); }

    QString comment() const { return tag(TagId::Comment); }
    void    setComment(const QString &value) { setTag(TagId::Comment, value); }

    QString title() const { return tag(TagId::Title); }
    void    setTitle(const QString &value) { setTag(TagId::Title, value); }

    QString genre() const { return tag(TagId::Genre); }
    void    setGenre(const QString &value) { setTag(TagId::Genre, value); }

    QString date() const { return tag(TagId::Date); }
    void    setDate(const QString &value) { setTag(TagId::Date, value); }

    QString discId() const { return tag(TagId::DiscId); }

    TrackNum trackNum() const;
    void     setTrackNum(TrackNum value);

    TrackNum trackCount() const;
    void     setTrackCount(TrackNum value);

    DiscNum discNum() const;
    void    setDiscNum(DiscNum value);

    DiscNum discCount() const;
    void    setDiscCount(DiscNum value);

    Duration duration() const;

    CueIndex cueIndex00() const;
    CueIndex cueIndex01() const;

    Disc *disc() const { return mDisc; }

    int index() const { return mIndex; }

    bool preEmphased() const;

private:
    Disc          *mDisc  = nullptr;
    int            mIndex = -1;
    InputAudioFile mAudiofile;
};

using Tracks       = QVector<Track>;
using TrackPtrList = QList<Track *>;

QDebug operator<<(QDebug debug, const Track &track);

#endif // TRACK_H
