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
#include "settings.h"
#include "project.h"
#include "inputaudiofile.h"
#include "outformat.h"
#include "internet/dataprovider.h"

#include <QTextCodec>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QDebug>


/************************************************

 ************************************************/
Track::Track(Disk *disk, int index):
    QObject(disk),
    mDisk(disk),
    mIndex(index),
    mCueIndexes(100),
    mStatus(NotRunning),
    mProgress(0)
{
    qRegisterMetaType<Track::Status>("Track::Status");
}


/************************************************

 ************************************************/
Track::~Track()
{
}


/************************************************
 *
 ************************************************/
uint Track::duration() const
{
    uint start = cueIndex(1).milliseconds();
    uint end = 0;
    if (index() < mDisk->count() - 1)
    {
        end = mDisk->track(index()+1)->cueIndex(1).milliseconds();
    }
    else if (mDisk->audioFile())
    {
        end = mDisk->audioFile()->duration();
    }

    if (start > end)
        return 0;

    return end - start;
}


/************************************************

 ************************************************/
QString Track::tag(const QString &tagName) const
{
    return mDisk->getTag(mIndex, tagName);
}


/************************************************

 ************************************************/
void Track::setTag(const QString &tagName, const QString &value)
{
    mDisk->setTag(mIndex, tagName, value);
}


/************************************************

 ************************************************/
void Track::setProgress(Track::Status status, int percent)
{
    bool status2status [] = {
        // NoRun   Cancel Error  Abort   OK   Process |_____________
            0,     1,     1,     1,     1,     1,    // # NoRun    |
            1,     0,     0,     0,     0,     0,    // # Cancel   |
            1,     0,     0,     0,     0,     0,    // # Error    | Current
            1,     0,     1,     0,     0,     0,    // # Aborted  | statuses
            1,     0,     1,     0,     0,     0,    // # OK       |
            1,     0,     1,     1,     1,     1,    // # Process  |

    };
    int len = 6;
    int processStatus = 5;
    // Process is some is Splitting, Encoding, Queued  etc.

    if (status2status[qMin(processStatus, int(mStatus)) * len +
                      qMin(processStatus, int(status))])
    {
        mStatus = status;
        mProgress = percent;
        project->emitTrackProgress(this);
    }
}


/************************************************

 ************************************************/
QString Track::resultFileName() const
{
    QString pattern = settings->value(Settings::OutFiles_Pattern).toString();
    if (pattern.isEmpty())
        pattern = QString("%a/%y - %A/%n - %t");

    return calcFileName(pattern,
                        disk()->count(),
                        trackNum(),
                        this->album(),
                        this->title(),
                        this->artist(),
                        this->genre(),
                        this->date(),
                        OutFormat::currentFormat()->ext());
}


/************************************************
  %N  Number of tracks       %n  Track number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title
 ************************************************/
QString Track::calcFileName(const QString &pattern,
                            int trackCount,
                            int trackNum,
                            const QString &album,
                            const QString &title,
                            const QString &artist,
                            const QString &genre,
                            const QString &date,
                            const QString &fileExt)
{
    QHash<QChar, QString> tokens;
    tokens.insert(QChar('N'),   QString("%1").arg(trackCount, 2, 10, QChar('0')));
    tokens.insert(QChar('n'),   QString("%1").arg(trackNum, 2, 10, QChar('0')));
    tokens.insert(QChar('A'),   Disk::safeString(album));
    tokens.insert(QChar('t'),   Disk::safeString(title));
    tokens.insert(QChar('a'),   Disk::safeString(artist));
    tokens.insert(QChar('g'),   Disk::safeString(genre));
    tokens.insert(QChar('y'),   Disk::safeString(date));

    QString res = expandPattern(pattern, &tokens, false);
    return res + "." + fileExt;
}


/************************************************

 ************************************************/
QString Track::expandPattern(const QString &pattern, const QHash<QChar, QString> *tokens, bool optional)
{
    QString res;
    bool perc = false;
    bool hasVars = false;
    bool isValid = true;


    for(int i=0; i<pattern.length(); ++i)
    {
        QChar c = pattern.at(i);


        // Sub pattern .................................
        if (c == '{')
        {
            int level = 0;
            int start = i + 1;
            //int j = i;
            QString s = "{";

            for (int j=i; j<pattern.length(); ++j)
            {
                c = pattern.at(j);
                if (c == '{')
                    level++;
                else if (c == '}')
                    level--;

                if (level == 0)
                {
                    s = expandPattern(pattern.mid(start, j - start), tokens, true);
                    i = j;
                    break;
                }
            }
            res += s;
        }
        // Sub pattern .................................

        else
        {
            if (perc)
            {
                perc = false;
                if (tokens->contains(c))
                {
                    QString s = tokens->value(c);
                    hasVars = true;
                    isValid = !s.isEmpty();
                    res += s;
                }
                else
                {
                    if (c == '%')
                        res += "%";
                    else
                        res += QString("%") + c;
                }
            }
            else
            {
                if (c == '%')
                    perc = true;
                else
                    res += c;
            }
        }
    }

    if (perc)
        res += "%";

    if (optional)
    {
        if  (hasVars)
        {
            if (!isValid)
                return "";
        }
        else
        {
            return "{" + res + "}";
        }
    }

    return res;
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

 ************************************************/
QString Track::calcResultFilePath() const
{
    QString settingsDir = settings->value(Settings::OutFiles_Directory).toString();

    if (settingsDir == ".")
        settingsDir = "";

    if (settingsDir.startsWith("~/"))
        return settingsDir.replace(0, 1, QDir::homePath());

    QFileInfo fi(settingsDir);

    if (fi.isAbsolute())
        return fi.absoluteFilePath();

    if (!disk()->audioFileName().isEmpty())
        return QFileInfo(disk()->audioFileName()).dir().absolutePath() + QDir::separator() + settingsDir;

    return QFileInfo(QDir::homePath() + QDir::separator() + settingsDir).absoluteFilePath();
}


/************************************************

 ************************************************/
int Track::trackNum() const
{
    return mDisk->startTrackNum() + mIndex;
}


/************************************************

 ************************************************/
CueIndex Track::cueIndex(int indexNum) const
{
    return mCueIndexes.at(indexNum);
}


/************************************************

 ************************************************/
void Track::setCueIndex(int indexNum, const CueIndex &value)
{
    mCueIndexes[indexNum] = value;
}



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
        patterns << QFileInfo(cueTags.diskTag("FILE")).completeBaseName();
        patterns << QString("%1.*").arg(cueFile.completeBaseName());
    }
    else
    {
        patterns << QFileInfo(cueTags.diskTag("FILE")).completeBaseName();
        patterns << cueFile.completeBaseName() + QString("(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueTags.diskNumInCue() + 1);
        patterns << QString(".*" "(disk|disc|side)" "(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueTags.diskNumInCue() + 1);
    }

    QString audioExt;
    foreach (InputAudioFormat format, InputAudioFormat::allFormats())
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format.ext();

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

    if (cueReaders.count() && cueReaders.first().diskCount() == 1)
    {
        loadFromCue(cueReaders.first().disk(0), true);
        return;
    }


    foreach (const CueReader &cue, cueReaders)
    {
        for (int i=0; i<cue.diskCount(); ++i)
        {
            if (!matchedAudioFiles(cue.disk(i), QFileInfoList() << audio).isEmpty())
            {
                loadFromCue(cue.disk(i), i);
                return;
            }
        }
    }
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
    foreach (InputAudioFormat format, InputAudioFormat::allFormats())
        exts << QString("*.%1").arg(format.ext());

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




