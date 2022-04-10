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

#include "inputaudiofile.h"
#include "project.h"
#include "settings.h"
#include "formats_out/outformat.h"
#include "patternexpander.h"

#include <uchardet.h>
#include <QDir>
#include <QTextCodec>
#include <QDebug>

/************************************************

 ************************************************/
Track::Track()
{
}

/************************************************

************************************************/
Track::Track(Disc *disc, int index) :
    mDisc(disc),
    mIndex(index)
{
}

/************************************************
 *
 ************************************************/
Track::Track(Disc *disc, int index, const TrackTags &tags) :
    mDisc(disc),
    mIndex(index),
    mTags(tags)
{
}

/************************************************

 ************************************************/
Track::~Track()
{
}

void Track::setAudioFile(const InputAudioFile &file)
{
    mAudiofile = file;
}

/************************************************
 *
 ************************************************/
void Track::setTags(const TrackTags &tags)
{
    mTags = tags;
}

/************************************************
 *
 ************************************************/
QString Track::tag(const TagId &tagId) const
{
    return mTags.tag(tagId);
}

/************************************************
 *
 ************************************************/
QByteArray Track::tagData(const TagId &tagId) const
{
    return mTags.tagData(tagId);
}

/************************************************
 *
 ************************************************/
TagValue Track::tagValue(TagId tagId) const
{
    return mTags.tagValue(tagId);
}

/************************************************
 *
 ************************************************/
void Track::setTag(const TagId &tagId, const QString &value)
{
    mTags.setTag(tagId, value);
    if (mDisc) {
        mDisc->trackChanged(tagId);
    }
}

/************************************************
 *
 ************************************************/
void Track::setTag(const TagId &tagId, const QByteArray &value)
{
    mTags.setTag(tagId, value);
    if (mDisc) {
        mDisc->trackChanged(tagId);
    }
}

/************************************************
 *
 ************************************************/
void Track::setTag(TagId tagId, const TagValue &value)
{
    mTags.setTag(tagId, value);
    if (mDisc) {
        mDisc->trackChanged(tagId);
    }
}

/************************************************
 *
 ************************************************/
QString Track::codecName() const
{
    return mTags.codecName();
}

/************************************************
 *
 ************************************************/
void Track::setCodecName(const QString &value)
{
    mTags.setCodecName(value);
}

/************************************************

 ************************************************/
QString Track::resultFileName() const
{
    QString pattern = Settings::i()->currentProfile().outFilePattern();
    if (pattern.isEmpty())
        pattern = QString("%a/%y - %A/%n - %t");

    int n = pattern.lastIndexOf(QDir::separator());
    if (n < 0) {
        PatternExpander expander(*this);
        return safeFilePathLen(expander.expand(pattern) + "." + Settings::i()->currentProfile().ext());
    }

    // If the disc is a collection, the files fall into different directories.
    // So we use the tag DiscPerformer for expand the directory path.
    PatternExpander albumExpander(*this);
    albumExpander.setArtist(this->tag(TagId::AlbumArtist));

    PatternExpander trackExpander(*this);

    return safeFilePathLen(
            albumExpander.expand(pattern.left(n)) + trackExpander.expand(pattern.mid(n)) + "." + Settings::i()->currentProfile().ext());
}

/************************************************
 *
 ************************************************/
bool Track::operator==(const Track &other) const
{
    // clang-format off
    return
        mDisc           == other.mDisc &&
        mCueFileName    == other.mCueFileName &&
        mAudiofile      == other.mAudiofile &&
        mTags           == other.mTags;
    // clang-format on
}

/************************************************
 *
 ************************************************/
TrackNum Track::trackNum() const
{
    bool ok;
    int  res = tag(TagId::TrackNum).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setTrackNum(TrackNum value)
{
    setTag(TagId::TrackNum, QString::number(value));
}

/************************************************
 *
 ************************************************/
TrackNum Track::trackCount() const
{
    bool ok;
    int  res = tag(TagId::TrackCount).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setTrackCount(TrackNum value)
{
    setTag(TagId::TrackCount, QString::number(value));
}

/************************************************
 *
 ************************************************/
DiscNum Track::discNum() const
{
    bool ok;
    int  res = tag(TagId::DiscNum).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setDiscNum(DiscNum value)
{
    setTag(TagId::DiscNum, QString::number(value));
}

/************************************************
 *
 ************************************************/
DiscNum Track::discCount() const
{
    bool ok;
    int  res = tag(TagId::DiscCount).toInt(&ok);

    if (ok)
        return res;

    return 1;
}

/************************************************
 *
 ************************************************/
void Track::setDiscCount(DiscNum value)
{
    setTag(TagId::DiscCount, QString::number(value));
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
 *
 ************************************************/
Duration Track::duration() const
{
    return mDisc ? mDisc->trackDuration(*this) : 0;
}

/************************************************

 ************************************************/
QString Track::calcResultFilePath() const
{
    QString dir = Settings::i()->currentProfile().outFileDir();

    if (dir == "~" || dir == "~//")
        return QDir::homePath();

    if (dir == ".")
        dir = "";

    if (dir.startsWith("~/"))
        return dir.replace(0, 1, QDir::homePath());

    QFileInfo fi(dir);

    if (fi.isAbsolute())
        return fi.absoluteFilePath();

    QString cueFile = mCueFileName;
    if (cueFile.startsWith(Cue::EMBEDED_PREFIX)) {
        cueFile = cueFile.mid(strlen(Cue::EMBEDED_PREFIX));
    }

    return QFileInfo(cueFile).dir().absolutePath() + QDir::separator() + dir;
}

/************************************************
 *
 ************************************************/
QString Track::safeFilePathLen(const QString &path) const
{
    QString file = path;
    QString ext  = QFileInfo(path).suffix();
    if (!ext.isEmpty()) {
        ext = "." + ext;
        file.resize(file.length() - ext.length());
    }

    QStringList res;
    for (QString f : file.split(QDir::separator())) {
        while (f.toUtf8().length() > 250) {
            f.resize(f.length() - 1);
        }
        res << f;
    }
    return res.join(QDir::separator()) + ext;
}

/************************************************

 ************************************************/
CueIndex Track::cueIndex(int indexNum) const
{
    return mTags.cueIndex(indexNum);
}

/************************************************
 *
 ************************************************/
Tracks::Tracks() :
    QVector<Track>()
{
}

/************************************************
 *
 ************************************************/
Tracks::Tracks(int size) :
    QVector<Track>(size)
{
}

/************************************************
 *
 ************************************************/
Tracks::Tracks(const Tracks &other) :
    QVector<Track>(other),
    mUri(other.mUri),
    mTitle(other.mTitle)
{
}

/************************************************
 *
 ************************************************/
Tracks::Tracks(const QList<Track *> &other)
{
    for (const Track *t : other) {
        *this << *t;
    }
}

/************************************************
 *
 ************************************************/
Tracks &Tracks::operator=(const Tracks &other)
{
    QVector<Track>::operator=(other);
    mUri                    = other.mUri;
    mTitle                  = other.mTitle;

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
        res = QTextCodec::codecForName(Settings::i()->value(Settings::Tags_DefaultCodepage).toString().toLocal8Bit());

    if (!res || res->name() == "US-ASCII")
        res = QTextCodec::codecForName("UTF-8");

    return res;
}

/************************************************
 *
 ************************************************/
QTextCodec *determineTextCodec(const QVector<Track *> &tracks)
{
    QTextCodec *res;
    uchardet_t  uc = uchardet_new();

    foreach (const Track *track, tracks) {
        const QByteArray &performer = track->tagData(TagId::Artist);
        const QByteArray &title     = track->tagData(TagId::Title);

        uchardet_handle_data(uc, performer.data(), performer.length());
        uchardet_handle_data(uc, title.data(), title.length());
    }

    uchardet_data_end(uc);
    res = QTextCodec::codecForName(uchardet_get_charset(uc));
    if (!res)
        res = QTextCodec::codecForName("UTF-8");

    uchardet_delete(uc);

    return res;
}

QDebug operator<<(QDebug debug, const Track &track)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << " Track {"
                    << " trackNum: " << track.trackNum()
                    << " trackCount:" << track.trackCount()
                    << " diskId:" << track.discId()
                    << " Artist:" << track.artist()
                    << " Album:" << track.album()
                    << " Title:" << track.title()
                    << " cueFile:" << track.cueFileName()
                    << "}";
    return debug;
}
