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

#include "types.h"
#include "tags.h"

#include <assert.h>
#include <QDebug>

#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "TAGS")
}

/************************************************
 *
 ************************************************/
TagValue::TagValue(const QString &value) :
    mValue(value.toUtf8()),
    mEncoded(true)
{
}

/************************************************
 *
 ************************************************/
QString TagValue::asString(const TextCodec &codec) const
{
    if (mEncoded)
        return QString::fromUtf8(mValue);

    // assert(codec != nullptr);
    if (codec.isValid()) {
        try {
            return codec.decode(mValue);
        }
        catch (const FlaconError &err) {
            qCWarning(LOG) << "Unable to convert text for" << codec.name() << ":" << err.what();
        }
    }

    return QString::fromUtf8(mValue);
}

/************************************************
 *
 ************************************************/
void TagValue::setValue(const QByteArray &value)
{
    mValue   = value;
    mEncoded = false;
}

/************************************************
 *
 ************************************************/
void TagValue::setValue(const QString &value)
{
    mValue   = value.toUtf8();
    mEncoded = true;
}

/************************************************
 *
 ************************************************/
bool TagValue::operator==(const TagValue &other) const
{
    return this->mEncoded == other.mEncoded && this->mValue == other.mValue;
}

/************************************************
 *
 ************************************************/
bool TrackTags::operator==(const TrackTags &other) const
{
    return mTags == other.mTags;
}

/************************************************
 *
 ************************************************/
QString TrackTags::tag(const TagId &tagId) const
{
    return mTags.value(static_cast<int>(tagId)).asString(mTextCodec);
}

/************************************************
 *
 ************************************************/
QByteArray TrackTags::tagData(const TagId &tagId) const
{
    return mTags.value(static_cast<int>(tagId)).value();
}

/************************************************
 *
 ************************************************/
TagValue TrackTags::tagValue(TagId tagId) const
{
    return mTags.value(static_cast<int>(tagId));
}

/************************************************
 *
 ************************************************/
void TrackTags::setTag(const TagId &tagId, const QString &value)
{
    mTags.insert(static_cast<int>(tagId), TagValue(value));
}

/************************************************
 *
 ************************************************/
void TrackTags::setTag(const TagId &tagId, const QByteArray &value)
{
    mTags.insert(static_cast<int>(tagId), TagValue(value, false));
}

/************************************************
 *
 ************************************************/
void TrackTags::setTag(TagId tagId, const TagValue &value)
{
    mTags.insert(static_cast<int>(tagId), value);
}

/************************************************
 *
 ************************************************/
void TrackTags::setCodec(const TextCodec &value)
{
    mTextCodec = value;
}

/************************************************
 *
 ************************************************/
CueIndex TrackTags::cueIndex(int indexNum) const
{
    if (indexNum < mCueIndexes.length())
        return mCueIndexes.at(indexNum);

    return CueIndex();
}

/************************************************
 *
 ************************************************/
void TrackTags::setCueIndex(int indexNum, const CueIndex &value)
{
    if (indexNum >= mCueIndexes.length())
        mCueIndexes.resize(indexNum + 1);

    mCueIndexes[indexNum] = value;
}

/************************************************
 *
 ************************************************/
int TrackTags::intTag(const TagId &tagId, int defaultValue) const
{
    bool ok;
    int  res = tag(tagId).toInt(&ok);

    if (ok)
        return res;

    return defaultValue;
}
/************************************************
 *
 ************************************************/
void TrackTags::setIntTag(const TagId &tagId, int value)
{
    setTag(tagId, QString::number(value));
}
