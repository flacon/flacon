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


#ifndef DISK_H
#define DISK_H

#include "track.h"
#include "cue.h"

#include <QObject>
#include <QList>
#include <QVector>
#include <QAction>
#include <QHash>
#include <QChar>
#include <QImage>

class CueReader;
class QFile;
class Disk;
class InputAudioFile;
class DataProvider;


class Disk: public QObject
{
    Q_OBJECT
    friend class Track;
public:
    explicit Disk(QObject *parent = 0);
    virtual ~Disk();

    Track *track(int index) const;
    int count() const { return mCount; }
    const Track *preGapTrack() const { return &mPreGapTrack; }

    void loadFromCue(const CueDisk &cueDisk, bool activate = true);
    QString cueFile() const { return mCueFile; }

    InputAudioFile *audioFile() const { return mAudioFile; }
    QString audioFileName() const;
    void setAudioFile(const InputAudioFile &audio);


    int startTrackNum() const { return mStartTrackNum; }
    void setStartTrackNum(int value);

    QString textCodecName() const;
    void setTextCodecName(const QString codecName);

    static QString safeString(const QString &str);

    QString tagsTitle() const;
    QString tagsUri() const;
    QString discId() const;
    QString fileTag() const;

    //QString tag(const QString tagName) const;

    QList<TagSet*> tagSets() const { return mTagSets; }

    bool canConvert(QString *description = 0) const;

    bool canDownloadInfo();
    bool isDownloads() const;

    void addTagSet(const TagSet &tagSet, bool activate);
    void activateTagSet(const TagSet *tagSet);

    int distance(const TagSet &other);
    int distance(const TagSet *other);

    QString coverImageFile() const { return mCoverImageFile; }
    void setCoverImageFile(const QString &fileName);
    QImage coverImagePreview() const;
    QImage coverImage() const;

    static QStringList searchCoverImages(const QString &startDir);
    static QString searchCoverImage(const QString &startDir);


public slots:
    void downloadInfo();

signals:
    void trackChanged(int track);

protected:
    QString getTag(int track, const QString &tagName);
    //void setTag(int track, const QString &tagName, const QString &value);

private slots:
    void downloadFinished();

private:
    QList<TagSet*> mTagSets;
    TagSet *mTags;

    QList<Track*> mTracks;
    int mStartTrackNum;
    int mCount;
    QString mCueFile;
    InputAudioFile *mAudioFile;
    Track mPreGapTrack;
    QList<DataProvider*> mDownloads;

    QString mCoverImageFile;
    mutable QImage  mCoverImagePreview;

    void findAudioFile(const CueDisk &cueDisk);
    void findCueFile();
    Duration trackDuration(TrackNum trackNum) const;
};

typedef QList<Disk*> DiskList;


class DiskAction: public QAction
{
    Q_OBJECT
public:
    DiskAction(QObject* parent, Disk *disk, Track *track = 0, const QString tagName ="");
    DiskAction(const QString &text, QObject* parent, Disk *disk, Track *track = 0, const QString tagName ="");
    DiskAction(const QIcon &icon, const QString &text, QObject* parent, Disk *disk, Track *track = 0, const QString tagName ="");

    Disk *disk() const { return mDisk; }
    Track *track() const { return mTrack; }
    QString tagName() const { return mTagName; }

private:
    Disk *mDisk;
    Track *mTrack;
    QString mTagName;
};


#endif // DISK_H
