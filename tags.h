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
#include "cue.h"

struct TagSet
{
    QString uri;
    QString title;
};

class Tags
{
public:
    QString albumTag(TagId tagId) const;
    void    setAlbumTag(TagId tagId, const QString &value);

    QString trackTag(int trackIndex, TagId tagId) const;
    void    setTrackTag(int trackIndex, TagId tagId, const QString &value);

    bool containsTrackTag(int trackIndex, TagId tagId) const;

    bool isEmpty() const { return mTrackTags.isEmpty(); }
    int  trackCount() const { return mTrackTags.count(); }
    void resize(int size);

    bool compareTags(const Tags &other) const;

private:
    QMap<TagId, QString>          mAlbumTags;
    QVector<QMap<TagId, QString>> mTrackTags;
};

// class InternetTags : public Tags
// {
// public:
//     QString uri() const { return mUri; }
//     QString name() const { return mName; }

//     void setUri(const QString &value) { mUri = value; }
//     void setName(const QString &value) { mName = value; }

// private:
//     QString mUri;
//     QString mName;
// };

class RawTags
{
public:
    QByteArray trackTag(int trackIndex, TagId tagId) const;
    void       setTrackTag(int trackIndex, TagId tagId, const QByteArray &value);

    bool containsTrackTag(int trackIndex, TagId tagId) const;

private:
    QVector<QMap<TagId, QByteArray>> mTrackTags;
};

/**************************************
 * TrackTags
 **************************************/
class TrackTags
{
    friend class Track;
    friend class Disc;

public:
    QString title() const { return mTitle; }

    void setTitle(const QString &value)
    {
        mTitle        = value;
        mTitleChanged = true;
    }

protected:
    void initFromCue(const Cue::Track &cueTrack, const TextCodec &textCodec);

private:
    QString mTitle;
    bool    mTitleChanged;
};

/**************************************
 * InternetTags
 **************************************/
class InternetTags
{
public:
    class Track
    {
    public:
        QString title() const { return mTitle; }
        void    setTitle(const QString &value) { mTitle = value; }

        int  trackNum() const { return mTrackNum; }
        void setTrackNum(int value) { mTrackNum = value; }

        bool compareTags(const Track &other) const;

    private:
        QString mTitle;
        int     mTrackNum = 0;
    };

public:
    QString uri() const { return mUri; }
    QString name() const { return mName; }

    void setUri(const QString &value) { mUri = value; }
    void setName(const QString &value) { mName = value; }

    const QVector<Track> &tracks() const { return mTracks; }
    QVector<Track>       &tracks() { return mTracks; }

    bool isEmpty() const { return mTracks.isEmpty(); }

    QString date() const { return mDate; }
    void    setDate(const QString &value) { mDate = value; }

    QString album() const { return mAlbum; }
    void    setAlbum(const QString &value) { mAlbum = value; }

    QString artist() const { return mArtist; }
    void    setArtist(const QString &value) { mArtist = value; }

    QString genre() const { return mGenre; }
    void    setGenre(const QString &value) { mGenre = value; }

    // QString songWriter() const { return mSongWriter; }
    // void    setSongWriter(const QString &value) { mSongWriter = value; }

    bool compareTags(const InternetTags &other) const;

private:
    QString        mUri;
    QString        mName;
    QVector<Track> mTracks;

    QString mDate;
    QString mAlbum;
    QString mArtist;
    QString mGenre;
    //  QString mSongWriter;
};

// /**************************************
//  * DiskTags
//  **************************************/
// class DiskTags
// {
// public:
//     class Track
//     {
//     public:
//         QString title() const { return mTitle; }
//         void    setTitle(const QString &value);

//         int  trackNum() const { return mTrackNum; }
//         void setTrackNum(int value);

//         void initFromInternetTags(const InternetTags::Track &tags);

//     private:
//         QString mTitle;
//         int     mTrackNum = 0;

//         bool mTitleChanged    = false;
//         bool mTrackNumChanged = false;
//     };

// public:
//     const QVector<Track> &tracks() const { return mTracks; }
//     QVector<Track>       &tracks() { return mTracks; }

//     QString date() const { return mDate; }
//     void    setDate(const QString &value);

//     QString album() const { return mAlbum; }
//     void    setAlbum(const QString &value);

//     QString artist() const { return mArtist; }
//     void    setArtist(const QString &value);

//     QString genre() const { return mGenre; }
//     void    setGenre(const QString &value);

//     void initFromInternetTags(const InternetTags &tags);

// private:
//     QVector<Track> mTracks;

//     QString mDate;
//     QString mAlbum;
//     QString mArtist;
//     QString mGenre;

//     bool mDateChanged   = false;
//     bool mAlbumChanged  = false;
//     bool mArtistChanged = false;
//     bool mGenreChanged  = false;
// };

#endif // TAGS_H
