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
#include <QDir>

using namespace Conv;

/************************************************

 ************************************************/
CueCreator::CueCreator(const Profile &profile, const Disc *disk, PreGapType preGapType) :
    mDisk(disk),
    mProfile(profile),
    mPreGapType(preGapType)
{
}

/**************************************
 *
 **************************************/
void CueCreator::writeLine(QIODevice *out, const QString &text) const
{
    out->write(text.toUtf8());
    out->write("\n");
}

/**************************************
 *
 **************************************/
void CueCreator::writeTag(QIODevice *out, const QString &format, const QString &value) const
{
    if (!value.isEmpty()) {
        writeLine(out, format.arg(value));
    }
}

/**************************************
 *
 **************************************/
void CueCreator::writeTrackTags(QIODevice *out, const Track *track) const
{
    CueFlags flags(track->flagsTag());
    flags.preEmphasis = false; // We already deephasis audio, so we reset this flag
    if (!flags.isEmpty()) {
        writeLine(out, QString("    FLAGS %1").arg(flags.toString()));
    }

    writeTag(out, "    ISRC %1", track->isrcTag());
    writeTag(out, "    TITLE \"%1\"", track->titleTag());
    // writeTag(out, "    REM GENRE \"%1\"", track.ge .TagId::Genre);
    writeTag(out, "    REM DATE %1", track->dateTag());
    writeTag(out, "    PERFORMER \"%1\"", track->performerTag());
    writeTag(out, "    SONGWRITER \"%1\"", track->songWriterTag());
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
    bool createPreGapFile = mPreGapType == PreGapType::ExtractToFile && mDisk->tracks().first()->cueIndex01().milliseconds() > 0;

    // Common ...........................
    writeTag(out, "CATALOG %1", mDisk->catalogTag());
    writeTag(out, "CDTEXTFILE \"%1\"", mDisk->cdTextfileTag());
    writeTag(out, "REM GENRE \"%1\"", mDisk->genreTag());
    writeTag(out, "REM DATE %1", mDisk->dateTag());
    writeTag(out, "REM DISCID %1", mDisk->discIdTag());
    if (mDisk->discNumTag() != 1 || mDisk->discCountTag() != 1) {
        writeTag(out, "REM TOTALDISCS %1", QString::number(mDisk->discCountTag()));
        writeTag(out, "REM DISCNUMBER %1", QString::number(mDisk->discNumTag()));
    }
    writeTag(out, "PERFORMER \"%1\"", mDisk->performerTag());
    writeTag(out, "SONGWRITER \"%1\"", mDisk->songWriterTag());
    writeTag(out, "TITLE \"%1\"", mDisk->albumTag());

    // Tracks ...........................
    CueTime prevIndex("00:00:00");
    for (int i = 0; i < mDisk->tracks().count(); ++i) {
        Track   *track  = mDisk->tracks().at(i);
        CueIndex index0 = track->cueIndex00();
        CueIndex index1 = track->cueIndex01();
        writeLine(out, "");
        if (i == 0) {
            if (index0.isNull() || index0 == index1) {
                // No pregap ....................
                writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFileName(track)).fileName()));
                writeLine(out, QString("TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));
                writeTrackTags(out, track);
                writeLine(out, QString("INDEX 01 %1").arg("00:00:00"));
            }
            else {
                // With pregap ..................
                if (createPreGapFile) {
                    PregapTrack preGapTrack(*mDisk->tracks().first());
                    writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFilePath(&preGapTrack)).fileName()));
                    writeLine(out, QString("TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));
                    writeTrackTags(out, track);
                    writeLine(out, QString("INDEX 00 %1").arg("00:00:00"));
                    writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFileName(track)).fileName()));
                    writeLine(out, QString("INDEX 01 %1").arg("00:00:00"));
                }
                else {
                    writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFileName(track)).fileName()));
                    writeLine(out, QString("TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));
                    writeTrackTags(out, track);
                    writeLine(out, QString("INDEX 00 %1").arg(index0.toString()));
                    writeLine(out, QString("INDEX 01 %1").arg(index1.toString()));
                }
            }
        }
        else {
            if (index0.isNull() || index0 == index1) {
                // No pregap ....................
                writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFileName(track)).fileName()));
                writeLine(out, QString("TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));
                writeTrackTags(out, track);
                writeLine(out, QString("INDEX 01 %1").arg("00:00:00"));
            }
            else {
                // With pregap ..................
                writeLine(out, QString("TRACK %1 AUDIO").arg(i + 1, 2, 10, QChar('0')));
                writeTrackTags(out, track);
                writeLine(out, QString("INDEX 00 %1").arg((index0 - prevIndex).toString()));
                writeLine(out, QString("FILE \"%1\" WAVE").arg(QFileInfo(mProfile.resultFileName(track)).fileName()));
                writeLine(out, QString("INDEX 01 %1").arg("00:00:00"));
            }
        }
        prevIndex = index1;
    }
}

QString CueCreator::writeToFile(const QString &fileTemplate)
{
    Track          *track = mDisk->tracks().first();
    QString         dir   = QFileInfo(mProfile.resultFilePath(track)).dir().absolutePath();
    PatternExpander expander(*track);
    expander.trackTags().setTrackNum(0);
    expander.albumTags().setTrackCount(mDisk->tracks().count());
    expander.albumTags().setDiscNum(mDisk->discNumTag());
    expander.albumTags().setDiscCount(mDisk->discCountTag());

    QString fileName = expander.expand(fileTemplate);

    if (fileName.endsWith(".cue")) {
        fileName = fileName.left(fileName.length() - 4);
    }

    if (track->trackNumTag() > 1) {
        fileName += QString(".tracks %1-%2").arg(track->trackNumTag()).arg(mDisk->tracks().last()->trackNumTag());
    }

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
