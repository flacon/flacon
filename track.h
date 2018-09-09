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

class Disk;

class Track: public QObject
{
    Q_OBJECT
    friend class Disk;
    friend class CueReader;
public:
    Track();
    Track(const Track &other);
    Track &operator =(const Track &other);
    void setTags(const Track &other);
    ~Track();

    QString tag(const TagId &tagId) const;
    QByteArray tagData(const TagId &tagId) const;
    TagValue tagValue(TagId tagId) const;
    void setTag(const TagId &tagId, const QString &value);
    void setTag(const TagId &tagId, const QByteArray &value);
    void setTag(TagId tagId, const TagValue &value);


    QString codecName() const;
    void setCodecName(const QString &value);
    const QTextCodec *codec() const { return mTextCodec; }

    bool operator ==(const Track &other) const;


    QString artist() const            { return tag(TagId::Artist); }
    void setArtist(const QString &value)  { setTag(TagId::Artist, value); }

    QString album() const             { return tag(TagId::Album); }
    void setAlbum(const QString &value)   { setTag(TagId::Album, value); }

    QString comment() const           { return tag(TagId::Comment) ;}
    void setComment(const QString &value)   { setTag(TagId::Comment, value); }

    QString title() const             { return tag(TagId::Title) ;}
    void setTitle(const QString &value)   { setTag(TagId::Title, value); }

    QString genre() const             { return tag(TagId::Genre) ;}
    void setGenre(const QString &value)   { setTag(TagId::Genre, value); }

    QString date() const              { return tag(TagId::Date) ;}
    void setDate(const QString &value)    { setTag(TagId::Date, value); }

    QString diskId() const              { return tag(TagId::DiscId) ;}

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

    QString cueFileName() const { return mCueFileName; }
    void setCueFileName(const QString &value) { mCueFileName = value; }

signals:
    void tagChanged(TagId tagId);

private:
    QHash<int, TagValue> mTags;
    QTextCodec *mTextCodec;
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

class UcharDet
{

public:
    UcharDet();
    UcharDet(const UcharDet &) =delete;
    UcharDet &operator =(const UcharDet &) =delete;
    ~UcharDet();

    void add(const Track &track);
    UcharDet& operator<<(const Track &track);


    QString textCodecName() const;
    QTextCodec* textCodec() const;

private:
    struct Data;
    Data *mData;
};

QTextCodec *determineTextCodec(const QVector<Track*> tracks);

#endif // TRACK_H
