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
#include "cue.h"

class Disk;

class Track: public QObject
{
    Q_OBJECT
public:

    enum Status
    {
        NotRunning  = 0,
        Canceled    = 1,
        Error       = 2,
        Aborted     = 3,
        OK          = 4,
        Splitting   = 5,
        Encoding    = 6,
        Queued      = 7,
        WaitGain    = 8,
        CalcGain    = 9,
        WriteGain   = 10
    };

    explicit Track(Disk *disk, int index);
    explicit Track(const Track &other);
    ~Track();

    QString artist() const            { return tag("PERFORMER"); }
    void setArtist(const QString &value)  { setTag("PERFORMER", value); }

    QString album() const             { return tag("ALBUM"); }
    void setAlbum(const QString &value)   { setTag("ALBUM", value); }

    QString comment() const           { return tag("COMMENT") ;}
    void setComment(const QString &value)   { setTag("COMMENT", value); }

    QString title() const             { return tag("TITLE") ;}
    void setTitle(const QString &value)   { setTag("TITLE", value); }

    QString genre() const             { return tag("GENRE") ;}
    void setGenre(const QString &value)   { setTag("GENRE", value); }

    QString date() const              { return tag("DATE") ;}
    void setDate(const QString &value)    { setTag("DATE", value); }


    QString resultFileName() const;
    QString resultFilePath() const;

    Disk *disk() const { return mDisk; }
    int index() const { return mIndex; }

    int trackNum() const;
    uint duration() const;

    CueIndex cueIndex(int indexNum) const;
    void setCueIndex(int indexNum, const CueIndex &value);

    virtual QString tag(const QString &tagName) const;
    virtual void setTag(const QString &tagName, const QString &value);

    int progress() const { return mProgress; }
    Status status() const { return mStatus; }
    void setProgress(Status status, int percent = -1);

    static QString calcFileName(const QString &pattern,
                                int trackCount,
                                int trackNum,
                                const QString &album,
                                const QString &title,
                                const QString &artist,
                                const QString &genre,
                                const QString &date,
                                const QString &fileExt);

private:
    Disk *mDisk;
    int mIndex;
    QVector<CueIndex> mCueIndexes;
    Status mStatus;
    int mProgress;

    QString calcResultFilePath() const;
    static QString expandPattern(const QString &pattern, const QHash<QChar,QString> *tokens, bool optional);
};

Q_DECLARE_METATYPE(Track::Status)


class PreGapTrack: public Track
{
public:
    explicit PreGapTrack(Disk *disk);

    virtual QString tag(const QString &tagName) const;
    virtual void setTag(const QString &tagName, const QString &value);
};

#endif // TRACK_H
