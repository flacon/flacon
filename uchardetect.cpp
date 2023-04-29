#include "uchardetect.h"
#include "settings.h"
#include <uchardet.h>
#include "track.h"
#include <QTextCodec>

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
        res = QTextCodec::codecForName(Settings::i()->defaultCodepage().toLocal8Bit());

    if (!res || res->name() == "US-ASCII")
        res = QTextCodec::codecForName("UTF-8");

    return res;
}
