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
#include "inputaudiofile.h"
#include "disc.h"

#include <QTest>
#include <QString>
#include <QDebug>

struct TestCueFile_
{
    QString name;
    QString fileTag;
    QString fileTag2;
};
Q_DECLARE_METATYPE(TestCueFile_)

struct TestFindCueFileData
{
    QList<TestCueFile_> cueFiles;
    QStringList         audioFiles;
    QString             chekAudioFile;
    QString             expected;
};
Q_DECLARE_METATYPE(TestFindCueFileData)

/************************************************

 ************************************************/
void TestFlacon::testFindCueFile()
{
    QFETCH(TestFindCueFileData, test);

    foreach (QString f, test.audioFiles) {
        QFile(mAudio_cd_wav).link(dir() + "/" + f.trimmed());
    }

    foreach (TestCueFile_ cueFile, test.cueFiles) {
        QStringList cue;
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        cue << cueFile.fileTag.trimmed();
        cue << "  TRACK 01 AUDIO";
        cue << "    TITLE \"Song01\"";
        cue << "    INDEX 01 00:00:00";

        if (!cueFile.fileTag2.isEmpty()) {
            cue << cueFile.fileTag2.trimmed();
            cue << "  TRACK 01 AUDIO";
            cue << "    TITLE \"Song01\"";
            cue << "    INDEX 01 00:00:00";
        }
        writeTextFile(dir() + "/" + cueFile.name, cue);
    }

    InputAudioFile audio(dir() + "/" + test.chekAudioFile);

    Disc disc(audio);
    disc.searchCueFile();

    QString real     = disc.cueFilePath();
    QString expected = dir() + "/" + test.expected;
    QCOMPARE(real, expected);
}

/************************************************

 ************************************************/
void TestFlacon::testFindCueFile_data()
{

    QTest::addColumn<TestFindCueFileData>("test", nullptr);
    TestFindCueFileData test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "1.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";

    test.audioFiles << "1.wav";

    test.chekAudioFile = "1.wav";
    test.expected      = "1.cue";
    QTest::newRow("01 1.cue 1.wav") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "1.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "2.cue";
    test.cueFiles.last().fileTag = "FILE \"2.wav\" WAVE";

    test.audioFiles << "1.wav";
    test.audioFiles << "2.wav";

    test.chekAudioFile = "1.wav";
    test.expected      = "1.cue";
    QTest::newRow("02 1.cue 2.cue *1.wav* 2.wav") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "1.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "2.cue";
    test.cueFiles.last().fileTag = "FILE \"2.wav\" WAVE";

    test.audioFiles << "1.wav";
    test.audioFiles << "2.wav";

    test.chekAudioFile = "2.wav";
    test.expected      = "2.cue";
    QTest::newRow("03 1.cue 2.cue 1.wav *2.wav*") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name     = "multi.cue";
    test.cueFiles.last().fileTag  = "FILE \"1.wav\" WAVE";
    test.cueFiles.last().fileTag2 = "FILE \"2.wav\" WAVE";

    test.audioFiles << "multi_1.wav";
    test.audioFiles << "multi_2.wav";

    test.chekAudioFile = "multi_1.wav";
    test.expected      = "multi.cue";
    QTest::newRow("04 multi.cue multi_1.wav multi_2.wav") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name     = "multi.cue";
    test.cueFiles.last().fileTag  = "FILE \"1.wav\" WAVE";
    test.cueFiles.last().fileTag2 = "FILE \"2.wav\" WAVE";

    test.audioFiles << "multi_1.wav";
    test.audioFiles << "multi_2.wav";

    test.chekAudioFile = "multi_2.wav";
    test.expected      = "multi.cue";
    QTest::newRow("05 multi.cue multi_1.wav multi_2.wav") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "ISO-8859-2.cue";
    test.cueFiles.last().fileTag = "FILE \"short.flac\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "short.cue";
    test.cueFiles.last().fileTag = "FILE \"short.wav\" WAVE";

    test.audioFiles << "short.wav";
    test.audioFiles << "short.flac";

    test.chekAudioFile = "short.flac";
    test.expected      = "short.cue";
    QTest::newRow("06 short.cue and ISO-8859-2.cue") << test;

    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "foo.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name    = "bar.cue";
    test.cueFiles.last().fileTag = "FILE \"2.wav\" WAVE";

    test.audioFiles << "1.wav";
    test.audioFiles << "2.wav";

    test.chekAudioFile = "2.wav";
    test.expected      = "bar.cue";
    QTest::newRow("07 by File tag") << test;
}
