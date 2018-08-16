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
#include <QTextCodec>
#include <QDebug>


#define ENC_CODEC "UTF-8"


/************************************************
 *
 ************************************************/
static QTextCodec *encCodec()
{
    static QTextCodec *res = QTextCodec::codecForName(ENC_CODEC);
    return res;
}



/************************************************
 *
 ************************************************/
TagValue::TagValue(const QString &value):
    mValue(encCodec()->fromUnicode(value)),
    mEncoded(true)
{
}


/************************************************
 *
 ************************************************/
QString TagValue::asString(const QTextCodec *codec) const
{
    if (mEncoded)
        return encCodec()->toUnicode(mValue);

    assert(codec != nullptr);
    if (codec)
        return codec->toUnicode(mValue);

    return encCodec()->toUnicode(mValue);
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
    mValue   = encCodec()->fromUnicode(value);
    mEncoded = true;
}


/************************************************
 *
 ************************************************/
bool TagValue::operator ==(const TagValue &other) const
{
    return this->mEncoded == other.mEncoded &&
           this->mValue   == other.mValue;
}


/************************************************
 *
 ************************************************/
TrackTags::TrackTags():
    mTextCodec(nullptr)
{
}


/************************************************
 *
 ************************************************/
TrackTags::TrackTags(const TrackTags &other):
    mTags(other.mTags),
    mTextCodec(other.mTextCodec)
{
}


/************************************************
 *
 ************************************************/
TrackTags &TrackTags::operator=(const TrackTags &other)
{
    mTags      = other.mTags;
    mTextCodec = other.mTextCodec;

    return *this;
}


/************************************************
 *
 ************************************************/
TrackTags::~TrackTags()
{

}


/************************************************
 *
 ************************************************/
QString TrackTags::tag(const TagId &tagID) const
{
    return mTags.value(static_cast<int>(tagID)).asString(mTextCodec);
}


/************************************************
 *
 ************************************************/
QByteArray TrackTags::tagData(const TagId &tagID) const
{
    return mTags.value(static_cast<int>(tagID)).value();
}


/************************************************
 *
 ************************************************/
void TrackTags::setTag(const TagId &tagID, const QString &value)
{
    mTags.insert(static_cast<int>(tagID), TagValue(encCodec()->fromUnicode(value), true));
}


/************************************************
 *
 ************************************************/
void TrackTags::setTag(const TagId &tagID, const QByteArray &value)
{
    mTags.insert(static_cast<int>(tagID), TagValue(value, false));
}


/************************************************
 *
 ************************************************/
QString TrackTags::codecName() const
{
    if (mTextCodec)
        return mTextCodec->name();

    return "";
}


/************************************************
 *
 ************************************************/
void TrackTags::setCodecName(const QString &value)
{
    if (!value.isEmpty())
        mTextCodec = QTextCodec::codecForName(value.toLatin1());
    else
        mTextCodec = nullptr;
}


/************************************************
 *
 ************************************************/
bool TrackTags::operator ==(const TrackTags &other) const
{
    return this->mTags == other.mTags;
}
