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
#include <QMap>
#include <QVector>
#include "textcodec.h"

struct TagSet
{
    QString uri;
    QString title;
};

class Tags
{
public:
    QString albumTag(TagId tagId) const;
    void    setAlbumTag(TagId tagId, const QString &value);

    QString trackTag(int trackIndex, TagId tagId) const;
    void    setTrackTag(int trackIndex, TagId tagId, const QString &value);

    bool containsTrackTag(int trackIndex, TagId tagId) const;

    bool isEmpty() const { return mTrackTags.isEmpty(); }
    int  trackCount() const { return mTrackTags.count(); }
    void resize(int size);

    bool compareTags(const Tags &other) const;

private:
    QMap<TagId, QString>          mAlbumTags;
    QVector<QMap<TagId, QString>> mTrackTags;
};

class InternetTags : public Tags
{
public:
    QString uri() const { return mUri; }
    QString name() const { return mName; }

    void setUri(const QString &value) { mUri = value; }
    void setName(const QString &value) { mName = value; }

private:
    QString mUri;
    QString mName;
};

class RawTags
{
public:
    QByteArray trackTag(int trackIndex, TagId tagId) const;
    void       setTrackTag(int trackIndex, TagId tagId, const QByteArray &value);

    bool containsTrackTag(int trackIndex, TagId tagId) const;

private:
    QVector<QMap<TagId, QByteArray>> mTrackTags;
};

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
    QString asString(const TextCodec &codec) const;

    const QByteArray &value() const { return mValue; }
    void              setValue(const QByteArray &value);
    void              setValue(const QString &value);

    bool operator==(const TagValue &other) const;

    bool isEmpty() const { return mValue.isEmpty(); }

private:
    QByteArray mValue;
    bool       mEncoded;
};

#endif // TAGS_H
