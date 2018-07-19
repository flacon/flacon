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
    TagValue value = mTags.value(static_cast<int>(tagID)).value;
    if (value.encoded)
        return encCodec()->toUnicode(value.value);

    assert(mTextCodec != nullptr);
    if (mTextCodec)
        return mTextCodec->toUnicode(value.value);

    return encCodec()->toUnicode(value.value);
}


/************************************************
 *
 ************************************************/
QByteArray TrackTags::tagData(const TagId &tagID) const
{
    return mTags.value(static_cast<int>(tagID)).value;
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
