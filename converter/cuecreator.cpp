/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "cuecreator.h"
#include "disk.h"

#include <QFileInfo>
#include <QTextCodec>
#include <QDir>

/************************************************

 ************************************************/
CueCreator::CueCreator(const Disk *disk, PreGapType preGapType):
    mDisk(disk),
    mPreGapType(preGapType)
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
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    bool createPreGapFile = mPreGapType == PreGapType::ExtractToFile &&
                            mDisk->track(0)->cueIndex(1).milliseconds() > 0;

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
    writeLine(QString("REM COMMENT \"Flacon v%1\"").arg(FLACON_VERSION));
    writeGlobalTag("PERFORMER \"%1\"",  TAG_PERFORMER);
    writeGlobalTag("SONGWRITER \"%1\"", TAG_SONGWRITER);
    writeGlobalTag("TITLE \"%1\"",      TAG_ALBUM);

    if (createPreGapFile)
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
            if (createPreGapFile)
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
