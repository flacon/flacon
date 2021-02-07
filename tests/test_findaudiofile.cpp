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

#include "testflacon.h"
#include "tools.h"
#include "cue.h"
#include "disc.h"

#include <QTest>
#include <QString>
#include <QDebug>

/************************************************

 ************************************************/
void TestFlacon::testFindAudioFile()
{
    QFETCH(QString, fileTag);
    QFETCH(QString, cueFileName);
    QFETCH(QString, audioFiles);
    QFETCH(QString, expected);

    QStringList fileTags = fileTag.split(",", QString::SkipEmptyParts);

    QString cueFile = dir() + "/" + cueFileName;
    {
        QStringList cue;
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        for (int i = 0; i < fileTags.count(); ++i) {
            cue << "FILE \"" + fileTags[i].trimmed() + "\" WAVE";
            cue << "  TRACK 01 AUDIO";
            cue << "    TITLE \"Song01\"";
            cue << "    INDEX 01 00:00:00";
        }
        writeTextFile(cueFile, cue);
    }

    foreach (QString f, audioFiles.split(",")) {
        QFile(mAudio_cd_wav).link(dir() + "/" + f.trimmed());
    }

    QStringList      expectedLists = expected.split(",", QString::SkipEmptyParts);
    QVector<CueDisc> cue;
    try {
        cue = CueReader().load(cueFile);
    }
    catch (FlaconError &err) {
        FAIL(QString("Cue isn't valid: %1").arg(err.what()).toLocal8Bit());
    }

    for (int i = 0; i < cue.count(); ++i) {
        Disc disc;
        disc.loadFromCue(cue.at(i));
        QString expected = expectedLists.at(i).trimmed();
        if (expected == "''")
            expected = "";
        else
            expected = dir() + "/" + expected;

        QString real = disc.audioFileName();
        QCOMPARE(real, expected);
    }
}

/************************************************

 ************************************************/
void TestFlacon::testFindAudioFile_data()
{
    QTest::addColumn<QString>("fileTag", nullptr);
    QTest::addColumn<QString>("cueFileName", nullptr);
    QTest::addColumn<QString>("audioFiles", nullptr);
    QTest::addColumn<QString>("expected", nullptr);

    QTest::newRow("01")
            << "Album.wav"
            << "Album.cue"
            << "Album.ape"
            << "Album.ape";

    QTest::newRow("02")
            << "Album.wav"
            << "Garbage.cue"
            << "Album.ape, Garbage.ape"
            << "Album.ape";

    QTest::newRow("03")
            << "Garbage.wav"
            << "Disc.cue"
            << "Disc.ape"
            << "Disc.ape";

    QTest::newRow("04 Multi disc => CueFile_1.ape")
            << "FileTag1.wav, FileTag2.wav"
            << "CueFile.cue"
            << "CueFile.ape,"
               "CueFile_1.ape,"
               "CueFile_2.ape"
            << "CueFile_1.ape,"
               "CueFile_2.ape";

    QTest::newRow("05")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album_Side1.ape, Album_Side2.ape"
            << "Album_Side1.ape, Album_Side2.ape";

    QTest::newRow("06")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album Side1.ape, Album Side2.ape"
            << "Album Side1.ape, Album Side2.ape";

    QTest::newRow("07")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album_Disk1.ape, Album_Disk2.ape"
            << "Album_Disk1.ape, Album_Disk2.ape";

    QTest::newRow("08 Multi disk => CUE+Disk1")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album Disk1.ape, Album Disk2.ape"
            << "Album Disk1.ape, Album Disk2.ape";

    QTest::newRow("09 Multi disk => CUE+[Disk 1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk 1].ape, Album [Disk 2].ape"
            << "Album [Disk 1].ape, Album [Disk 2].ape";

    QTest::newRow("10 Multi disk => CUE+[Disk #1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk #1].ape, Album [Disk #2].ape"
            << "Album [Disk #1].ape, Album [Disk #2].ape";

    QTest::newRow("11 Multi disk => CUE+[Disk №1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk №1].ape, Album [Disk №2].ape"
            << "Album [Disk №1].ape, Album [Disk №2].ape";

    QTest::newRow("12 Multi disk => CUE+Disk 001")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Album Disk 001.ape,"
               "Album Disk 002.ape"
            << "Album Disk 001.ape,"
               "Album Disk 002.ape";

    QTest::newRow("13")
            << "FileTag1.wav,"
               "FileTag2.wav"
            << "CueFile.cue"
            << "FileTag1.wav,"
               "FileTag2.wav,"
               "CueFile.ape,"
               "CueFile Disk1.ape,"
               "CueFile Disk2.ape"
            << "FileTag1.wav,"
               "FileTag2.wav";

    QTest::newRow("14 Not confuse 1 and 11")
            << "FileTag1.wav,"
               "FileTag2.wav"
            << "CueFile.cue"
            << "CueFile_11.flac,"
               "CueFile_2.flac"
            << "'',"
               "CueFile_2.flac";

    QTest::newRow("15 Multi disk => Side 1")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Side 1.flac,"
               "Side 2.flac"
            << "Side 1.flac,"
               "Side 2.flac";

    QTest::newRow("16 Multi disk => Disk 0001")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Disk 0001.ape,"
               "Disk 0002.ape"
            << "Disk 0001.ape,"
               "Disk 0002.ape";

    QTest::newRow("17 Multi dot")
            << "FileTag.wav"
            << "CueFile.cue"
            << "CueFile.ape.flac"
            << "CueFile.ape.flac";

    QTest::newRow("18 AditionalText")
            << "FileTag.wav"
            << "CueFile.cue"
            << "CueFile_AditionalText.flac"
            << "CueFile_AditionalText.flac";

    QTest::newRow("19 Bush - (1996) Razorblade Suitcase (Issue #31)")
            << "Bush - (1996) Razorblade Suitcase.flac"
            << "Bush - (1996) Razorblade Suitcase.cue"
            << "3 Doors Down - (2000) The Better Life.flac,"
               "Bush - (1996) Razorblade Suitcase.flac"
            << "Bush - (1996) Razorblade Suitcase.flac";

    QTest::newRow("20 FileTag(1) and FileTag(2)")
            << "FileTag(1).wav,"
               "FileTag(2).wav"
            << "CueFile.cue"
            << "FileTag(1).wav,"
               "FileTag(2).wav,"
               "CueFile.ape,"
               "CueFile Disk1.ape,"
               "CueFile Disk2.ape"
            << "FileTag(1).wav,"
               "FileTag(2).wav";
}
