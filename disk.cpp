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


#include "disk.h"
#include "track.h"
#include "project.h"
#include "inputaudiofile.h"
#include "formats/format.h"
#include "outformat.h"

#include "assert.h"
#include <QTextCodec>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QQueue>
#include <QtAlgorithms>
#include <QDebug>

#define COVER_PREVIEW_SIZE 500


/************************************************

 ************************************************/
Disk::Disk(QObject *parent) :
    QObject(parent),
    mStartTrackNum(1),
    mCount(0),
    mAudioFile(0)
{

}


/************************************************

 ************************************************/
Disk::~Disk()
{
    delete mAudioFile;
}


/************************************************

 ************************************************/
Track *Disk::track(int index) const
{
    return mTracks.at(index);
}


/************************************************

 ************************************************/
bool Disk::canConvert(QString *description) const
{
    bool res = true;
    QStringList msg;

    if (!mAudioFile || !mAudioFile->isValid())
    {
        msg << tr("Audio file not set.");
        res = false;
    }

    if (count() < 1)
    {
        msg << tr("Cue file not set.");
        res = false;
    }

    if (res)
    {
        uint duration = 0;
        for (int i=0; i<mTracks.count()-1; ++i)
            duration += track(i)->duration();

        if (mAudioFile->duration() <= duration)
        {
            msg << tr("Audio file shorter than expected from cue sheet.");
            res = false;
        }

    }

    if (description)
        *description = msg.join("\n");

    return res;
}



/************************************************

 ************************************************/
bool Disk::canDownloadInfo() const
{
    return !discId().isEmpty();
}


/************************************************

 ************************************************/
void Disk::loadFromCue(const CueDisk &cueDisk)
{
    QString oldDir = QFileInfo(mCueFile).dir().absolutePath();

    // If the tracks contain tags from the Internet and the
    // tags have been changed, we must save the changes.
    syncTagsFromTracks();
    mTagSets.remove(mCueFile);

    mCount = cueDisk.count();
    mCueFile = cueDisk.fileName();

    // Remove all tags if number of tracks differ from loaded CUE.
    for(auto it = mTagSets.begin(); it != mTagSets.end();)
    {
        if (it.value().count() != mCount)
            it = mTagSets.erase(it);
        else
            ++it;
    }


    // Sync count of tracks
    for (int i=mTracks.count(); i<mCount; ++i)
        mTracks.append(new Track());

    while (mTracks.count() > mCount)
        delete mTracks.takeLast();


    for (int t=0; t<mCount; ++t)
        *mTracks[t] = cueDisk.at(t);

    for (int i=0; i<mCount; ++i)
    {
        Track *track = mTracks[i];
        track->setTrackCount(mCount);
        track->setTrackNum(mStartTrackNum + i);
        track->mDuration = this->trackDuration(i);


    }

    mCurrentTagsUri = mCueFile;
    syncTagsFromTracks();
    mTagSets[mCurrentTagsUri].setTitle(cueDisk.title());


    mPreGapTrack = *mTracks.first();
    mPreGapTrack.setTrackNum(0);
    mPreGapTrack.setTitle("(HTOA)");

    if (!mAudioFile)
        findAudioFile(cueDisk);


    QString dir = QFileInfo(mCueFile).dir().absolutePath();
    if (dir != oldDir)
    {
        mCoverImagePreview = QImage();
        mCoverImageFile = searchCoverImage(dir);
    }

    project->emitLayoutChanged();
}


/************************************************

 ************************************************/
QFileInfoList matchedAudioFiles(const CueDisk &cueDisk, const QFileInfoList &audioFiles)
{
    QFileInfoList res;
    QFileInfo cueFile(cueDisk.fileName());

    QStringList patterns;
    if (cueDisk.diskCount() > 1)
    {
        patterns << QRegExp::escape(QFileInfo(cueDisk.first().tag(TagId::File)).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + QString("(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueDisk.diskNum());
        patterns << QString(".*" "(disk|disc|side)" "(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueDisk.diskNum());
    }
    else
    {
        patterns << QRegExp::escape(QFileInfo(cueDisk.first().tag(TagId::File)).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + ".*";
    }

    QString audioExt;
    foreach (const AudioFormat *format, AudioFormat::inputFormats())
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format->ext();


    foreach (const QString &pattern, patterns)
    {
        QRegExp re(QString("%1(%2)+").arg(pattern).arg(audioExt), Qt::CaseInsensitive, QRegExp::RegExp2);      
        foreach (const QFileInfo &audio, audioFiles)
        {
            if (re.exactMatch(audio.fileName()))
                res << audio;
        }
    }

    return res;
}


/************************************************

 ************************************************/
void Disk::findCueFile()
{
    if (!mAudioFile)
        return;

    QFileInfo audio(mAudioFile->fileName());
    QList<Cue> cues;
    QFileInfoList files = audio.dir().entryInfoList(QStringList() << "*.cue", QDir::Files | QDir::Readable);
    foreach(QFileInfo f, files)
    {
        try
        {
            CueReader c;
            cues << c.load(f.absoluteFilePath());
        }
        catch (FlaconError)
        {
        }
    }

    if (cues.count() == 1 && cues.first().count() == 1)
    {
        loadFromCue(cues.first().first());
        return;
    }

    unsigned int bestWeight = 99999;
    CueDisk bestDisk;

    foreach (const Cue &cue, cues)
    {
        foreach (const CueDisk &cueDisk, cue)
        {
            if (!matchedAudioFiles(cueDisk, QFileInfoList() << audio).isEmpty())
            {
                unsigned int weight = levenshteinDistance(QFileInfo(cueDisk.fileName()).baseName(), audio.baseName());
                if (weight < bestWeight)
                {
                    bestWeight = weight;
                    bestDisk   = cueDisk;
                }
            }
        }
    }

    if (!bestDisk.uri().isEmpty())
        loadFromCue(bestDisk);
}


/************************************************
 *
 ************************************************/
Duration Disk::trackDuration(TrackNum trackNum) const
{
    const Track *track = mTracks[trackNum];
    uint start = track->cueIndex(1).milliseconds();
    uint end = 0;

    if (trackNum < mTracks.count() - 1)
    {
        end = mTracks.at(trackNum+1)->cueIndex(1).milliseconds();
    }
    else if (audioFile() != nullptr)
    {
        end   = audioFile()->duration();
    }

    return end > start ? end - start : 0;
}


/************************************************
 *
 ************************************************/
void Disk::syncTagsFromTracks()
{
    DiskTags &tags = mTagSets[mCurrentTagsUri];
    if (tags.isEmpty())
    {
        tags.resize(mTracks.count());
        tags.setUri(mCurrentTagsUri);
    }

    for (int i=0; i<qMin(tags.count(), mTracks.count()); ++i)
    {
        tags[i] = *(mTracks.at(i));
    }
}


/************************************************
 *
 ************************************************/
void Disk::syncTagsToTracks()
{
    DiskTags &tags = mTagSets[mCurrentTagsUri];
    assert(tags.count() == mTracks.count());

    for (int i=0; i<mTracks.count(); ++i)
    {
        mTracks[i]->setTags(tags.at(i));
    }
}


/************************************************
 *
 ************************************************/
int Disk::distance(const DiskTags &other)
{
    if (mTracks.isEmpty() || other.empty())
        return std::numeric_limits<int>::max();

    int res = 0;

    QString str1 = mTracks.first()->performer().toUpper().replace("THE ", "");
    QString str2 = other.first().performer().toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2) * 3;

    str1 = mTracks.first()->album().toUpper().replace("THE ", "");
    str2 = other.first().album().toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2);

    return res;
}



/************************************************

 ************************************************/
QString Disk::audioFileName() const
{
    if (mAudioFile)
        return mAudioFile->fileName();
    else
        return "";
}


/************************************************

 ************************************************/
void Disk::setAudioFile(const InputAudioFile &audio)
{
    if (mAudioFile)
        delete mAudioFile;

    mAudioFile = new InputAudioFile(audio);

    if (mTracks.isEmpty())
        findCueFile();

    if (!mTracks.isEmpty())
        mTracks.last()->mDuration = trackDuration(mTracks.count() - 1);
}


/************************************************

 ************************************************/
void Disk::findAudioFile(const CueDisk &cueDisk)
{
    if (cueDisk.isEmpty())
        return;

    QStringList exts;
    foreach (const AudioFormat *format, AudioFormat::inputFormats())
        exts << QString("*.%1").arg(format->ext());

    QFileInfo cueFile(mCueFile);
    QFileInfoList files = cueFile.dir().entryInfoList(exts, QDir::Files | QDir::Readable);

    if (cueDisk.diskCount() == 1 && files.count() == 1)
    {
        InputAudioFile audio(files.first().filePath());
        if (audio.isValid())
            setAudioFile(audio);
        return;
    }

    QFileInfoList audioFiles = matchedAudioFiles(cueDisk, files);
    foreach (const QFileInfo &file, audioFiles)
    {
        InputAudioFile audio(file.filePath());
        if (audio.isValid())
        {
            setAudioFile(audio);
            return;
        }
    }
}


/************************************************

 ************************************************/
void Disk::setStartTrackNum(int value)
{
    mStartTrackNum = value;
    foreach (auto track, mTracks)
        track->setTrackNum(value++);

    project->emitDiskChanged(this);
}


/************************************************

 ************************************************/
QString Disk::codecName() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->codecName();

    return "";
}


/************************************************

 ************************************************/
void Disk::setCodecName(const QString codecName)
{

    QString codec = codecName;

    if (codecName == CODEC_AUTODETECT)
    {
        UcharDet charDet;
        foreach (auto track, mTracks)
            charDet << *track;

        codec = charDet.textCodecName();
    }


    foreach (auto track, mTracks)
    {
        track->setCodecName(codec);
    }


    project->emitDiskChanged(this);
}


/************************************************

 ************************************************/
QString Disk::safeString(const QString &str)
{
    QString res = str;
    res.replace('|', "-");
    res.replace('/', "-");
    res.replace('\\', "-");
    res.replace(':', "-");
    res.replace('*', "-");
    res.replace('?', "-");
    return res;
}


/************************************************

 ************************************************/
QString Disk::tagSetTitle() const
{
    if (mCurrentTagsUri.isEmpty())
        return "";

    const_cast<Disk*>(this)->syncTagsFromTracks();
    return mTagSets[mCurrentTagsUri].title();
}


/************************************************

 ************************************************/
QString Disk::tagsUri() const
{
    return mCurrentTagsUri;
}


/************************************************
 *
 ************************************************/
QString Disk::discId() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->tag(TagId::DiscId);

    return "";
}


/************************************************
 *
 ************************************************/
QString Disk::fileTag() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->tag(TagId::File);

    return "";
}


/************************************************
 *
 ************************************************/
QList<DiskTags> Disk::tagSets() const
{
    QList<DiskTags> res;
    const_cast<Disk*>(this)->syncTagsFromTracks();

    res << mTagSets[mCueFile];

    QStringList keys = mTagSets.keys();
    keys.removeAll(mCueFile);
    qSort(keys);

    foreach (const QString &key, keys)
    {
        res << mTagSets[key];
    }

    return res;
}


/************************************************
 *
 ************************************************/
void Disk::addTagSet(const DiskTags &tags, bool activate)
{
    mTagSets[tags.uri()] = tags;

    if (activate)
        activateTagSet(tags);
}


/************************************************
 *
 ************************************************/
void Disk::addTagSets(const QVector<DiskTags> &disks)
{
    if (disks.isEmpty())
        return;

    int minDist = std::numeric_limits<int>::max();
    int bestDisk = 0;
    for (int i=0; i<disks.count(); ++i)
    {
        const DiskTags &disk = disks.at(i);
        addTagSet(disk, false);
        int n = distance(disk);
        if (n<minDist)
        {
            minDist  = n;
            bestDisk = i;
        }
    }

    activateTagSet(disks.at(bestDisk));
}


/************************************************
 *
 ************************************************/
void Disk::activateTagSet(const DiskTags &tags)
{
    if (!mTagSets.contains(tags.uri()))
        return;

    syncTagsFromTracks();
    mCurrentTagsUri = tags.uri();
    syncTagsToTracks();
    project->emitLayoutChanged();
}


/************************************************

 ************************************************/
void Disk::setCoverImageFile(const QString &fileName)
{
    mCoverImageFile = fileName;
    mCoverImagePreview = QImage();
}


/************************************************

 ************************************************/
QImage Disk::coverImagePreview() const
{
    if (!mCoverImageFile.isEmpty() && mCoverImagePreview.isNull())
    {
        mCoverImagePreview = coverImage();
        if (!mCoverImagePreview.isNull())
        {
            mCoverImagePreview.scaled(COVER_PREVIEW_SIZE, COVER_PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    return mCoverImagePreview;
}


/************************************************

 ************************************************/
QImage Disk::coverImage() const
{
    if (mCoverImageFile.isEmpty())
        return QImage();

    return QImage(mCoverImageFile);
}


/************************************************

 ************************************************/
bool compareCoverImages(const QFileInfo &f1, const QFileInfo &f2)
{
    static QStringList order(QStringList()
            << "COVER"
            << "FRONT"
            << "FOLDER");

    int n1 = order.indexOf(f1.baseName().toUpper());
    if (n1 < 0)
        n1 = 9999;

    int n2 = order.indexOf(f2.baseName().toUpper());
    if (n2 < 0)
        n2 = 9999;

    if (n1 != n2)
        return n1 < n2;

    // If we have 2 files with same name but in different directories,
    // we choose the nearest (with the shorter path).
    int l1 = f1.absoluteFilePath().length();
    int l2 = f2.absoluteFilePath().length();
    if (l1 != l2)
        return l1<l2;

    return f1.absoluteFilePath() < f2.absoluteFilePath();
}


/************************************************

 ************************************************/
QStringList Disk::searchCoverImages(const QString &startDir)
{
    QFileInfoList files;

    QStringList exts;
    exts << "*.jpg";
    exts << "*.jpeg";
    exts << "*.png";
    exts << "*.bmp";
    exts << "*.tiff";


    QQueue<QString> query;
    query << startDir;

    QSet<QString> processed;
    while (!query.isEmpty())
    {
        QDir dir(query.dequeue());

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        foreach(QFileInfo d, dirs)
        {
            if (d.isSymLink())
                d = QFileInfo(d.symLinkTarget());

            if (!processed.contains(d.absoluteFilePath()))
            {
                processed << d.absoluteFilePath();
                query << d.absoluteFilePath();
            }
        }

        files << dir.entryInfoList(exts, QDir::Files | QDir::Readable);
    }

    qStableSort(files.begin(), files.end(), compareCoverImages);

    QStringList res;
    foreach (QFileInfo f, files)
        res << f.absoluteFilePath();

    return res;
}


/************************************************

 ************************************************/
QString Disk::searchCoverImage(const QString &startDir)
{
    QStringList l = searchCoverImages(startDir);
    foreach (QString file, l)
    {
        QImage img(file);
        if (!img.isNull())
            return file;
    }
    return "";
}
