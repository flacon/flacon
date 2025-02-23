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

#ifndef DISC_H
#define DISC_H

#include "track.h"
#include "cue.h"

#include <QObject>
#include <QImage>

class QFile;

class Disc : public QObject
{
    Q_OBJECT
    friend class Track;

    struct TagSet
    {
        TagsId tagsId;
        Tags   userTags;
        Tags   loadedTags;
    };

public:
    explicit Disc(QObject *parent = nullptr);
    virtual ~Disc();

    QList<Track *> tracks() const { return mTracks; }
    bool           isEmpty() const { return mTracks.isEmpty(); }

    Cue     cue() const { return mCue; }
    void    setCue(const Cue &cue);
    QString cueFilePath() const;

    QList<TrackPtrList> tracksByFileTag() const;

    InputAudioFileList audioFiles() const;
    void               setAudioFiles(const InputAudioFileList &files);
    QStringList        audioFileNames() const;

    /// If some tracks don't have audio, result will have empty item.
    QStringList audioFilePaths() const;
    void        setAudioFile(const InputAudioFile &file, int fileNum);
    bool        isMultiAudio() const;

    int  startTrackNum() const;
    void setStartTrackNum(TrackNum value);

    const TextCodec &textCodec() const { return mTextCodec; }
    QString          codecName() const;
    void             setCodecName(const QString &codecName);

    QList<TagsId> tagSets() const;
    TagsId        currentTagSet() const;
    void          activateTagSet(const QString &uri);

    void addInternetTags(const QVector<InternetTags> &tags);

    void searchCoverImage(bool replaceExisting = false);

    QString coverImageFile() const { return mCoverImageFile; }
    void    setCoverImageFile(const QString &fileName);
    QImage  coverImagePreview() const;
    QImage  coverImage() const;

    static QStringList searchCoverImages(const QString &startDir);
    static QString     searchCoverImage(const QString &startDir);

public:
    // Tags
    QString tag(AlbumTags::TagId tagId) const;
    void    setTag(AlbumTags::TagId tagId, const QString &value);

    DiscNum  discCountTag() const;
    DiscNum  discNumTag() const;
    TrackNum trackCountTag() const;

    QString albumTag() const { return tag(AlbumTags::TagId::Album); }
    QString catalogTag() const { return tag(AlbumTags::TagId::Catalog); }
    QString cdTextfileTag() const { return tag(AlbumTags::TagId::CdTextfile); }
    QString discIdTag() const { return tag(AlbumTags::TagId::DiscId); }
    QString albumPerformerTag() const { return tag(AlbumTags::TagId::AlbumPerformer); }

    void setDiscCountTag(DiscNum value);
    void setDiscNumTag(DiscNum value);

    void setAlbumTag(const QString &value) { setTag(AlbumTags::TagId::Album, value); }
    void setCatalogTag(const QString &value) { setTag(AlbumTags::TagId::Catalog, value); }
    void setCdTextfileTag(const QString &value) { setTag(AlbumTags::TagId::CdTextfile, value); }
    void setDiscIdTag(const QString &value) { setTag(AlbumTags::TagId::DiscId, value); }
    void setAlbumPerformerTag(const QString &value) { setTag(AlbumTags::TagId::AlbumPerformer, value); }

    AlbumTags toTags() const;

public:
    DiskState state() const { return mState; }
    void      setState(DiskState value);

private:
    QList<Track *> mTracks;
    Cue            mCue;

    Tags mCueUserTags;
    Tags mUserTags;
    Tags mLoadedTags;

    TextCodec mTextCodec = TextCodecUtf8();

    QVector<TagSet> mTagSets;

    QList<InternetTags> mInternetTags;
    QList<Tags>         mInternetUserTags;
    int                 mInternetTagsIndex = -1;

    InputAudioFile mAudioFile;

    QString        mCoverImageFile;
    mutable QImage mCoverImagePreview;

    DiskState mState = DiskState::NotRunning;

    int distance(const InternetTags &other);

    void syncTagsFromTracks();
    void syncTagsToTracks();
};

typedef QList<Disc *> DiscList;

using Disk     = Disc;
using DiskList = DiscList;

#endif // DISC_H
