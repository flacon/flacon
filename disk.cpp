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
CueIndex::CueIndex(const QString &str):
    mNull(true),
    mCdValue(0),
    mHiValue(0)
{
    if (!str.isEmpty())
        mNull = !parse(str);
}


/************************************************

 ************************************************/
QString CueIndex::toString(bool cdQuality) const
{
    if (cdQuality)
    {
        int min =  mCdValue / (60 * 75);
        int sec = (mCdValue - min * 60 * 75) / 75;
        int frm =  mCdValue - (min * 60 + sec) * 75;

        return QString("%1:%2:%3")
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(frm, 2, 10, QChar('0'));
    }
    else
    {
        int min = mHiValue / (60 * 1000);
        int sec = (mHiValue - min * 60 * 1000) / 1000;
        int msec = mHiValue - (min * 60 + sec) * 1000;

        return QString("%1:%2.%3")
                .arg(min,  2, 10, QChar('0'))
                .arg(sec,  2, 10, QChar('0'))
                .arg(msec, 3, 10, QChar('0'));
    }

}


/************************************************

 ************************************************/
CueIndex CueIndex::operator -(const CueIndex &other) const
{
    CueIndex res;
    res.mCdValue = this->mCdValue - other.mCdValue;
    res.mHiValue = this->mHiValue - other.mHiValue;
    res.mNull = false;
    return res;
}


/************************************************

 ************************************************/
bool CueIndex::operator ==(const CueIndex &other) const
{
    return this->mHiValue == other.mHiValue;
}


/************************************************

 ************************************************/
bool CueIndex::operator !=(const CueIndex &other) const
{
    return this->mHiValue != other.mHiValue;
}


/************************************************

 ************************************************/
bool CueIndex::parse(const QString &str)
{
    QStringList sl = str.split(QRegExp("\\D"), QString::KeepEmptyParts);

    if (sl.length()<3)
        return false;

    bool ok;
    int min = sl[0].toInt(&ok);
    if (!ok)
        return false;

    int sec = sl[1].toInt(&ok);
    if (!ok)
        return false;

    int frm = sl[2].leftJustified(2, '0').toInt(&ok);
    if (!ok)
        return false;

    int msec = sl[2].leftJustified(3, '0').toInt(&ok);
    if (!ok)
        return false;

    mCdValue = (min * 60 + sec) * 75 + frm;
    mHiValue = (min * 60 + sec) * 1000 + msec;
    return true;
}



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

    return calcResultFilePath() + QDir::separator() + fileName;
}


/************************************************

 ************************************************/
QString Track::calcResultFilePath() const
{
    QString settingsDir = settings->value(Settings::OutFiles_Directory).toString();

    if (settingsDir.isEmpty())
        settingsDir = ".";

    if (settingsDir.startsWith("~/"))
        return settingsDir.replace(0, 1, QDir::homePath());

    QFileInfo fi(settingsDir);

    if (fi.isAbsolute())
        return fi.absoluteFilePath();
    if (!disk()->audioFileName().isEmpty())
    {
        return QFileInfo(disk()->audioFileName()).dir().absolutePath() + QDir::separator() + settingsDir;
    }
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
    mCueTags(0),
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
        msg << tr("CUE file not set.");
        res = false;
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
void Disk::loadFromCue(const QString &cueFile, bool activate)
{
    mErrorString.clear();
    mCount = 0;

    QFileInfo fi(cueFile);
    TagSet *tags = new TagSet(fi.canonicalFilePath());
    tags->setTitle(fi.fileName());

    QFile file(fi.canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        mErrorString = file.errorString();
    }

    int seek = 0;
    QByteArray magic = file.read(3);

    if (magic.startsWith("\xEF\xBB\xBF"))
    {
        tags->setTextCodecName("UTF-8");
        seek = 3;
    }

    else if (magic.startsWith("\xFE\xFF"))
    {
        tags->setTextCodecName("UTF-16BE");
        seek = 2;
    }

    else if (magic.startsWith("\xFF\xFE"))
    {
        tags->setTextCodecName("UTF-16LE");
        seek = 2;
    }

    file.seek(seek);
    mValid = parseCue(file, tags);
    file.close();

    if (!mValid)
    {
        delete tags;
        return;
    }

    if (mCueTags)
    {
        mTagSets.removeAll(mCueTags);
        delete mCueTags;
    }

    mCueFile = cueFile;
    mCueTags = tags;
    mTagSets.append(tags);
    if (!mTags || activate)
    {
        mTags = mCueTags;
        project->emitLayoutChanged();
    }

}

/************************************************

 ************************************************/
QByteArray leftPart(const QByteArray &line, const QChar separator)
{
    int n = line.indexOf(separator);
    if (n > -1)
        return line.left(n);
    else
        return line;
}


/************************************************

 ************************************************/
QByteArray rightPart(const QByteArray &line, const QChar separator)
{
    int n = line.indexOf(separator);
    if (n > -1)
        return line.right(line.length() - n - 1);
    else
        return QByteArray();
}


/************************************************
 Complete cue sheet syntax documentation
 http://digitalx.org/cue-sheet/syntax/
 ************************************************/
bool Disk::parseCue(QFile &file, TagSet *tags)
{
    Track *track = 0;
    int trackIdx = -1;
    QByteArray performer;
    QByteArray album;
    QByteArray genre;
    QByteArray date;
    QByteArray comment;
    QByteArray songwriter;

    while (!file.atEnd())
    {

        QByteArray line = file.readLine().trimmed();

        if (line.isEmpty())
            continue;

        QByteArray tag = leftPart(line, ' ').toUpper();
        QByteArray value = rightPart(line, ' ').trimmed();


        if (tag == "REM")
        {
            tag = leftPart(value, ' ').toUpper();
            value = rightPart(value, ' ').trimmed();
        }

        if (value.length() > 2 && (value.at(0) == '"' || value.at(0) == '\''))
            value = value.mid(1, value.length() - 2);


        //=============================
        if (tag == "TRACK")
        {
            bool ok;
            leftPart(value, ' ').toInt(&ok);
            if (!ok)
            {
                mErrorString = tr("File <b>%1</b> is not a valid CUE file.").arg(file.fileName());
                return false;
            }

            trackIdx++;

            if (trackIdx < mTracks.count())
            {
                track = mTracks.at(trackIdx);
            }
            else
            {
                track = new Track(this, trackIdx);
                mTracks.append(track);
            }

            tags->setTrackTag(trackIdx, TAG_PERFORMER,  performer,  false);
            tags->setTrackTag(trackIdx, TAG_ALBUM,      album,      false);
            tags->setTrackTag(trackIdx, TAG_GENRE,      genre,      false);
            tags->setTrackTag(trackIdx, TAG_DATE,       date,       false);
            tags->setTrackTag(trackIdx, TAG_COMMENT,    comment,    false);
            tags->setTrackTag(trackIdx, TAG_SONGWRITER, songwriter, false);
        }

        //=============================
        else if (tag == "INDEX")
        {
            if (track)
            {
                bool ok;
                int num = leftPart(value, ' ').toInt(&ok);
                if (!ok)
                {
                    mErrorString = tr("File <b>%1</b> is not a valid CUE file.").arg(file.fileName());
                    return false;
                }

                QString time = rightPart(value, ' ');
                track->setCueIndex(num, CueIndex(time));
            }
        }

        //=============================
        else if (tag == "DISCID")
        {
            tags->setDiskTag(TAG_DISCID, value, true);
        }

        //=============================
        else if (tag == "TITLE")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_TITLE, value, false);
            else
                album = value;
        }

        //=============================
        else if (tag == "CATALOG")
        {
            tags->setDiskTag(TAG_CATALOG, value, false);
        }

        //=============================
        else if (tag == "CDTEXTFILE")
        {
            tags->setDiskTag(TAG_CDTEXTFILE, value, false);
        }

        //=============================
        else if (tag == "COMMENT")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_COMMENT, value, false);
            else
                comment = value;
        }

        //=============================
        else if (tag == "DATE")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_DATE, value, false);
            else
                date = value;
        }

        //=============================
        else if (tag == "FLAGS")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_FLAGS, value, true);
        }

        //=============================
        else if (tag == "GENRE")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_GENRE, value, false);
            else
                genre = value;
        }

        //=============================
        else if (tag == "ISRC")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_ISRC, value, true);
        }

        //=============================
        else if (tag == "PERFORMER")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_PERFORMER, value, false);
            else
                performer = value;
        }

        //=============================
        else if (tag == "SONGWRITER")
        {
            if (trackIdx > -1)
                tags->setTrackTag(trackIdx, TAG_SONGWRITER, value, false);
            else
                songwriter = value;
        }

        //=============================
        else if (tag == "FILE")
        {
            if (trackIdx > -1)
            {
                mErrorString = tr("File <b>%1</b> contains several FILE tags.<br>These CUE files are not supported yet.").arg(tags->uri());
                return false;
            }
            else
                tags->setDiskTag(TAG_FILE,  value, false);
        }
    }

    mCount = trackIdx + 1;
    return true;
}


/************************************************

 ************************************************/
void Disk::findCueFile()
{
    if (!mAudioFile)
        return;

    QStringList exts;
    exts << "*.cue";

    QFileInfo fi(mAudioFile->fileName());
    QString pattern = fi.completeBaseName();

    QFileInfoList files = fi.dir().entryInfoList(exts, QDir::Files | QDir::Readable);

    foreach(QFileInfo f, files)
    {
        if (f.fileName().startsWith(pattern))
            loadFromCue(f.absoluteFilePath());
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
void Disk::setAudioFile(const QString &fileName)
{
    QFileInfo fi(fileName);
    InputAudioFile *audio = new InputAudioFile(fi.absoluteFilePath());

    if (audio->isValid())
    {
        if (mAudioFile)
            delete mAudioFile;

        mAudioFile = audio;
        project->emitDiskChanged(this);
    }
    else
    {
        delete audio;
    }
}


/************************************************

 ************************************************/
void Disk::findAudioFile()
{
    QStringList exts;
    foreach (InputAudioFormat format, InputAudioFormat::allFormats())
        exts << QString("*.%1").arg(format.ext());

    QFileInfo fi(mCueFile);

    QSet<QString> patterns;
    patterns << fi.completeBaseName();

    QString ft = fileTag();
    if (! ft.isEmpty())
        patterns << QFileInfo(ft).completeBaseName();


    QFileInfoList files = fi.dir().entryInfoList(exts, QDir::Files | QDir::Readable);
    foreach(QFileInfo f, files)
    {
        QSetIterator<QString> i(patterns);
        while (i.hasNext())
        {
            QString pattern = i.next();

            if (f.fileName().startsWith(pattern))
            {
                InputAudioFile *audio = new InputAudioFile(f.absoluteFilePath());
                if (audio->isValid())
                {
                    if (mAudioFile)
                        delete mAudioFile;

                    mAudioFile = audio;
                    return;
                }
                else
                {
                    delete audio;
                }
            }
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
            mTagSets.removeAll(t);
            if (mTags == t)
                mTags = 0;
            delete t;
        }
    }

    TagSet *newTagSet = new TagSet(tagSet);
    mTagSets << newTagSet;

    if (mTags == 0 || activate)
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




