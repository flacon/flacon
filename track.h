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

#include "types.h"
#include "tags.h"

class Disk;

class Track: public TrackTags
{
    friend class Disk;
    friend class CueReader;
public:
    Track();
    Track(const Track &other);
    Track &operator =(const Track &other);
    void setTags(const TrackTags &tags);
    ~Track();

    QString resultFileName() const;
    QString resultFilePath() const;

    Duration duration() const { return mDuration; }

    CueIndex cueIndex(int indexNum) const;
    void setCueIndex(int indexNum, const CueIndex &value);


    static QString calcFileName(const QString &pattern,
                                int trackCount,
                                int trackNum,
                                const QString &album,
                                const QString &title,
                                const QString &artist,
                                const QString &genre,
                                const QString &date,
                                const QString &fileExt);

    TrackNum trackNum() const { return mTrackNum; }
    void setTrackNum(TrackNum value) { mTrackNum = value; }

    TrackNum trackCount() const { return mTrackCount; }
    void setTrackCount(TrackNum value) { mTrackCount = value; }

    bool operator ==(const Track &other) const;

    QString cueFileName() const { return mCueFileName; }

private:
    QVector<CueIndex> mCueIndexes;
    TrackNum mTrackNum;
    TrackNum mTrackCount;
    Duration mDuration;
    QString mCueFileName;

    QString calcResultFilePath() const;
    static QString expandPattern(const QString &pattern, const QHash<QChar,QString> *tokens, bool optional);
};


class Tracks: public QVector<Track>
{
public:
    Tracks();
    explicit Tracks(int size);
    Tracks(const Tracks &other);
    Tracks &operator=(const Tracks &other);
    virtual ~Tracks();

    QString uri() const { return mUri; }
    void setUri(const QString &value) { mUri = value ;}

    QString title() const;
    void setTitle(const QByteArray &value);
    void setTitle(const QString &value);

private:
    QString mUri;
    TagValue mTitle;
};

#endif // TRACK_H
