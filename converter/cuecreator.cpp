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
#include "disc.h"
#include "patternexpander.h"

#include <QFileInfo>
#include <QTextCodec>
#include <QDir>

using namespace Conv;

/************************************************

 ************************************************/
CueCreator::CueCreator(const Disc *disc, PreGapType preGapType, const QString &fileTemplate) :
    mDisc(disc),
    mPreGapType(preGapType)
{
    Track *         track = mDisc->track(0);
    QString         dir   = QFileInfo(track->resultFilePath()).dir().absolutePath();
    PatternExpander expander(*track);
    expander.setTrackNum(0);
    expander.setTrackCount(mDisc->count());
    expander.setDiscNum(mDisc->discNum());
    expander.setDiscCount(mDisc->discCount());

    QString fileName = expander.expand(fileTemplate);

    if (!fileName.endsWith(".cue"))
        fileName += ".cue";

    mFile.setFileName(dir + QDir::separator() + fileName);
    setTextCodecName("UTF-8");
}

/************************************************

 ************************************************/
void CueCreator::setTextCodecName(const QString &codecName)
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
    static TagId tags[] = {
        TagId::Genre,
        TagId::Date,
        TagId::Artist,
        TagId::SongWriter,
        TagId::Album,
        TagId::Catalog,
        TagId::CDTextfile,
        TagId::DiscId,
        TagId::DiscCount,
        TagId::DiscNum
    };

    mGlobalTags.setCodecName(mTextCodec->name());

    Track *firstTrack = mDisc->track(0);
    for (uint t = 0; t < sizeof(tags) / sizeof(TagId); ++t) {
        TagId   tagId = tags[t];
        QString value = firstTrack->tag(tagId);

        for (int i = 1; i < mDisc->count(); ++i) {
            if (mDisc->track(i)->tag(tagId) != value) {
                value.clear();
                break;
            }
        }

        if (!value.isEmpty())
            mGlobalTags.setTag(tagId, value);
    }

    // Don't write defaults values
    if (mGlobalTags.tag(TagId::DiscCount) == "1" && mGlobalTags.tag(TagId::DiscNum) == "1") {
        mGlobalTags.setTag(TagId::DiscCount, QByteArray());
        mGlobalTags.setTag(TagId::DiscNum, QByteArray());
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
void CueCreator::writeGlobalTag(const QString &format, TagId tagId)
{
    QString value = mGlobalTags.tag(tagId);

    if (!value.isEmpty())
        writeLine(format.arg(value));
}

/************************************************

 ************************************************/
void CueCreator::writeTrackTag(const Track *track, const QString &prefix, TagId tagId)
{
    QString value = track->tag(tagId);

    if (!value.isEmpty() && value != mGlobalTags.tag(tagId))
        writeLine(prefix.arg(value));
}

/************************************************

 ************************************************/
bool CueCreator::write()
{
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    bool createPreGapFile = mPreGapType == PreGapType::ExtractToFile && mDisc->track(0)->cueIndex(1).milliseconds() > 0;

    if (!mFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mErrorString = QObject::tr("I can't write CUE file <b>%1</b>:<br>%2").arg(mFile.fileName(), mFile.errorString());
        return false;
    }

    initGlobalTags();

    // Common ...........................
    writeGlobalTag("CATALOG %1", TagId::Catalog);
    writeGlobalTag("CDTEXTFILE \"%1\"", TagId::CDTextfile);
    writeGlobalTag("REM GENRE \"%1\"", TagId::Genre);
    writeGlobalTag("REM DATE %1", TagId::Date);
    writeGlobalTag("REM DISCID %1", TagId::DiscId);
    writeLine(QString("REM COMMENT \"Flacon v%1\"").arg(FLACON_VERSION));
    writeGlobalTag("REM TOTALDISCS %1", TagId::DiscCount);
    writeGlobalTag("REM DISCNUMBER %1", TagId::DiscNum);
    writeGlobalTag("PERFORMER \"%1\"", TagId::Artist);
    writeGlobalTag("SONGWRITER \"%1\"", TagId::SongWriter);
    writeGlobalTag("TITLE \"%1\"", TagId::Album);

    if (createPreGapFile)
        writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisc->preGapTrack()->resultFilePath()).fileName()));
    else
        writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisc->track(0)->resultFilePath()).fileName()));

    // Tracks ...........................
    CueIndex prevIndex("00:00:00");
    for (int i = 0; i < mDisc->count(); ++i) {
        Track *  track  = mDisc->track(i);
        CueIndex index0 = track->cueIndex(0);
        CueIndex index1 = track->cueIndex(1);

        writeLine(QString("  TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));

        writeTrackTag(track, "    FLAGS %1", TagId::Flags);
        writeTrackTag(track, "    ISRC %1", TagId::ISRC);
        writeTrackTag(track, "    TITLE \"%1\"", TagId::Title);

        if (i == 0) {
            if (createPreGapFile) {
                writeLine(QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
                writeLine(QString("    INDEX 01 %1").arg("00:00:00"));
            }
            else {
                writeLine(QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(QString("    INDEX 01 %1").arg(index1.toString()));
            }
        }
        else {
            if (!index0.isNull())
                writeLine(QString("    INDEX 00 %1").arg((index0 - prevIndex).toString()));

            prevIndex = index1;
            writeLine(QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
            writeLine(QString("    INDEX 01 %1").arg("00:00:00"));
        }

        writeTrackTag(track, "    REM GENRE \"%1\"", TagId::Genre);
        writeTrackTag(track, "    REM DATE %1", TagId::Date);
        writeTrackTag(track, "    PERFORMER \"%1\"", TagId::Artist);
        writeTrackTag(track, "    SONGWRITER \"%1\"", TagId::SongWriter);
    }

    mFile.close();
    return true;
}
