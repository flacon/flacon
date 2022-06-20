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
CueCreator::CueCreator(const Disc *disc, PreGapType preGapType) :
    mDisc(disc),
    mPreGapType(preGapType)
{

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
void CueCreator::writeLine(QIODevice *out, const QString &text)
{
    out->write(mTextCodec->fromUnicode(text));
    out->write("\n");
}

/************************************************

 ************************************************/
void CueCreator::writeGlobalTag(QIODevice *out, const QString &format, TagId tagId)
{
    QString value = mGlobalTags.tag(tagId);

    if (!value.isEmpty()) {
        writeLine(out, format.arg(value));
    }
}

/************************************************

 ************************************************/
void CueCreator::writeTrackTag(QIODevice *out, const Track *track, const QString &prefix, TagId tagId)
{
    QString value = track->tag(tagId);

    if (!value.isEmpty() && value != mGlobalTags.tag(tagId)) {
        writeLine(out, prefix.arg(value));
    }
}

/************************************************

 ************************************************/
void CueCreator::write(QIODevice *out)
{
    if (!out->isOpen()) {
        if (!out->open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw FlaconError(QObject::tr("I can't write CUE:<br>%1").arg(out->errorString()));
        }
    }
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    bool createPreGapFile = mPreGapType == PreGapType::ExtractToFile && mDisc->track(0)->cueIndex(1).milliseconds() > 0;

    initGlobalTags();

    // Common ...........................
    writeGlobalTag(out, "CATALOG %1", TagId::Catalog);
    writeGlobalTag(out, "CDTEXTFILE \"%1\"", TagId::CDTextfile);
    writeGlobalTag(out, "REM GENRE \"%1\"", TagId::Genre);
    writeGlobalTag(out, "REM DATE %1", TagId::Date);
    writeGlobalTag(out, "REM DISCID %1", TagId::DiscId);
    writeGlobalTag(out, "REM TOTALDISCS %1", TagId::DiscCount);
    writeGlobalTag(out, "REM DISCNUMBER %1", TagId::DiscNum);
    writeGlobalTag(out, "PERFORMER \"%1\"", TagId::Artist);
    writeGlobalTag(out, "SONGWRITER \"%1\"", TagId::SongWriter);
    writeGlobalTag(out, "TITLE \"%1\"", TagId::Album);

    if (createPreGapFile)
        writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisc->preGapTrack()->resultFilePath()).fileName()));
    else
        writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mDisc->track(0)->resultFilePath()).fileName()));

    // Tracks ...........................
    CueTime prevIndex("00:00:00");
    for (int i = 0; i < mDisc->count(); ++i) {
        Track   *track  = mDisc->track(i);
        CueIndex index0 = track->cueIndex(0);
        CueIndex index1 = track->cueIndex(1);

        writeLine(out, QString("  TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));

        CueFlags flags(track->tag(TagId::Flags));
        flags.preEmphasis = false; // We already deephasis audio, so we reset this flag
        if (!flags.isEmpty()) {
            writeLine(out, QString("    FLAGS %1").arg(flags.toString()));
        }

        writeTrackTag(out, track, "    ISRC %1", TagId::ISRC);
        writeTrackTag(out, track, "    TITLE \"%1\"", TagId::Title);

        if (i == 0) {
            if (createPreGapFile) {
                writeLine(out, QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
                writeLine(out, QString("    INDEX 01 %1").arg("00:00:00"));
            }
            else {
                writeLine(out, QString("    INDEX 00 %1").arg("00:00:00"));
                writeLine(out, QString("    INDEX 01 %1").arg(index1.toString()));
            }
        }
        else {
            if (!index0.isNull() && index0 != index1)
                writeLine(out, QString("    INDEX 00 %1").arg((index0 - prevIndex).toString()));

            prevIndex = index1;
            writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(track->resultFileName()).fileName()));
            writeLine(out, QString("    INDEX 01 %1").arg("00:00:00"));
        }

        writeTrackTag(out, track, "    REM GENRE \"%1\"", TagId::Genre);
        writeTrackTag(out, track, "    REM DATE %1", TagId::Date);
        writeTrackTag(out, track, "    PERFORMER \"%1\"", TagId::Artist);
        writeTrackTag(out, track, "    SONGWRITER \"%1\"", TagId::SongWriter);
    }
}

QString CueCreator::writeToFile(const QString &fileTemplate)
{
    Track          *track = mDisc->track(0);
    QString         dir   = QFileInfo(track->resultFilePath()).dir().absolutePath();
    PatternExpander expander(*track);
    expander.setTrackNum(0);
    expander.setTrackCount(mDisc->count());
    expander.setDiscNum(mDisc->discNum());
    expander.setDiscCount(mDisc->discCount());

    QString fileName = expander.expand(fileTemplate);

    if (!fileName.endsWith(".cue"))
        fileName += ".cue";

    QFile file;
    file.setFileName(dir + QDir::separator() + fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw FlaconError(QObject::tr("I can't write CUE file <b>%1</b>:<br>%2").arg(file.fileName(), file.errorString()));
    }

    write(&file);
    file.close();

    return fileName;
}
