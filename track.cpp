/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "track.h"

#include <assert.h>

#include "disk.h"
#include "inputaudiofile.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"

#include <uchardet.h>
#include <QDir>
#include <QTextCodec>
#include <QDebug>


/************************************************

 ************************************************/
Track::Track():
    mTextCodec(nullptr),
    mTrackNum(0),
    mTrackCount(0),
    mDuration(0)
{
}


/************************************************
 *
 ************************************************/
Track::Track(const Track &other):
    mTags(other.mTags),
    mTextCodec(other.mTextCodec),
    mCueIndexes(other.mCueIndexes),
    mTrackNum(other.mTrackNum),
    mTrackCount(other.mTrackCount),
    mDuration(other.mDuration),
    mCueFileName(other.mCueFileName)
{
}


/************************************************
 *
 ************************************************/
Track &Track::operator =(const Track &other)
{
    setTags(other);
    mCueIndexes = other.mCueIndexes;
    mTrackNum   = other.mTrackNum;
    mTrackCount = other.mTrackCount;
    mDuration   = other.mDuration;
    mCueFileName= other.mCueFileName;

    return *this;
}


/************************************************
 *
 ************************************************/
void Track::setTags(const Track &other)
{
    mTags      = other.mTags;
    mTextCodec = other.mTextCodec;
}



/************************************************

 ************************************************/
Track::~Track()
{
}


/************************************************
 *
 ************************************************/
QString Track::tag(const TagId &tagID) const
{
    return mTags.value(static_cast<int>(tagID)).asString(mTextCodec);
}


/************************************************
 *
 ************************************************/
QByteArray Track::tagData(const TagId &tagID) const
{
    return mTags.value(static_cast<int>(tagID)).value();
}


/************************************************
 *
 ************************************************/
void Track::setTag(const TagId &tagID, const QString &value)
{
    mTags.insert(static_cast<int>(tagID), TagValue(value));
}


/************************************************
 *
 ************************************************/
void Track::setTag(const TagId &tagID, const QByteArray &value)
{
    mTags.insert(static_cast<int>(tagID), TagValue(value, false));
}


/************************************************
 *
 ************************************************/
QString Track::codecName() const
{
    if (mTextCodec)
        return mTextCodec->name();

    return "";
}


/************************************************
 *
 ************************************************/
void Track::setCodecName(const QString &value)
{
    if (!value.isEmpty())
        mTextCodec = QTextCodec::codecForName(value.toLatin1());
    else
        mTextCodec = nullptr;
}


/************************************************

 ************************************************/
QString Track::resultFileName() const
{
    QString pattern = settings->outFilePattern();
    if (pattern.isEmpty())
        pattern = QString("%a/%y - %A/%n - %t");

    int n = pattern.lastIndexOf(QDir::separator());
    if (n < 0)
    {
        return calcFileName(pattern,
                            this->trackCount(),
                            this->trackNum(),
                            this->album(),
                            this->title(),
                            this->artist(),
                            this->genre(),
                            this->date(),
                            settings->outFormat()->ext());
    }

    // If the disk is a collection, the files fall into different directories.
    // So we use the tag DiskPerformer for expand the directory path.
    return calcFileName(pattern.left(n),
                        this->trackCount(),
                        this->trackNum(),
                        this->album(),
                        this->title(),
                        this->tag(TagId::DiskPerformer),
                        this->genre(),
                        this->date(),
                        "")

            +

            calcFileName(pattern.mid(n),
                         this->trackCount(),
                         this->trackNum(),
                         this->album(),
                         this->title(),
                         this->artist(),
                         this->genre(),
                         this->date(),
                         settings->outFormat()->ext());





}


/************************************************
  %N  Number of tracks       %n  Track number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title
 ************************************************/
QString Track::calcFileName(const QString &pattern,
                            int trackCount,
                            int trackNum,
                            const QString &album,
                            const QString &title,
                            const QString &artist,
                            const QString &genre,
                            const QString &date,
                            const QString &fileExt)
{
    QHash<QChar, QString> tokens;
    tokens.insert(QChar('N'),   QString("%1").arg(trackCount, 2, 10, QChar('0')));
    tokens.insert(QChar('n'),   QString("%1").arg(trackNum, 2, 10, QChar('0')));
    tokens.insert(QChar('A'),   Disk::safeString(album));
    tokens.insert(QChar('t'),   Disk::safeString(title));
    tokens.insert(QChar('a'),   Disk::safeString(artist));
    tokens.insert(QChar('g'),   Disk::safeString(genre));
    tokens.insert(QChar('y'),   Disk::safeString(date));

    QString res = expandPattern(pattern, &tokens, false);

    if (fileExt.isEmpty())
        return res;

    return res + "." + fileExt;
}


/************************************************
 *
 ************************************************/
bool Track::operator ==(const Track &other) const
{
    if (this->mCueFileName != other.mCueFileName)
        return false;

    return mTags ==(other.mTags);
}


/************************************************

 ************************************************/
QString Track::expandPattern(const QString &pattern, const QHash<QChar, QString> *tokens, bool optional)
{
    QString res;
    bool perc = false;
    bool hasVars = false;
    bool isValid = true;


    for(int i=0; i<pattern.length(); ++i)
    {
        QChar c = pattern.at(i);


        // Sub pattern .................................
        if (c == '{')
        {
            int level = 0;
            int start = i + 1;
            //int j = i;
            QString s = "{";

            for (int j=i; j<pattern.length(); ++j)
            {
                c = pattern.at(j);
                if (c == '{')
                    level++;
                else if (c == '}')
                    level--;

                if (level == 0)
                {
                    s = expandPattern(pattern.mid(start, j - start), tokens, true);
                    i = j;
                    break;
                }
            }
            res += s;
        }
        // Sub pattern .................................

        else
        {
            if (perc)
            {
                perc = false;
                if (tokens->contains(c))
                {
                    QString s = tokens->value(c);
                    hasVars = true;
                    isValid = !s.isEmpty();
                    res += s;
                }
                else
                {
                    if (c == '%')
                        res += "%";
                    else
                        res += QString("%") + c;
                }
            }
            else
            {
                if (c == '%')
                    perc = true;
                else
                    res += c;
            }
        }
    }

    if (perc)
        res += "%";

    if (optional)
    {
        if  (hasVars)
        {
            if (!isValid)
                return "";
        }
        else
        {
            return "{" + res + "}";
        }
    }

    return res;
}



/************************************************

 ************************************************/
QString Track::resultFilePath() const
{
    QString fileName = resultFileName();
    if (fileName.isEmpty())
        return "";

    QString dir = calcResultFilePath();
    if (dir.endsWith("/") || fileName.startsWith("/"))
        return calcResultFilePath() + fileName;
    else
        return calcResultFilePath() + "/" + fileName;
}


/************************************************

 ************************************************/
QString Track::calcResultFilePath() const
{
    QString dir = settings->outFileDir();

    if (dir == "~" || dir == "~//")
        return QDir::homePath();

    if (dir == ".")
        dir = "";

    if (dir.startsWith("~/"))
        return dir.replace(0, 1, QDir::homePath());

    QFileInfo fi(dir);

    if (fi.isAbsolute())
        return fi.absoluteFilePath();

    return QFileInfo(mCueFileName).dir().absolutePath() + QDir::separator() + dir;
}


/************************************************

 ************************************************/
CueIndex Track::cueIndex(int indexNum) const
{
    if (indexNum < mCueIndexes.length())
        return mCueIndexes.at(indexNum);

    return CueIndex();
}


/************************************************

 ************************************************/
void Track::setCueIndex(int indexNum, const CueIndex &value)
{
    if (indexNum >= mCueIndexes.length())
        mCueIndexes.resize(indexNum+1);

    mCueIndexes[indexNum] = value;
}



/************************************************
 *
 ************************************************/
Tracks::Tracks():
    QVector<Track>()
{

}


/************************************************
 *
 ************************************************/
Tracks::Tracks(int size):
    QVector<Track>(size)
{

}


/************************************************
 *
 ************************************************/
Tracks::Tracks(const Tracks &other):
    QVector<Track>(other),
    mUri(other.mUri),
    mTitle(other.mTitle)
{

}

/************************************************
 *
 ************************************************/
Tracks& Tracks::operator=(const Tracks &other)
{
    QVector<Track>::operator =(other);
    mUri   = other.mUri;
    mTitle = other.mTitle;

    return *this;
}


/************************************************
 *
 ************************************************/
Tracks::~Tracks()
{

}


/************************************************
 *
 ************************************************/
QString Tracks::title() const
{
    assert(!isEmpty());
    return mTitle.asString(first().codec());
}


/************************************************
 *
 ************************************************/
void Tracks::setTitle(const QByteArray &value)
{
    mTitle.setValue(value);
}


/************************************************
 *
 ************************************************/
void Tracks::setTitle(const QString &value)
{
    mTitle.setValue(value);
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
UcharDet &UcharDet::operator<<(const Track &track)
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
