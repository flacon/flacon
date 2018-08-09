/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2018
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


#ifndef TAGS_H
#define TAGS_H

#include "types.h"
#include <QByteArray>
#include <QString>
#include <QHash>
#include <QVector>

class QTextCodec;

enum class TagId
{
    Album,
    Catalog,
    CDTextfile,
    Comment,
    Date,
    Flags,
    Genre,
    ISRC,
    Performer,
    SongWriter,
    Title,
    DiscId,
    File,
    Disknum,
    CueFile,
    StartTrackNum
};


class TagValue
{
public:
    TagValue():
        mEncoded(false)
    {
    }

    TagValue(const QByteArray &val, bool encoded):
        mValue(val),
        mEncoded(encoded)
    {
    }

    bool encoded() const { return mEncoded; }
    QString asString(const QTextCodec *codec) const;

    const QByteArray &value() const { return mValue; }
    void setValue(const QByteArray &value);
    void setValue(const QString &value);

    bool operator ==(const TagValue &other) const;

private:
    QByteArray mValue;
    bool mEncoded;
};

class TrackTags
{
public:
    TrackTags();
    TrackTags(const TrackTags &other);
    TrackTags &operator=(const TrackTags &other);
    virtual ~TrackTags();

    QString artist() const            { return performer(); }
    void setArtist(const QString &value)  { setPerformer(value); }

    QString performer() const            { return tag(TagId::Performer); }
    void setPerformer(const QString &value)  { setTag(TagId::Performer, value); }


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

    QString tag(const TagId &tagID) const;
    QByteArray tagData(const TagId &tagID) const;
    void setTag(const TagId &tagID, const QString &value);
    void setTag(const TagId &tagID, const QByteArray &value);

    QString codecName() const;
    void setCodecName(const QString &value);
    const QTextCodec *codec() const { return mTextCodec; }

    bool operator ==(const TrackTags &other) const;

private:
    QHash<int, TagValue> mTags;
    QTextCodec *mTextCodec;
};


class DiskTags: public QVector<TrackTags>
{
public:
    DiskTags();
    explicit DiskTags(int size);
    DiskTags(const DiskTags &other);
    DiskTags& operator=(const DiskTags &other);

    QString uri() const { return mUri; }
    void setUri(const QString &value) { mUri = value; }

    QString title() const;
    void setTitle(const QByteArray &value);
    void setTitle(const QString &value);

private:
public:
    QString mUri;
    TagValue mTitle;
};


class UcharDet
{

public:
    UcharDet();
    ~UcharDet();

    void add(const TrackTags &track);
    UcharDet& operator<<(const TrackTags &track);


    QString textCodecName() const;
    QTextCodec* textCodec() const;

private:
    struct Data;
    Data *mData;
};

QTextCodec *determineTextCodec(const QVector<TrackTags*> tracks);

#endif // TAGS_H
