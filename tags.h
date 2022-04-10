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

class TagValue
{
public:
    TagValue() :
        mEncoded(false)
    {
    }

    TagValue(const QByteArray &val, bool encoded) :
        mValue(val),
        mEncoded(encoded)
    {
    }

    explicit TagValue(const QString &val);

    bool    encoded() const { return mEncoded; }
    QString asString(const QTextCodec *codec) const;

    const QByteArray &value() const { return mValue; }
    void              setValue(const QByteArray &value);
    void              setValue(const QString &value);

    bool operator==(const TagValue &other) const;

    bool isEmpty() const { return mValue.isEmpty(); }

private:
    QByteArray mValue;
    bool       mEncoded;
};

class TrackTags
{
public:
    TrackTags()                       = default;
    TrackTags(const TrackTags &other) = default;
    TrackTags &operator=(const TrackTags &other) = default;

    bool operator==(const TrackTags &other) const;

    QString    tag(const TagId &tagId) const;
    QByteArray tagData(const TagId &tagId) const;
    TagValue   tagValue(TagId tagId) const;
    void       setTag(const TagId &tagId, const QString &value);
    void       setTag(const TagId &tagId, const QByteArray &value);
    void       setTag(TagId tagId, const TagValue &value);

    QString           codecName() const;
    void              setCodecName(const QString &value);
    const QTextCodec *codec() const { return mTextCodec; }

    QString artist() const { return tag(TagId::Artist); }
    void    setArtist(const QString &value) { setTag(TagId::Artist, value); }

    QString album() const { return tag(TagId::Album); }
    void    setAlbum(const QString &value) { setTag(TagId::Album, value); }

    QString comment() const { return tag(TagId::Comment); }
    void    setComment(const QString &value) { setTag(TagId::Comment, value); }

    QString title() const { return tag(TagId::Title); }
    void    setTitle(const QString &value) { setTag(TagId::Title, value); }

    QString genre() const { return tag(TagId::Genre); }
    void    setGenre(const QString &value) { setTag(TagId::Genre, value); }

    QString date() const { return tag(TagId::Date); }
    void    setDate(const QString &value) { setTag(TagId::Date, value); }

    QString discId() const { return tag(TagId::DiscId); }

    TrackNum trackNum() const { return intTag(TagId::TrackNum, 1); };
    void     setTrackNum(TrackNum value) { setIntTag(TagId::TrackNum, value); }

    TrackNum trackCount() const { return intTag(TagId::TrackCount, 1); }
    void     setTrackCount(TrackNum value) { setIntTag(TagId::TrackCount, value); }

    DiscNum discNum() const { return intTag(TagId::DiscNum, 1); }
    void    setDiscNum(DiscNum value) { setIntTag(TagId::DiscNum, value); }

    DiscNum discCount() const { return intTag(TagId::DiscCount, 1); }
    void    setDiscCount(DiscNum value) { return setIntTag(TagId::DiscCount, value); }

    CueIndex cueIndex(int indexNum) const;
    void     setCueIndex(int indexNum, const CueIndex &value);

private:
    QHash<int, TagValue> mTags;
    QTextCodec *         mTextCodec;
    QVector<CueIndex>    mCueIndexes;

    int  intTag(const TagId &tagId, int defaultValue = 1) const;
    void setIntTag(const TagId &tagId, int value);
};

using DiskTags = QVector<TrackTags>;

#endif // TAGS_H
