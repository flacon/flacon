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

#include <uchardet.h>

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
QString TagValue::asString(const QTextCodec *codec) const
{
    if (mEncoded)
        return encCodec()->toUnicode(mValue);

    assert(codec != nullptr);
    if (codec)
        return codec->toUnicode(mValue);

    return codec->toUnicode(mValue);
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
QTextCodec *determineTextCodec(const QVector<TrackTags*> tracks)
{
    QTextCodec *res;
    uchardet_t uc = uchardet_new();

    foreach(const TrackTags *track, tracks)
    {
        const QByteArray &performer = track->tagData(TagId::Performer);
        const QByteArray &title     = track->tagData(TagId::Title);

        uchardet_handle_data(uc, performer.data(), performer.length());
        uchardet_handle_data(uc, title.data(),     title.length());
    }

    uchardet_data_end(uc);
    res = QTextCodec::codecForName(uchardet_get_charset(uc));
    if (!res)
        res = QTextCodec::codecForName("UTF-8");

    uchardet_delete(uc);

    return res;
}



/************************************************
 *
 ************************************************/
struct UcharDet::Data
{
    uchardet_t mUchcharDet;

};


/************************************************
 *
 ************************************************/
UcharDet::UcharDet():
    mData(new Data())
{
    mData->mUchcharDet = uchardet_new();
}


/************************************************
 *
 ************************************************/
UcharDet::~UcharDet()
{
    uchardet_delete(mData->mUchcharDet);
    delete mData;
}


/************************************************
 *
 ************************************************/
UcharDet &UcharDet::operator<<(const TrackTags &track)
{
    const QByteArray &performer = track.tagData(TagId::Performer);
    const QByteArray &title     = track.tagData(TagId::Title);

    uchardet_handle_data(mData->mUchcharDet, performer.data(), performer.length());
    uchardet_handle_data(mData->mUchcharDet, title.data(),     title.length());

    return *this;
}


/************************************************
 *
 ************************************************/
QString UcharDet::textCodecName() const
{
    return textCodec()->name();
}


/************************************************
 *
 ************************************************/
QTextCodec *UcharDet::textCodec() const
{
    uchardet_data_end(mData->mUchcharDet);
    QTextCodec *res = QTextCodec::codecForName(uchardet_get_charset(mData->mUchcharDet));
    if (!res)
        res = QTextCodec::codecForName("UTF-8");

    return res;
}


/************************************************
 *
 ************************************************/
DiskTags::DiskTags():
    QVector<TrackTags>()
{

}


/************************************************
 *
 ************************************************/
DiskTags::DiskTags(int size):
    QVector<TrackTags>(size)
{

}


/************************************************
 *
 ************************************************/
DiskTags::DiskTags(const DiskTags &other):
    QVector<TrackTags>(other),
    mUri(other.mUri),
    mTitle(other.mTitle)
{

}

/************************************************
 *
 ************************************************/
DiskTags& DiskTags::operator=(const DiskTags &other)
{
    QVector<TrackTags>::operator =(other);
    mUri   = other.mUri;
    mTitle = other.mTitle;

    return *this;
}


/************************************************
 *
 ************************************************/
QString DiskTags::title() const
{
    assert(!isEmpty());

    return mTitle.asString(first().codec());
}


/************************************************
 *
 ************************************************/
void DiskTags::setTitle(const QByteArray &value)
{
    mTitle.setValue(value);
}


/************************************************
 *
 ************************************************/
void DiskTags::setTitle(const QString &value)
{
    mTitle.setValue(value);
}
