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

public:
    explicit Disc(QObject *parent = nullptr);
    virtual ~Disc();

    QList<Track *> tracks() const { return mTracks; }
    Track         *track(int index) const;
    int            count() const { return mTracks.count(); }
    const Track   *preGapTrack() const;

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

    QString codecName() const;
    void    setCodecName(const QString &codecName);

    QString tagSetTitle() const;
    QString tagsUri() const;
    QString discId() const;
    DiscNum discNum() const;
    DiscNum discCount() const;

    struct TagSet
    {
        QString uri;
        QString name;
    };

    QVector<TagSet> tagSets() const;
    void            addTagSet(const Tracks &tags, bool activate);
    void            addTagSets(const QVector<Tracks> &discs);
    void            activateTagSet(const QString &uri);

    void searchCoverImage(bool replaceExisting = false);

    QString coverImageFile() const { return mCoverImageFile; }
    void    setCoverImageFile(const QString &fileName);
    QImage  coverImagePreview() const;
    QImage  coverImage() const;

    QString    discTag(TagId tagId) const;
    QByteArray discTagData(TagId tagId) const;
    void       setDiscTag(TagId tagId, const QString &value);
    void       setDiscTag(TagId tagId, const QByteArray &value);

    static QStringList searchCoverImages(const QString &startDir);
    static QString     searchCoverImage(const QString &startDir);

    bool isEmpty() const { return mTracks.isEmpty(); }

    DiskState state() const { return mState; }
    void      setState(DiskState value);

signals:
    void tagChanged();
    void revalidateRequested();

protected:
    void trackChanged(TagId tagId);

    Duration trackDuration(const Track &track) const;

private:
    QHash<QString, Tracks> mTagSets;

    QList<Track *> mTracks;
    Cue            mCue;
    QString        mCurrentTagsUri;

    InputAudioFile mAudioFile;
    mutable Track  mPreGapTrack;

    QString        mCoverImageFile;
    mutable QImage mCoverImagePreview;

    DiskState mState = DiskState::NotRunning;

    void syncTagsFromTracks();
    void syncTagsToTracks();

    int  distance(const Tracks &other);
    bool isSameTagValue(TagId tagId);
};

typedef QList<Disc *> DiscList;

using Disk     = Disc;
using DiskList = DiscList;

#endif // DISC_H
