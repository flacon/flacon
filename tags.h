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

class Disc;
class Track;

/**************************************
 * TagsId
 **************************************/
struct TagsId
{
    QString uri;
    QString title;
};

/**************************************
 * AlbumTags
 **************************************/
class AlbumTags
{
public:
    DiscNum  discCount() const { return mDiscCount; }
    DiscNum  discNum() const { return mDiscNum; }
    TrackNum trackCount() const { return mTrackCount; }

    QString album() const { return mAlbum; }
    QString artist() const { return performer(); }
    QString catalog() const { return mCatalog; }
    QString cdTextfile() const { return mCdTextfile; }
    QString comment() const { return mComment; }
    QString date() const { return mDate; }
    QString discId() const { return mDiscId; }
    QString genre() const { return mGenre; }
    QString performer() const { return mPerformer; }
    QString songWriter() const { return mSongWriter; }

    void setDiscCount(DiscNum value) { mDiscCount = value; }
    void setDiscNum(DiscNum value) { mDiscNum = value; }
    void setTrackCount(TrackCount value) { mTrackCount = value; }
    void setAlbum(const QString &value) { mAlbum = value; }
    void setArtist(const QString &value) { setPerformer(value); }
    void setCatalog(const QString &value) { mCatalog = value; }
    void setCdTextfile(const QString &value) { mCdTextfile = value; }
    void setComment(const QString &value) { mComment = value; }
    void setDate(const QString &value) { mDate = value; }
    void setDiscId(const QString &value) { mDiscId = value; }
    void setGenre(const QString &value) { mGenre = value; }
    void setPerformer(const QString &value) { mPerformer = value; }
    void setSongWriter(const QString &value) { mSongWriter = value; }

    void merge(const AlbumTags &other);

private:
    DiscNum  mDiscCount  = 1;
    DiscNum  mDiscNum    = 1;
    TrackNum mTrackCount = 0;

    QString mAlbum;
    QString mCatalog;
    QString mCdTextfile;
    QString mComment;
    QString mDate;
    QString mDiscId;
    QString mGenre;
    QString mPerformer;
    QString mSongWriter;
};

/**************************************
 * TrackTags
 **************************************/
class TrackTags
{
public:
    using Getter = QString (TrackTags::*)() const;
    using Setter = void (TrackTags::*)(const QString &);

public:
    int  trackNum() const { return mTrackNum; }
    void setTrackNum(int value) { mTrackNum = value; }

    QString artist() const { return performer(); }
    QString comment() const { return mComment; }
    QString date() const { return mDate; }
    QString flagsTag() const { return mFlagsTag; }
    QString isrc() const { return mIsrc; }
    QString title() const { return mTitle; }
    QString performer() const { return mPerformer; }
    QString songWriter() const { return mSongWriter; }

    void setArtist(const QString &value) { setPerformer(value); }
    void setComment(const QString &value) { mComment = value; }
    void setDate(const QString &value) { mDate = value; }
    void setFlagsTag(const QString &value) { mFlagsTag = value; }
    void setIsrc(const QString &value) { mIsrc = value; }
    void setTitle(const QString &value) { mTitle = value; }
    void setPerformer(const QString &value) { mPerformer = value; }
    void setSongWriter(const QString &value) { mSongWriter = value; }

    void merge(const TrackTags &other);

private:
    int mTrackNum = 0;

    QString mComment;
    QString mFlagsTag;
    QString mDate;
    QString mIsrc;
    QString mPerformer;
    QString mSongWriter;
    QString mTitle;
};

/**************************************
 * Tags
 **************************************/
class Tags : public AlbumTags
{
public:
    using Track = TrackTags;

    const QVector<TrackTags> &tracks() const { return mTracks; }
    QVector<TrackTags>       &tracks() { return mTracks; }

    bool isEmpty() const { return mTracks.isEmpty(); }
    void resize(int size);

    void merge(const Tags &other);

private:
    QVector<TrackTags> mTracks;
};

/**************************************
 * InternetTags
 **************************************/
class InternetTags : public Tags
{
public:
    TagsId tagsId() const { return mTagsId; }
    void   setTagsId(const TagsId &value) { mTagsId = value; }

private:
    TagsId mTagsId;
};

QDebug operator<<(QDebug debug, const Tags::Track &track);

#endif // TAGS_H
