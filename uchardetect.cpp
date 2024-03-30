/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2023
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

#include "uchardetect.h"
#include "settings.h"
#include <uchardet.h>
#include "track.h"

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
UcharDet::UcharDet() :
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
UcharDet &UcharDet::operator<<(const Track &track)
{
    TagId tags[] = { TagId::Artist, TagId::Title };

    for (uint i = 0; i < sizeof(tags) / sizeof(TagId); ++i) {
        TagValue tv = track.tagValue(tags[i]);
        if (!tv.encoded())
            uchardet_handle_data(mData->mUchcharDet, tv.value().data(), tv.value().length());
    }

    return *this;
}
/************************************************
 *
 ************************************************/
UcharDet &UcharDet::operator<<(const TrackTags &track)
{
    TagId tags[] = { TagId::Artist, TagId::Title };

    for (uint i = 0; i < sizeof(tags) / sizeof(TagId); ++i) {
        TagValue tv = track.tagValue(tags[i]);
        if (!tv.encoded())
            uchardet_handle_data(mData->mUchcharDet, tv.value().data(), tv.value().length());
    }

    return *this;
}

/************************************************
 *
 ************************************************/
QString UcharDet::textCodecName() const
{
    uchardet_data_end(mData->mUchcharDet);
    QString res = uchardet_get_charset(mData->mUchcharDet);

    if (!TextCodec::codecForName(res).isValid()) {
        res = Settings::i()->defaultCodepage();
    }

    if (res == "US-ASCII") {
        res = "UTF-8";
    }

    return res;
}
