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


#include "splitter.h"
#include "settings.h"
#include "project.h"
#include "disk.h"
#include "inputaudiofile.h"

#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QRegExp>
#include <QTextCodec>
#include <QDebug>

class CueCreator
{
public:
    explicit CueCreator(Disk *disk);
    bool write();

    QString errorString() const { return mErrorString; }

    QTextCodec *textCodec() const { return mTextCodec; }
    void setTextCodecName(const QString codecName);
    void setTextCodecMib(int mib);

    bool hasPreGapFile() const { return mHasPreGapFile; }
    void setHasPregapFile(bool value) { mHasPreGapFile = value; }

private:
    Disk *mDisk;
    QFile mFile;
    bool mHasPreGapFile;
    QString mErrorString;
    QTextCodec *mTextCodec;
    QHash<QString, QString>mGlobalTags;

    void initGlobalTags();
    void writeLine(const QString &text);
    void writeDiskTag(const QString &format, const QString &tagName);
    void writeGlobalTag(const QString &format, const QString &tagName);
    void writeTrackTag(const Track *track, const QString &prefix, const QString &tagName);

};

/************************************************

 ************************************************/
Splitter::Splitter(Disk *disk, QObject *parent):
    ConverterThread(disk, parent),
    mProcess(0),
    mPreGapExists(false)
{    
    QString tmpDir = settings->value(Settings::Encoder_TmpDir).toString();

    if (tmpDir.isEmpty())
        mWorkDir = QFileInfo(disk->track(0)->resultFilePath()).dir().absolutePath();
    else
        mWorkDir = QDir(QString("%1/flacon.%2").arg(tmpDir).arg(QCoreApplication::applicationPid())).absolutePath();

    mFilePrefix = QString("tmp-%1-%2.").arg(QCoreApplication::applicationPid()).arg(project->indexOf(disk));
}


/************************************************

 ************************************************/
bool Splitter::isReadyStart() const
{
    return true;
}


/************************************************

 ************************************************/
void Splitter::inputDataReady(Track *track, const QString &fileName)
{
}


/************************************************

 ************************************************/
void Splitter::doStop()
{
    if (mProcess)
    {
        mProcess->closeReadChannel(QProcess::StandardError);
        mProcess->closeReadChannel(QProcess::StandardOutput);
        mProcess->closeWriteChannel();
        mProcess->terminate();
    }
}


/************************************************
 Split audio file to temporary dir
 ************************************************/
void Splitter::doRun()
{
    QStringList args;
    args << "split";
    args << "-w";
    args << "-O" << "always";
    args << "-n" << "%04d";
    args << "-t" << mFilePrefix +"%n";
    args << "-d" << mWorkDir;
    args << disk()->audioFileName();
    //qDebug() << args;

    QString shntool = settings->value(Settings::Prog_Shntool).toString();
    mProcess = new QProcess();
    mProcess->setReadChannel(QProcess::StandardError);

    mProcess->start(shntool, args);
    mProcess->waitForStarted();

    sendCueData();
    mProcess->closeWriteChannel();

    parseOut();
    mProcess->waitForFinished(-1);

    QProcess *proc = mProcess;
    mProcess = 0;
    delete proc;

    if (OutFormat::currentFormat()->createCue())
    {
        CueCreator cue(disk());
        cue.setHasPregapFile(mPreGapExists);
        if (!cue.write())
            error(disk()->track(0), cue.errorString());
    }
}

/************************************************

 ************************************************/

void Splitter::parseOut()
{
    Track *track = disk()->track(0);

    bool deletePregap = !OutFormat::currentFormat()->createCue();
    char c;
    QByteArray buf;

    while(mProcess->waitForReadyRead(-1))
    {
        while (mProcess->read(&c, 1))
        {
            // .......................................
            if (c == '\n')
            {
                if (buf.contains(": error:"))
                {
                    error(track, QString::fromLocal8Bit(buf.mid(17)));
                    return;
                }
                buf = "";
            }


            // .......................................
            else if (c == '%')
            {
                bool ok;
                int percent = buf.right(3).trimmed().toInt(&ok);
                if (!ok)
                    continue;


                // Splitting [/home/user/inDir/input.wav] (10:00.000) --> [/home/user/outDir/tmp-15196-00000.wav] (0:00.440) : 100% OK

                QString pattern = "[" +  mWorkDir + QDir::separator() + mFilePrefix;
                QString sbuf = QString::fromLocal8Bit(buf);
                int n = sbuf.indexOf(pattern, disk()->audioFileName().length() + 20);

                if (n < 0 && sbuf.length() < n + pattern.length() + 4)
                {
                    qWarning() << "I can't parse" << sbuf;
                    continue;
                }

                QString fileName = sbuf.mid(n + 1, + pattern.length() - 1 + 4 + 4); // -1 for leading "[", 4 for 4 digits tracknum, 4 - file ext ".wav"

                int trackNum = fileName.mid(fileName.length() - 8, 4).toInt(&ok);

                if (!ok)
                {
                    qWarning() << "I can't parse" << sbuf;
                    continue;
                }



                if (trackNum > disk()->count())
                {
                    error(disk()->track(0), tr("The number of tracks is higher than expected."));
                    return;
                }

                track = (trackNum == 0) ? disk()->preGapTrack() : disk()->track(trackNum - 1);
                mPreGapExists = mPreGapExists || (trackNum == 0);

                emit trackProgress(track, Track::Splitting, percent);

                if (percent == 100)
                {
                    if (trackNum == 0 && deletePregap)
                        deleteFile(fileName);
                    else
                        emit trackReady(track, fileName);
                }
            }

            // .......................................
            else
            {
                buf += c;
            }

            //std::cerr << c;
        }
    }
}


/************************************************

 ************************************************/
void Splitter::sendCueData()
{
    bool cdQuality = disk()->audioFile()->isCdQuality();
    OutFormat *format = OutFormat::currentFormat();

    bool fakeIndex = (format->createCue() and
                      format->preGapType() == OutFormat::PreGapAddToFirstTrack);

    QFile cue(disk()->cueFile());
    cue.open(QFile::ReadOnly);

    int trackNum = 0;
    QByteArray line;
    while (!cue.atEnd())
    {
        line = cue.readLine();
        QString str = QString(line).trimmed();
        QString key = str.section(' ', 0, 0).toUpper();

        if (key == "TRACK")
        {
            trackNum++;
            mProcess->write(line);
            continue;
        }

        if (key == "INDEX")
        {
            int indexNum = str.section(' ', 1, 1).toInt();

            if (fakeIndex && trackNum == 1)
            {
                if (indexNum == 1)
                {
                    if (cdQuality)
                        mProcess->write("  INDEX 01 00:00:00\n");
                    else
                        mProcess->write("  INDEX 01 00:00.000\n");
                }
            }
            else
            {
                CueIndex index(str.section(' ', 2));
                mProcess->write(QString("  INDEX %1 %2\n")
                                .arg(indexNum, 2, 10, QChar('0'))
                                .arg(index.toString(cdQuality))
                                .toLocal8Bit());
            }
            continue;
        }

        if (key == "FILE")
        {
            mProcess->write(line);
            continue;
        }

    }

    cue.close();
}


/************************************************

 ************************************************/
CueCreator::CueCreator(Disk *disk):
    mDisk(disk),
    mHasPreGapFile(false)
{
    Track *track = mDisk->track(0);
    QString fileName = QFileInfo(track->resultFilePath()).dir().absolutePath() + QDir::separator() +
                       Disk::safeString(QString("%1-%2.cue").arg(track->artist(), track->album()));

    mFile.setFileName(fileName);
    setTextCodecName("UTF-8");
}


/************************************************

 ************************************************/
void CueCreator::setTextCodecName(const QString codecName)
{
    mTextCodec = QTextCodec::codecForName(codecName.toLatin1());
    if (!mTextCodec)
        mTextCodec = QTextCodec::codecForName("UTF-8");
}


/************************************************

 ************************************************/
void CueCreator::setTextCodecMib(int mib)
{
    mTextCodec = QTextCodec::codecForMib(mib);
}


/************************************************

 ************************************************/
void CueCreator::initGlobalTags()
{
    QStringList tagNames;
    tagNames << TAG_GENRE;
    tagNames << TAG_DATE;
    tagNames << TAG_PERFORMER;
    tagNames << TAG_SONGWRITER;
    tagNames << TAG_ALBUM;

    Track *firstTrack = mDisk->track(0);

    foreach(QString tagName, tagNames)
    {
        QString value = firstTrack->tag(tagName);

        for (int i=1; i<mDisk->count(); ++i)
        {
            if (mDisk->track(i)->tag(tagName) != value)
                value = "";
        }

        mGlobalTags.insert(tagName, value);
    }
}


/************************************************

 ************************************************/
void CueCreator::writeLine(const QString &text)
{
    mFile.write(mTextCodec->fromUnicode(text));
    mFile.write("\n");
}


/************************************************

 ************************************************/
void CueCreator::writeDiskTag(const QString &format, const QString &tagName)
{
    QString value = mDisk->tag(tagName);

    if (!value.isEmpty())
        writeLine(format.arg(value));
}


/************************************************

 ************************************************/
void CueCreator::writeGlobalTag(const QString &format, const QString &tagName)
{
    QString value = mGlobalTags.value(tagName);

    if (!value.isEmpty())
        writeLine(format.arg(value));
}


/************************************************

 ************************************************/
void CueCreator::writeTrackTag(const Track *track, const QString &prefix, const QString &tagName)
{
    QString value = track->tag(tagName);

    if (!value.isEmpty() && value != mGlobalTags.value(tagName))
        writeLine(prefix.arg(value));
}

/************************************************

 ************************************************/
bool CueCreator::write()
{
    if (!mFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        mErrorString = QObject::tr("I can't write CUE file <b>%1</b>:<br>%2").arg(mFile.fileName(), mFile.errorString());
        return false;
    }

    initGlobalTags();

    // Common ...........................
    writeDiskTag("CATALOG %1",          TAG_CATALOG);
    writeDiskTag("CDTEXTFILE \"%1\"",   TAG_CDTEXTFILE);
    writeGlobalTag("REM GENRE \"%1\"",  TAG_GENRE);
    writeGlobalTag("REM DATE %1",       TAG_DATE);
    writeDiskTag("REM DISCID %1",       TAG_DISCID);

    if (settings->value(Settings::PerTrackCue_FlaconTags).toBool())
        writeLine(QString("REM COMMENT \"Flacon v%1\"").arg(FLACON_VERSION));

    writeGlobalTag("PERFORMER \"%1\"",  TAG_PERFORMER);
    writeGlobalTag("SONGWRITER \"%1\"", TAG_SONGWRITER);
    writeGlobalTag("TITLE \"%1\"",      TAG_ALBUM);

    if (mHasPreGapFile)
        writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisk->preGapTrack()->resultFilePath()).fileName()));
    else
        writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisk->track(0)->resultFilePath()).fileName()));

    // Tracks ...........................
    CueIndex prevIndex("00:00:00");
    for(int i=0; i<mDisk->count(); ++i)
    {
        Track *track = mDisk->track(i);
        CueIndex index0 = track->cueIndex(0);
        CueIndex index1 = track->cueIndex(1);

        writeLine(QString("  TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));

        writeTrackTag(track, "    FLAGS %1",     TAG_FLAGS);
        writeTrackTag(track, "    ISRC %1",      TAG_ISRC);
        writeTrackTag(track, "    TITLE \"%1\"", TAG_TITLE);

        if( i == 0)
        {
            if (mHasPreGapFile)
            {
                writeLine(QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
                writeLine(QString("    INDEX 01 %1").arg("00:00:00"));
            }
            else
            {
                writeLine(QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(QString("    INDEX 01 %1").arg(index1.toString()));
            }
        }
        else
        {
            if (!index0.isNull())
                writeLine(QString("    INDEX 00 %1").arg((index0 - prevIndex).toString()));

            prevIndex = index1;
            writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
            writeLine(QString("    INDEX 01 %1").arg("00:00:00"));

        }

        writeTrackTag(track, "    REM GENRE \"%1\"",  TAG_GENRE);
        writeTrackTag(track, "    REM DATE %1",       TAG_DATE);
        writeTrackTag(track, "    PERFORMER \"%1\"",  TAG_PERFORMER);
        writeTrackTag(track, "    SONGWRITER \"%1\"", TAG_SONGWRITER);
    }

    mFile.close();
    return true;
}
