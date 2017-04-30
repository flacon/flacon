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
#include "decoder.h"

#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QRegExp>
#include <QTextCodec>
#include <QUuid>
#include <QDebug>

//#define DEBUG_CUE_ON

#ifdef DEBUG_CUE_ON
#define DEBUG_CUE qDebug()
#else
#define DEBUG_CUE QT_NO_QDEBUG_MACRO()
#endif

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
Splitter::Splitter(Disk *disk, const OutFormat *format, QObject *parent):
    ConverterThread(disk, format, parent),
    mProcess(0),
    mTrack(NULL)
{    
    QString tmpDir = settings->value(Settings::Encoder_TmpDir).toString();

    if (tmpDir.isEmpty())
        mWorkDir = QFileInfo(disk->track(0)->resultFilePath()).dir().absolutePath();
    else
        mWorkDir = QDir(QString("%1/flacon.%2").arg(tmpDir).arg(QCoreApplication::applicationPid())).absolutePath();
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

struct SplitterRequest {
    CueTime start;
    CueTime end;
    QString outFileName;
    Track *track;
};

/************************************************
 Split audio file to temporary dir
 ************************************************/
void Splitter::doRun()
{
    mTrack = disk()->track(0);
    Decoder decoder;
    connect(&decoder, SIGNAL(progress(int)),
            this, SLOT(decoderProgress(int)));

    if (!decoder.open(disk()->audioFileName()))
    {
        error(mTrack,
              tr("I can't read <b>%1</b>:<br>%2",
                 "Splitter error. %1 is a file name, %2 is a system error text.")
              .arg(disk()->audioFileName())
              .arg(decoder.errorString()));
        return;
    }

    bool separatePregap = format()->createCue() &&
                          disk()->track(0)->cueIndex(1).milliseconds() > 0 &&
                          OutFormat::currentFormat()->preGapType() == OutFormat::PreGapExtractToFile;


    bool embededPregap  = format()->createCue() &&
                          disk()->track(0)->cueIndex(1).milliseconds() > 0 &&
                          OutFormat::currentFormat()->preGapType() == OutFormat::PreGapAddToFirstTrack;

    QList<SplitterRequest> requests;


    // Extract pregap to separate file ....................
    if (separatePregap)
    {
        SplitterRequest req;
        req.track = disk()->preGapTrack();
        req.outFileName = QDir::toNativeSeparators(QString("%1/flacon_%2_%3.wav").arg(mWorkDir).arg("00").arg(QUuid::createUuid().toString().mid(1, 36)));
        req.start = disk()->track(0)->cueIndex(0);
        req.end   = disk()->track(0)->cueIndex(1);
        requests << req;
    }
    // Extract pregap to separate file ....................

    for (int i=0; i<disk()->count(); ++i)
    {
        SplitterRequest req;
        req.track = disk()->track(i);
        req.outFileName = QDir::toNativeSeparators(QString("%1/flacon_%2_%3.wav").arg(mWorkDir).arg(i, 2, 10, QChar('0')).arg(QUuid::createUuid().toString().mid(1, 36)));

        if (i==0 && embededPregap)
            req.start = CueTime("00:00:00");
        else
            req.start = disk()->track(i)->cueIndex(1);

        if (i<disk()->count()-1)
            req.end = disk()->track(i+1)->cueIndex(01);

        requests << req;
    }

    foreach (SplitterRequest req, requests)
    {
        mTrack = req.track;
        bool ret = decoder.extract(req.start, req.end, req.outFileName);
        if (!ret)
        {
            qWarning() << "Splitter error for track " << req.track->index() << ": " <<  decoder.errorString();
            deleteFile(req.outFileName);
            error(mTrack, decoder.errorString());
            return;
        }

        emit trackReady(req.track, req.outFileName);
    }


    if (OutFormat::currentFormat()->createCue())
    {
        CueCreator cue(disk());
        cue.setHasPregapFile(separatePregap);
        if (!cue.write())
            error(disk()->track(0), cue.errorString());
    }
}

/************************************************
 *
 ***********************************************/
void Splitter::decoderProgress(int percent)
{
    emit trackProgress(mTrack, Track::Splitting, percent);
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
        mErrorString = QObject::tr("I can't write cue file <b>%1</b>:<br>%2").arg(mFile.fileName(), mFile.errorString());
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
