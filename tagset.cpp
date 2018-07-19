/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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


#include "tagset.h"
#include "settings.h"
#include <QTextCodec>
#include <QSharedData>
#include <QDebug>

#include <uchardet.h>

#define ENC_CODEC "UTF-8"

class Tag
{
public:
    Tag(const QByteArray &val="", bool enc = false):
        value(val),
        encoded(enc)
    {
    }

    QByteArray value;
    bool encoded;
};


class TagSetData: public QSharedData
{
public:
    TagSetData();
    Tag mFileTag;
    int mTrackCount;
    mutable QTextCodec *mTextCodec;
    QString mTextCodecName;
    QString mUri;
    QHash<QString, Tag> mTags;

    inline QString key(int track, const QString &tagName) const;
    QString decode(const Tag &tag) const;
};


/************************************************

 ************************************************/
QString TagSetData::key(int track, const QString &tagName) const
{
    return QString("%1:%2").arg(track).arg(tagName.toUpper());
}


/************************************************

 ************************************************/
QString TagSetData::decode(const Tag &tag) const
{
    if (tag.encoded)
        return QTextCodec::codecForName(ENC_CODEC)->toUnicode(tag.value);

    if (!mTextCodec)
    {
        // Auto detect codepage ...........................
        uchardet_t uc = uchardet_new();

        for(int i=0; i<mTrackCount; ++i)
        {
            const QByteArray &performer = mTags.value(key(i, TAG_PERFORMER)).value;
            const QByteArray &title = mTags.value(key(i, TAG_TITLE)).value;

            uchardet_handle_data(uc, performer.data(), performer.length());
            uchardet_handle_data(uc, title.data(),     title.length());
        }

        uchardet_data_end(uc);
        mTextCodec = QTextCodec::codecForName(uchardet_get_charset(uc));
        if (!mTextCodec)
            mTextCodec = QTextCodec::codecForName("UTF-8");

        uchardet_delete(uc);
    }

    return mTextCodec->toUnicode(tag.value);
}


/************************************************

 ************************************************/
TagSetData::TagSetData():
    mTrackCount(0),
    mTextCodec(0)
{
}


/************************************************

 ************************************************/
TagSet::TagSet(const QString &uri):
    d(new TagSetData)
{
    d->mUri = uri;
    setTextCodecName(settings->value(Settings::Tags_DefaultCodepage).toString());
}


/************************************************

 ************************************************/
TagSet::TagSet(const TagSet &other):
    d(other.d)
{
}

TagSet::TagSet(const QString &uri, const Tracks &tracks):
    d(new TagSetData)
{
    d->mUri = uri;
    d->mTrackCount = tracks.count();
    //d->mFileTag
    //setTextCodecName(mutable QTextCodec *mTextCodec;
    //QString mTextCodecName;

    //d->mTags;

}


/************************************************

 ************************************************/
TagSet::~TagSet()
{
}


/************************************************

 ************************************************/
TagSet &TagSet::operator =(const TagSet &other)
{
    d = other.d;
    return *this;
}


/************************************************

 ************************************************/
QString TagSet::uri() const
{
    return d->mUri;
}


/************************************************

 ************************************************/
QString TagSet::title() const
{
    return diskTag("FLACON_TAGSET_TITLE");
}


/************************************************

 ************************************************/
void TagSet::setTitle(const QString &title)
{
    setTitle(QTextCodec::codecForName(ENC_CODEC)->fromUnicode(title), true);
}


/************************************************

 ************************************************/
void TagSet::setTitle(const QByteArray &title, bool encoded)
{
    setDiskTag("FLACON_TAGSET_TITLE", title, encoded);
}


/************************************************

 ************************************************/
int TagSet::tracksCount() const
{
    return d->mTrackCount;
}


/************************************************

 ************************************************/
QString TagSet::textCodecName() const
{
    return d->mTextCodecName;
}


/************************************************

 ************************************************/
void TagSet::setTextCodecName(const QString codecName)
{
    d->mTextCodecName = codecName;

    if (codecName == CODEC_AUTODETECT)
        d->mTextCodec = 0;
    else
        d->mTextCodec = QTextCodec::codecForName(codecName.toLatin1());
}


/************************************************

 ************************************************/
QString TagSet::trackTag(int track, const QString &tagName) const
{
    QString key = d->key(track, tagName);

    if (d->mTags.contains(key))
        return d->decode(d->mTags.value(key));
    else
        return diskTag(tagName);
}


/************************************************

 ************************************************/
void TagSet::setTrackTag(int track, const QString &tagName, const QString &value)
{
    const QByteArray &data = QTextCodec::codecForName(ENC_CODEC)->fromUnicode(value);
    setTrackTag(track, tagName, data, true);
}


/************************************************

 ************************************************/
void TagSet::setTrackTag(int track, const QString &tagName, const QByteArray &value, bool encoded)
{
    d->mTags.insert(d->key(track, tagName), Tag(value, encoded));
    d->mTrackCount = qMax(track + 1, d->mTrackCount);

    if (d->mTags.contains(tagName))
    {
        if (d->mTags.value(tagName).value != value)
            d->mTags.insert(tagName, Tag("", true));
    }
    else
    {
        d->mTags.insert(tagName, Tag(value, encoded));
    }
}


/************************************************

 ************************************************/
QString TagSet::diskTag(const QString &tagName) const
{
    Tag t = d->mTags.value(tagName);
    return d->decode(t);
}



/************************************************

 ************************************************/
void TagSet::setDiskTag(const QString &tagName, const QString &value)
{
    const QByteArray &data = QTextCodec::codecForName(ENC_CODEC)->fromUnicode(value);
    setDiskTag(tagName, data, true);
}


/************************************************

 ************************************************/
void TagSet::setDiskTag(const QString &tagName, const QByteArray &value, bool encoded)
{
    d->mTags.insert(tagName, Tag(value, encoded));
}


/************************************************

 ************************************************/
int TagSet::distance(const TagSet &other)
{
    return distance(&other);
}


/************************************************

 ************************************************/
int TagSet::distance(const TagSet *other)
{
    int res = 0;
    QString str1 = this->diskTag(TAG_PERFORMER).toUpper().replace("THE ", "");
    QString str2 = other->diskTag(TAG_PERFORMER).toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2) * 3;

    str1 = this->diskTag(TAG_ALBUM).toUpper().replace("THE ", "");
    str2 = other->diskTag(TAG_ALBUM).toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2);

    return res;
}


/************************************************

 ************************************************/
TagSetAction::TagSetAction(QObject *parent, Disk *disk, TagSet *tagSet):
    QAction(parent),
    mDisk(disk),
    mTagSet(tagSet)
{
}


/************************************************

 ************************************************/
TagSetAction::TagSetAction(const QString &text, QObject *parent, Disk *disk, TagSet *tagSet):
    QAction(text, parent),
    mDisk(disk),
    mTagSet(tagSet)
{
}


/************************************************

 ************************************************/
TagSetAction::TagSetAction(const QIcon &icon, const QString &text, QObject *parent, Disk *disk, TagSet *tagSet):
    QAction(icon, text, parent),
    mDisk(disk),
    mTagSet(tagSet)
{
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const TagSet &ts)
{

    dbg.nospace() << "Tagset " << ts.uri() << " [" << ts.title() << "]" <<"\n";
    dbg.nospace() << "  File   " << ts.diskTag(TAG_FILE) << "\n";
    dbg.nospace() << "  DiscId " << ts.diskTag(TAG_DISCID) << "\n";
    dbg.nospace() << "  Genre  " << ts.diskTag(TAG_GENRE) << "\n";
    dbg.nospace() << "  Artist " << ts.diskTag(TAG_PERFORMER) << "\n";
    dbg.nospace() << "  Album  " << ts.diskTag(TAG_ALBUM) << "\n";
    dbg.nospace() << "  Date   " << ts.diskTag(TAG_DATE) << "\n";
    for (int i=0; i < ts.tracksCount(); ++i)
    {
        dbg.nospace() << "  Track " << i << "-=-=-=-=-" << "\n";
        dbg.nospace() << "    * Album  " << ts.trackTag(i, TAG_ALBUM) << "\n";
        dbg.nospace() << "    * Artist " << ts.trackTag(i, TAG_PERFORMER) << "\n";
        dbg.nospace() << "    * Title  " << ts.trackTag(i, TAG_TITLE) << "\n";
        dbg.nospace() << "    * Genre  " << ts.trackTag(i, TAG_GENRE) << "\n";
    }

    return dbg.space();
}

