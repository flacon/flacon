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
    Q_GADGET
public:
    enum class TagId {
        Album,
        Catalog,
        CdTextfile,
        DiscId,
        AlbumPerformer,
    };
    Q_ENUM(TagId)
    static QList<TagId> allTagId();

    QString tag(TagId tagId) const { return mTags.value(tagId); }
    void    setTag(TagId tagId, const QString &value) { mTags[tagId] = value; }

public:
    DiscNum  discCount() const { return mDiscCount; }
    DiscNum  discNum() const { return mDiscNum; }
    TrackNum trackCount() const { return mTrackCount; }

    QString album() const { return tag(TagId::Album); }
    QString catalog() const { return tag(TagId::Catalog); }
    QString cdTextfile() const { return tag(TagId::CdTextfile); }
    QString discId() const { return tag(TagId::DiscId); }
    QString albumPerformer() const { return tag(TagId::AlbumPerformer); }

    void setDiscCount(DiscNum value) { mDiscCount = value; }
    void setDiscNum(DiscNum value) { mDiscNum = value; }
    void setTrackCount(TrackCount value) { mTrackCount = value; }
    void setAlbum(const QString &value) { setTag(TagId::Album, value); }
    void setCatalog(const QString &value) { setTag(TagId::Catalog, value); }
    void setCdTextfile(const QString &value) { setTag(TagId::CdTextfile, value); }
    void setDiscId(const QString &value) { setTag(TagId::DiscId, value); }
    void setmAlbumPerformer(const QString &value) { setTag(TagId::AlbumPerformer, value); }

    void merge(const AlbumTags &other);

private:
    DiscNum              mDiscCount  = 0;
    DiscNum              mDiscNum    = 0;
    TrackNum             mTrackCount = 0;
    QMap<TagId, QString> mTags;
};

/**************************************
 * TrackTags
 **************************************/
class TrackTags
{
    Q_GADGET
public:
    enum class TagId {
        Comment,
        Date,
        Genre,
        Flags,
        Isrc,
        Title,
        Performer,
        SongWriter,
    };
    Q_ENUM(TagId)
    static QList<TagId> allTagId();

    using Getter = QString (TrackTags::*)() const;
    using Setter = void (TrackTags::*)(const QString &);

public:
    int  trackNum() const { return mTrackNum; }
    void setTrackNum(int value) { mTrackNum = value; }

    QString tag(TagId tagId) const { return mTags.value(tagId); }
    void    setTag(TagId tagId, const QString &value) { mTags[tagId] = value; }

    QString artist() const { return performer(); }
    QString comment() const { return tag(TagId::Comment); }
    QString date() const { return tag(TagId::Date); }
    QString genre() const { return tag(TagId::Genre); }
    QString flagsTag() const { return tag(TagId::Flags); }
    QString isrc() const { return tag(TagId::Isrc); }
    QString title() const { return tag(TagId::Title); }
    QString performer() const { return tag(TagId::Performer); }
    QString songWriter() const { return tag(TagId::SongWriter); }

    void setArtist(const QString &value) { setPerformer(value); }
    void setComment(const QString &value) { setTag(TagId::Comment, value); }
    void setDate(const QString &value) { setTag(TagId::Date, value); }
    void setGenre(const QString &value) { setTag(TagId::Genre, value); }
    void setFlagsTag(const QString &value) { setTag(TagId::Flags, value); }
    void setIsrc(const QString &value) { setTag(TagId::Isrc, value); }
    void setTitle(const QString &value) { setTag(TagId::Title, value); }
    void setPerformer(const QString &value) { setTag(TagId::Performer, value); }
    void setSongWriter(const QString &value) { setTag(TagId::SongWriter, value); }

    void merge(const TrackTags &other);

private:
    int                  mTrackNum = 0;
    QMap<TagId, QString> mTags;
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
