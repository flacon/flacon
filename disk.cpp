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
#include "internet/dataprovider.h"

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
    mTags(0),
    mStartTrackNum(1),
    mCount(0),
    mValid(false),
    mAudioFile(0)
{
    mPreGapTrack = new PreGapTrack(this);
}


/************************************************

 ************************************************/
Disk::~Disk()
{
    delete mAudioFile;
    qDeleteAll(mTagSets);
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
void Disk::downloadInfo()
{
    if (discId().isEmpty())
        return;

    DataProvider *provider = new FreeDbProvider(this);
    connect(provider, SIGNAL(finished()), this, SLOT(downloadFinished()));
    mDownloads << provider;
    provider->start();
    project->emitDiskChanged(this);
}


/************************************************

 ************************************************/
bool Disk::canDownloadInfo()
{
    return !discId().isEmpty();
}


/************************************************

 ************************************************/
bool Disk::isDownloads() const
{
    return mDownloads.count() > 0;
}


/************************************************

 ************************************************/
void Disk::downloadFinished()
{
    DataProvider *provider = qobject_cast<DataProvider*>(sender());
    mDownloads.removeAll(provider);
    provider->deleteLater();
}


/************************************************

 ************************************************/
void Disk::loadFromCue(const CueTagSet &cueTags, bool activate)
{
    mCount = cueTags.tracksCount();
    mCueFile = cueTags.cueFileName();

    for (int i=mTracks.count(); i<mCount; ++i)
        mTracks.append(new Track(this, i));

    while (mTracks.count() > mCount)
        mTracks.takeLast()->deleteLater();

    for (int t=0; t<cueTags.tracksCount(); ++t)
    {
        for (int idx=0; idx<100; ++idx)
            mTracks[t]->setCueIndex(idx, cueTags.index(t, idx));
    }

    if (mTagSets.isEmpty())
        mTagSets << 0;
    else
        delete mTagSets.first();

    mTagSets[0] = new TagSet(cueTags);
    setStartTrackNum(cueTags.diskTag(START_TRACK_NUM).toInt());

    if (!mAudioFile)
        findAudioFile(cueTags);

    if (activate || !mTags)
    {
        mTags = mTagSets.first();
        project->emitLayoutChanged();
    }

    mCoverImagePreview = QImage();
    mCoverImageFile = searchCoverImage(QFileInfo(mCueFile).dir().absolutePath());
}


/************************************************

 ************************************************/
QFileInfoList matchedAudioFiles(const CueTagSet &cueTags, const QFileInfoList &audioFiles)
{
    QFileInfoList res;
    QFileInfo cueFile(cueTags.cueFileName());

    QStringList patterns;
    if (!cueTags.isMultiFileCue())
    {
        patterns << QRegExp::escape(QFileInfo(cueTags.diskTag("FILE")).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + ".*";
    }
    else
    {
        patterns << QRegExp::escape(QFileInfo(cueTags.diskTag("FILE")).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + QString("(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueTags.diskNumInCue() + 1);
        patterns << QString(".*" "(disk|disc|side)" "(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueTags.diskNumInCue() + 1);
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
    QList<CueReader> cueReaders;
    QFileInfoList files = audio.dir().entryInfoList(QStringList() << "*.cue", QDir::Files | QDir::Readable);
    foreach(QFileInfo f, files)
    {
        CueReader c(f.absoluteFilePath());
        if (c.isValid())
            cueReaders << c;
    }

    if (cueReaders.count() == 1 && cueReaders.first().diskCount() == 1)
    {
        loadFromCue(cueReaders.first().disk(0), true);
        return;
    }

    unsigned int bestWeight = 99999;
    CueTagSet bestDisk("");

    foreach (const CueReader &cue, cueReaders)
    {
        for (int i=0; i<cue.diskCount(); ++i)
        {
            if (!matchedAudioFiles(cue.disk(i), QFileInfoList() << audio).isEmpty())
            {
                unsigned int weight = levenshteinDistance(QFileInfo(cue.fileName()).baseName(), audio.baseName());
                if (weight < bestWeight)
                {
                    bestWeight = weight;
                    bestDisk = cue.disk(i);
                }
            }
        }
    }

    if (!bestDisk.uri().isEmpty())
        loadFromCue(bestDisk);
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

    if (mTagSets.isEmpty())
        findCueFile();
}


/************************************************

 ************************************************/
void Disk::findAudioFile(const CueTagSet &cueTags)
{
    if (mTagSets.isEmpty())
        return;

    QStringList exts;
    foreach (const AudioFormat *format, AudioFormat::inputFormats())
        exts << QString("*.%1").arg(format->ext());

    QFileInfo cueFile(mCueFile);
    QFileInfoList files = cueFile.dir().entryInfoList(exts, QDir::Files | QDir::Readable);

    if (!cueTags.isMultiFileCue() && files.count() == 1)
    {
        InputAudioFile audio(files.first().filePath());
        if (audio.isValid())
            setAudioFile(audio);
        return;
    }

    QFileInfoList audioFiles = matchedAudioFiles(cueTags, files);
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
    project->emitDiskChanged(this);
}


/************************************************

 ************************************************/
QString Disk::textCodecName() const
{
    if (mTags)
        return mTags->textCodecName();
    else
        return "";
}


/************************************************

 ************************************************/
void Disk::setTextCodecName(const QString codecName)
{
    if (mTags)
        mTags->setTextCodecName(codecName);

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
QString Disk::tagsTitle() const
{
    if (mTags)
        return mTags->title();
    else
        return "";
}


/************************************************

 ************************************************/
QString Disk::tagsUri() const
{
    if (mTags)
        return mTags->uri();
    else
        return "";
}


/************************************************

 ************************************************/
QString Disk::tag(const QString tagName) const
{
    if (mTags)
        return mTags->diskTag(tagName);
    else
        return "";
}


/************************************************

 ************************************************/
void Disk::addTagSet(const TagSet &tagSet, bool activate)
{
    // Remove old doublicates
    foreach (TagSet *t, mTagSets)
    {
        if (t->uri() == tagSet.uri())
        {
            if (mTags == t)
                mTags = 0;

            mTagSets.removeAll(t);
            delete t;
        }
    }

    TagSet *newTagSet = new TagSet(tagSet);
    mTagSets << newTagSet;

    if (activate || mTags == 0)
    {
        mTags = newTagSet;
        project->emitLayoutChanged();
    }
}


/************************************************

 ************************************************/
void Disk::activateTagSet(const TagSet *tagSet)
{
    foreach (TagSet *t, mTagSets)
    {
        if (t->uri() == tagSet->uri())
        {
            mTags = t;
            project->emitLayoutChanged();
        }
    }
}


/************************************************

 ************************************************/
int Disk::distance(const TagSet &other)
{
    return distance(&other);
}


/************************************************

 ************************************************/
int Disk::distance(const TagSet *other)
{
    if (mTags)
        return mTags->distance(other);
    else
        return 999999;
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
    if (l.isEmpty())
        return "";
    else
        return l.first();
}


/************************************************

 ************************************************/
QString Disk::getTag(int track, const QString &tagName)
{
    if (mTags)
        return mTags->trackTag(track, tagName);
    else
        return "";
}


/************************************************

 ************************************************/
void Disk::setTag(int track, const QString &tagName, const QString &value)
{
    mTags->setTrackTag(track, tagName, value);
    emit trackChanged(track);
    int disk = project->indexOf(this);
    project->emitTrackChanged(disk, track);
}


/************************************************

 ************************************************/
DiskAction::DiskAction(QObject *parent, Disk *disk, Track *track, const QString tagName):
    QAction(parent),
    mDisk(disk),
    mTrack(track),
    mTagName(tagName)
{
}


/************************************************

 ************************************************/
DiskAction::DiskAction(const QString &text, QObject *parent, Disk *disk, Track *track, const QString tagName):
    QAction(text, parent),
    mDisk(disk),
    mTrack(track),
    mTagName(tagName)
{
}


/************************************************

 ************************************************/
DiskAction::DiskAction(const QIcon &icon, const QString &text, QObject *parent, Disk *disk, Track *track, const QString tagName):
    QAction(icon, text, parent),
    mDisk(disk),
    mTrack(track),
    mTagName(tagName)
{
}



/************************************************

 ************************************************/
PreGapTrack::PreGapTrack(Disk *disk):
    Track(disk, -1)
{
}


/************************************************

 ************************************************/
QString PreGapTrack::tag(const QString &tagName) const
{
    if (tagName == TAG_TITLE)
        return "(HTOA)";

    if (disk()->count())
        return disk()->track(0)->tag(tagName);

    return "";
}


/************************************************

 ************************************************/
void PreGapTrack::setTag(const QString &tagName, const QString &value)
{
}




