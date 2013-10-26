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

#include "tagset.h"

#include <QObject>
#include <QList>
#include <QVector>
#include <QAction>
#include <QHash>
#include <QChar>

class QFile;
class Disk;
class Track;
class InputAudioFile;
class DataProvider;

class CueIndex
{
public:
    CueIndex(const QString &str = "");

    bool isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueIndex operator-(const CueIndex &other) const;
    bool operator==(const CueIndex &other) const;
    bool operator!=(const CueIndex &other) const;

private:
    bool mNull;
    int mValue;

    bool parse(const QString &str);
};



class Disk: public QObject
{
    Q_OBJECT
    friend class Track;
public:
    explicit Disk(QObject *parent = 0);
    virtual ~Disk();

    Track *track(int index) const;
    int count() const { return mCount; }
    Track *preGapTrack() const { return mPreGapTrack; }

    void loadFromCue(const QString &cueFile, bool activate = true);
    QString cueFile() const { return mCueFile; }
    void findCueFile();

    InputAudioFile *audioFile() const { return mAudioFile; }
    QString audioFileName() const;
    void setAudioFile(const QString &fileName);
    void findAudioFile();

    int startTrackNum() const { return mStartTrackNum; }
    void setStartTrackNum(int value);

    QString textCodecName() const;
    void setTextCodecName(const QString codecName);

    static QString safeString(const QString &str);

    QString tagsTitle() const;
    QString tagsUri() const;
    QString discId() const  { return tag(TAG_DISCID); }
    QString fileTag() const { return tag(TAG_FILE); }

    QString tag(const QString tagName) const;


    bool isValid() const { return mValid; }
    QString errorString() const { return mErrorString; }

    QList<TagSet*> tagSets() const { return mTagSets; }

    bool canConvert(QString *description = 0) const;

    bool canDownloadInfo();
    bool isDownloads() const;

    void addTagSet(const TagSet &tagSet, bool activate);
    void activateTagSet(const TagSet *tagSet);

    int distance(const TagSet &other);
    int distance(const TagSet *other);

public slots:
    void downloadInfo();

signals:
    void trackChanged(int track);

protected:
    QString getTag(int track, const QString &tagName);
    void setTag(int track, const QString &tagName, const QString &value);

private slots:
    void downloadFinished();

private:
    QList<TagSet*> mTagSets;
    TagSet *mTags;
    TagSet *mCueTags;
    QList<Track*> mTracks;
    int mStartTrackNum;
    int mCount;
    bool mValid;
    QString mErrorString;
    QString mCueFile;
    InputAudioFile *mAudioFile;
    Track *mPreGapTrack;
    QList<DataProvider*> mDownloads;

    bool parseCue(QFile &file, TagSet *tags);
};


class Track: public QObject
{
    Q_OBJECT
public:

    enum Status
    {
        NotRunning  = 0,
        Canceled    = 1,
        Error       = 2,
        Aborted     = 3,
        OK          = 4,
        Splitting   = 5,
        Encoding    = 6,
        Queued      = 7,
        WaitGain    = 8,
        CalcGain    = 9,
        WriteGain   = 10
    };

    explicit Track(Disk *disk, int index);
    ~Track();

    QString artist() const            { return tag("PERFORMER"); }
    void setArtist(const QString &value)  { setTag("PERFORMER", value); }

    QString album() const             { return tag("ALBUM"); }
    void setAlbum(const QString &value)   { setTag("ALBUM", value); }

    QString comment() const           { return tag("COMMENT") ;}
    void setComment(const QString &value)   { setTag("COMMENT", value); }

    QString title() const             { return tag("TITLE") ;}
    void setTitle(const QString &value)   { setTag("TITLE", value); }

    QString genre() const             { return tag("GENRE") ;}
    void setGenre(const QString &value)   { setTag("GENRE", value); }

    QString date() const              { return tag("DATE") ;}
    void setDate(const QString &value)    { setTag("DATE", value); }


    QString resultFileName() const;
    QString resultFilePath() const;

    Disk *disk() const { return mDisk; }
    int index() const { return mIndex; }

    int trackNum() const;

    CueIndex cueIndex(int indexNum) const;
    void setCueIndex(int indexNum, const CueIndex &value);

    virtual QString tag(const QString &tagName) const;
    virtual void setTag(const QString &tagName, const QString &value);

    int progress() const { return mProgress; }
    Status status() const { return mStatus; }
    void setProgress(Status status, int percent = -1);

private:
    Disk *mDisk;
    int mIndex;
    QVector<CueIndex> mCueIndexes;
    Status mStatus;
    int mProgress;

    QString calcResultFilePath() const;
    QString expandPattern(const QString &pattern, const QHash<QChar,QString> *tokens, bool optional) const;
};

Q_DECLARE_METATYPE(Track::Status)


class PreGapTrack: public Track
{
public:
    explicit PreGapTrack(Disk *disk);

    virtual QString tag(const QString &tagName) const;
    virtual void setTag(const QString &tagName, const QString &value);
};


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
