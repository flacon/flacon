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


#include "testflacon.h"
#include "tools.h"
#include "../disk.h"
#include "../settings.h"
#include "../project.h"
#include "../inputaudiofile.h"
#include "converter/converter.h"
#include "converter/wavheader.h"
#include "converter/splitter.h"
#include "outformat.h"

#include <QTest>
#include <QDebug>
#include <QProcess>
#include <QBuffer>
#include <QFileInfo>
#include <QDir>
#include <QThreadPool>

#define protected public;


int TestFlacon::mTestNum = -1;


/************************************************

 ************************************************/
TestFlacon::TestFlacon(QObject *parent) :
    QObject(parent),
    mTmpDir(TEST_OUT_DIR),
    mDataDir(TEST_DATA_DIR),
    mStandardDisk(0)
{
}


/************************************************

 ************************************************/
void TestFlacon::writeTextFile(const QString &fileName, const QString &content)
{
    QFile file(fileName);
    file.open(QFile::WriteOnly | QFile::Truncate);
    file.write(content.toLocal8Bit());
    file.close();
}


/************************************************

 ************************************************/
void TestFlacon::writeTextFile(const QString &fileName, const QStringList &content)
{
    QFile file(fileName);
    file.open(QFile::WriteOnly | QFile::Truncate);
    for(int i=0; i<content.count(); ++i)
    {
        file.write(content.at(i).toLocal8Bit());
        file.write("\n");
    }
    file.close();
}


/************************************************

 ************************************************/
bool TestFlacon::removeDir(const QString &dirName) const
{
    QDir dir(dirName);
    if (!dir.exists())
        return true;

    bool res = false;
    foreach(QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (fi.isDir())
            res = removeDir(fi.absoluteFilePath());
        else
            res = QFile::remove(fi.absoluteFilePath());

        if (!res)
            return false;
    }

    return dir.rmdir(dirName);
}


/************************************************

 ************************************************/
bool TestFlacon::clearDir(const QString &dirName) const
{
    QDir dir(dirName);
    if (!dir.exists())
        return true;

    bool res = false;
    foreach(QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (fi.isDir())
            res = removeDir(fi.absoluteFilePath());
        else
            res = QFile::remove(fi.absoluteFilePath());

        if (!res)
            return false;
    }

    return true;
}


/************************************************

 ************************************************/
void TestFlacon::checkFileExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists())
        QFAIL(QString("File not exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
}


/************************************************

 ************************************************/
void TestFlacon::checkFileNotExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (fi.exists())
        QFAIL(QString("File exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
}


/************************************************

 ************************************************/
void TestFlacon::applySettings(const SettingsValues &config)
{
    SettingsValues::const_iterator i;
    i = config.begin();
    for (i = config.begin(); i != config.end(); ++i)
    {
        settings->setValue(i.key(), i.value());
    }
    settings->sync();
}




/************************************************

 ************************************************/
Disk *TestFlacon::standardDisk()
{
    if (mStandardDisk == 0)
    {
        QString cueFile = dir() + "testTrackResultFileName.cue";

        QStringList cue;
        cue << "REM GENRE \"Genre\"";
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        cue << "TITLE \"Album\"";
        cue << "FILE \"en.wav\" WAVE";
        cue << "  TRACK 01 AUDIO";
        cue << "    TITLE \"Song01\"";
        cue << "    INDEX 01 00:00:00";
        cue << "  TRACK 02 AUDIO";
        cue << "    TITLE \"Song02\"";
        cue << "    INDEX 01 03:39:10";
        cue << "  TRACK 03 AUDIO";
        cue << "    TITLE \"Song03\"";
        cue << "    INDEX 01 07:25:42";
        cue << "  TRACK 04 AUDIO";
        cue << "    TITLE \"Song04\"";
        cue << "    INDEX 01 12:04:72";

        writeTextFile(cueFile, cue);
        mStandardDisk = loadFromCue(cueFile);
    }

    return mStandardDisk;
}


/************************************************

 ************************************************/
bool TestFlacon::compareCue(const QString &result, const QString &expected, QString *error)
{
    QFile resFile(result);
    resFile.open(QFile::ReadOnly);
    QByteArray resData = resFile.readAll();
    resFile.close();
    resData.replace("\r\n", "\n");

    QFile expFile(expected);
    expFile.open(QFile::ReadOnly);
    QByteArray expData = expFile.readAll();
    expFile.close();
    expData.replace("\r\n", "\n");

    if (resData != expData)
    {
        QString s = "The result is different from the expected. Use the following command for details: \n diff %1 %2";
        *error = s.arg(expected, result);
        return false;
    }

    return true;
}


/************************************************

 ************************************************/
void TestFlacon::testByteArraySplit()
{
    QFETCH(QByteArray,  inArray);
    QFETCH(char,        separator);
    QFETCH(QByteArray,  expectedLeft);
    QFETCH(QByteArray,  expectedRight);

    QByteArray left = leftPart(inArray, separator);
    QCOMPARE(left, expectedLeft);

    QByteArray right = rightPart(inArray, separator);
    QCOMPARE(right, expectedRight);
}


/************************************************

 ************************************************/
void TestFlacon::testByteArraySplit_data()
{
    QTest::addColumn<QByteArray>("inArray");
    QTest::addColumn<char>("separator");
    QTest::addColumn<QByteArray>("expectedLeft");
    QTest::addColumn<QByteArray>("expectedRight");

    QTest::newRow("'' split by ' '")
            << QByteArray()
            << ' '
            << QByteArray()
            << QByteArray();


    QTest::newRow("'Test' split by ' '")
            << QByteArray("Test")
            << ' '
            << QByteArray("Test")
            << QByteArray();


    QTest::newRow("'REM GENRE Pop' split by ' '")
            << QByteArray("REM GENRE Pop")
            << ' '
            << QByteArray("REM")
            << QByteArray("GENRE Pop");


    QTest::newRow("'TITLE Рок группа' split by ' '")
            << QByteArray("TITLE Рок группа")
            << ' '
            << QByteArray("TITLE")
            << QByteArray("Рок группа");


    QTest::newRow("'Название Рок группа' split by ' '")
            << QByteArray("Название Рок группа")
            << ' '
            << QByteArray("Название")
            << QByteArray("Рок группа");


    QTest::newRow("'Название=Рок группа' split by '='")
            << QByteArray("Название=Рок группа")
            << '='
            << QByteArray("Название")
            << QByteArray("Рок группа");

}


/************************************************

 ************************************************/
void TestFlacon::testSafeString()
{
    QCOMPARE(Disk::safeString("A|B/C|D\\E:F*G?H"), QString("A-B-C-D-E-F-G-H"));
}



/************************************************

 ************************************************/
QStringList TestFlacon::readFile(const QString &fileName)
{
    QStringList res;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isOpen())
    {
        FAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());
        return res;
    }

    QTextStream stream(&file);
    while(!stream.atEnd())
    {
        QString str = stream.readLine();
        res << str << "\n";
    }

    file.close();
    return res;
}


/************************************************

 ************************************************/
void TestFlacon::writeFile(const QStringList &strings, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    if (!file.isOpen())
        QFAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());

    foreach(const QString &string, strings)
    {
        file.write(string.toLocal8Bit());
    }
}


/************************************************

 ************************************************/
QString TestFlacon::stigListToString(const QStringList &strings, const QString divider)
{
    return  "\n--------------------------\n" +
            strings.join(divider) +
            "\n--------------------------\n";
}



/************************************************

 ************************************************/
QStringList &operator<<(QStringList &list, int value)
{
    return list << QString("%1").arg(value);
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName()
{
    QFETCH(QString, cue);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);


    settings->setOutFilePattern(pattern);
    settings->setOutFormat("WAV");

    project->clear();

    QString cueFile = dir() + "/input.cue";
    writeTextFile(cueFile, cue);

    Disk *disk = loadFromCue(cueFile);

    QString result = disk->track(0)->resultFileName();
    //QCOMPARE(result, expected);

    if (result != expected)
    {
        QString msg = QString("Compared values are not the same\n   Pattern   %1\n   Actual:   %2\n   Expected: %3").arg(
                    pattern,
                    result,
                    expected);
        QFAIL(msg.toLocal8Bit());
    }
    disk->deleteLater();
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName_data()
{
    QTest::addColumn<QString>("cue");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");


    QTest::newRow("1.1")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"
            << "%a/%y - %A/%n - %t"
            << "Artist/2013 - Album/01 - Song01.wav";

    QTest::newRow("1.2")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("1.3")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("1.4")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "/%%/%Q/%N/%n/%A/%a/%t/%y/%g/%%"
            << "/%/%Q/04/01/Album/Artist/Song01/2013/Genre/%.wav";


    QTest::newRow("1.5")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "%%Q/%%N/%%n/%%A/%%a/%%t/%%y/%%g"
            << "%Q/%N/%n/%A/%a/%t/%y/%g.wav";

    QTest::newRow("1.6")
            <<  "REM GENRE \"Genre\"\n"
                "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "TITLE \"Album\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "%%%Q/%%%N/%%%n/%%%A/%%%a/%%%t/%%%y/%%%g/%%%"
            << "%%Q/%04/%01/%Album/%Artist/%Song01/%2013/%Genre/%%.wav";

    QTest::newRow("2.1")
            <<  "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "{}/{Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "{}/{Text}/01/01 Text//.wav";


    QTest::newRow("2.2")
            <<  "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "Test{Text/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Test{Text/01/01 Text//.wav";


    QTest::newRow("2.3")
            <<  "REM DATE 2013\n"
                "REM DISCID 123456789\n"
                "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
                "PERFORMER \"Artist\"\n"
                "FILE \"en.wav\" WAVE\n"
                "  TRACK 01 AUDIO\n"
                "    TITLE \"Song01\"\n"
                "    INDEX 01 00:00:00\n"
                "  TRACK 02 AUDIO\n"
                "    TITLE \"Song02\"\n"
                "    INDEX 01 03:39:10\n"
                "  TRACK 03 AUDIO\n"
                "    TITLE \"Song03\"\n"
                "    INDEX 01 07:25:42\n"
                "  TRACK 04 AUDIO\n"
                "    TITLE \"Song04\"\n"
                "    INDEX 01 12:04:72\n"

            << "Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Text}/01/01 Text//.wav";

}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath()
{
    QFETCH(QString, outDir);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);
    QFETCH(QString, audioFile);

    settings->setOutFileDir(outDir);
    settings->setOutFilePattern(pattern);
    settings->setOutFormat("WAV");

    Disk *disk = loadFromCue(mDataDir + "simple.cue");

    if (!audioFile.isEmpty())
    {
        InputAudioFile audio(audioFile);
        if (!audio.isValid())
            QFAIL(audio.errorString().toLocal8Bit().data());

        disk->setAudioFile(audio);
    }

    QString result = disk->track(0)->resultFilePath();
    if (QFileInfo(result).absoluteFilePath() != QFileInfo(expected).absoluteFilePath())
    {
        QString msg = QString("Compared values are not the same\n   Actual:   %1 [%2]\n   Expected: %3\n   AudioFile: %4").arg(
                    QFileInfo(result).absoluteFilePath(), result,
                    expected,
                    audioFile);
        QFAIL(msg.toLocal8Bit());
    }
    //QCOMPARE(result, expected);
    disk->deleteLater();
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath_data()
{
    //QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("outDir");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("audioFile");


    QTest::newRow("1: /home/user/music")
            << "/home/user/music"
            << "%a/%y - %A/%n - %t"
            << "/home/user/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("2: ~/music")
            << "~/music"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("3: /music")
            << "/music"
            << "%a/%y - %A/%n - %t"
            << "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("4: empty")
            << ""
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("5: dot (.) with CdAudioFile")
            << "."
            << "%a/%y - %A/%n - %t"
            << mTmpDir + "/Artist/2013 - Album/01 - Song01.wav"
            << mAudio_cd_wav;

    QTest::newRow("6. empty with CdAudioFile")
            << ""
            << "%a/%y - %A/%n - %t"
            << mTmpDir + "/Artist/2013 - Album/01 - Song01.wav"
            << mAudio_cd_wav;

    QTest::newRow("7: ~")
            << "~"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/Artist/2013 - Album/01 - Song01.wav"
            << "";

}


/************************************************

 ************************************************/
void TestFlacon::testTrackSetCodepages()
{
    static int count = 0;
    count++;

    QFETCH(QString, cueFile);
    QFETCH(QString, sampleFile);
    QFETCH(QString, codepageBefore);
    QFETCH(QString, codepageAfter);

    QString testDataDir = TEST_DATA_DIR;

    QString testCueFile  = dir() + "/input.cue";
    QString expectedFile = dir() + "/expected.cue";
    QString resultFile   = dir() + "/result.cue";

    if (QFileInfo(testCueFile).exists())
        QFile(testCueFile).remove();

    if (!QFile::copy(testDataDir + cueFile, testCueFile))
        QFAIL(QString("Can't copy file %1 to %2").arg(testDataDir + cueFile, testCueFile).toLocal8Bit().data());

    if (!codepageBefore.isEmpty())
        settings->setValue(Settings::Tags_DefaultCodepage, codepageBefore);
    else
        settings->setValue(Settings::Tags_DefaultCodepage, "UTF-8");

    Disk *disk = loadFromCue(testCueFile);

    if (!codepageAfter.isEmpty())
        disk->setTextCodecName(codepageAfter);

    QStringList expected = this->readFile(TEST_DATA_DIR + sampleFile);

    QStringList result;
    // Result *************************
    //result << "GENRE:" << tracks.genre() << "\n";
    //resultSl << "ALBUM:" << tracks.album() << "\n";
    result << "DISCID:" << disk->discId() << "\n";

    for(int i=0; i<disk->count(); ++i)
    {
        Track *track = disk->track(i);
        result << "Track " << (i + 1) << "\n";
        result << "  " << "INDEX:"    << track->index()       << "\n";
        result << "  " << "TRACKNUM:" << track->trackNum()    << "\n";
        result << "  " << "ALBUM:"    << track->album()       << "\n";
        result << "  " << "TITLE:"    << track->title()       << "\n";
        result << "  " << "ARTIST:"   << track->artist()      << "\n";
        result << "  " << "GENRE:"    << track->genre()       << "\n";
        result << "  " << "YEAR:"     << track->date()        << "\n";
    }

    // Result *************************




    writeFile(result, resultFile);
    writeFile(expected, expectedFile);

    if (result.join("") != expected.join(""))
    {
        QString msg = "The result is different from the expected. Use the following command for details:";
        QString cmd = QString("diff %1 %2").arg(expectedFile, resultFile);
        QFAIL((msg + "\n    " + cmd).toLocal8Bit());
    }

    disk->deleteLater();
}


/************************************************

 ************************************************/
void TestFlacon::testTrackSetCodepages_data()
{
    QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("sampleFile");
    QTest::addColumn<QString>("codepageBefore");
    QTest::addColumn<QString>("codepageAfter");


    QTest::newRow("TrackSet_UTF-8")
            << "ru_utf8.cue"    << "ru.result"  << "Big5"   << "UTF-8";

    QTest::newRow("TrackSet_UTF-8_BOM")
            << "ru_utf8_BOM.cue"<< "ru.result"  << "Big5"   << "";

    QTest::newRow("TrackSet_CP1251")
            << "ru_cp1251.cue"  << "ru.result"  << "UTF-8"  << "Windows-1251";
}


/************************************************

 ************************************************/
void TestFlacon::testOutFormatEncoderArgs()
{
    QFETCH(QString, formatId);
    QFETCH(SettingsValues, config);
    QFETCH(QString, expected);

    applySettings(config);

    foreach (OutFormat *format, OutFormat::allFormats())
    {
        if (format->id() != formatId)
            continue;

        Disk *disk = standardDisk();
        QStringList args = format->encoderArgs(disk->track(0), "OutFile.wav");

        QString result = args.join(" ");
        if (result != expected)
        {
            QString msg = QString("Compared values are not the same\n   Format   %1\n   Actual:   %2\n   Expected: %3").arg(
                        formatId,
                        result,
                        expected);
            QFAIL(msg.toLocal8Bit());
        }
        return;
    }

    FAIL(QString("Unknown format \"%1\"").arg(formatId).toLocal8Bit());
}


/************************************************

 ************************************************/
void TestFlacon::testOutFormatEncoderArgs_data()
{
    QTest::addColumn<QString>("formatId");
    QTest::addColumn<SettingsValues>("config");
    QTest::addColumn<QString>("expected");

    SettingsValues cfg;

    //*******************************************
    // FLAC
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/flac",     "/opt/flac");
    cfg.insert("Flac/Compression",  5);
    cfg.insert("Flac/ReplayGain",   "Disable");

    QTest::newRow("Flac_1")
            << "FLAC"
            << cfg
            << "/opt/flac --force --silent "
               "--compression-level-5 "
               "--tag artist=Artist --tag album=Album --tag genre=Genre --tag date=2013 --tag title=Song01 --tag comment=ExactAudioCopy v0.99pb4 "
               "--tag discId=123456789 --tag TRACKNUMBER=1 --tag TOTALTRACKS=4 --tag TRACKTOTAL=4 - -o OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Programs/flac",     "/opt/flac");
    cfg.insert("Flac/Compression",  1);
    cfg.insert("Flac/ReplayGain",   "Disable");

    QTest::newRow("Flac_2")
            << "FLAC"
            << cfg
            << "/opt/flac --force --silent "
               "--compression-level-1 "
               "--tag artist=Artist --tag album=Album --tag genre=Genre --tag date=2013 --tag title=Song01 --tag comment=ExactAudioCopy v0.99pb4 "
               "--tag discId=123456789 --tag TRACKNUMBER=1 --tag TOTALTRACKS=4 --tag TRACKTOTAL=4 - -o OutFile.wav";


    //*******************************************
    // AAC
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/faac",  "/opt/faac");
    cfg.insert("Aac/UseQuality", true);
    cfg.insert("Aac/Quality",    500);

    QTest::newRow("AAC_1")
            << "AAC"
            << cfg
            << "/opt/faac -w "
               "-q 500 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --year 2013 --comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";


    //*******************************************
    cfg.clear();
    cfg.insert("Programs/faac",  "/opt/faac");
    cfg.insert("Aac/UseQuality", true);
    cfg.insert("Aac/Quality",    10);

    QTest::newRow("AAC_2")
            << "AAC"
            << cfg
            << "/opt/faac -w "
               "-q 10 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --year 2013 --comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";

    //*******************************************
    cfg.clear();
    cfg.insert("Programs/faac",  "/opt/faac");
    cfg.insert("Aac/UseQuality", false);
    cfg.insert("Aac/Quality",    500);
    cfg.insert("Aac/Bitrate",    64);

    QTest::newRow("AAC_3")
            << "AAC"
            << cfg
            << "/opt/faac -w "
               "-b 64 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --year 2013 --comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";


    //*******************************************
    // MP3
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/lame",  "/opt/lame");
    cfg.insert("Mp3/Preset",     "vbrMedium");
    cfg.insert("Aac/Quality",    500);

    QTest::newRow("MP3_vbrMedium")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset medium "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";


    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrStandard");

    QTest::newRow("MP3_vbrStandard")
            << "MP3"
            << cfg
            << "/opt/lame "
               "--silent "
               "--preset standard "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";



    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrStandardFast");

    QTest::newRow("MP3_vbrStandardFast")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset fast standard "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";


    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrExtreme");

    QTest::newRow("MP3_vbrExtreme")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset extreme "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrExtremeFast");

    QTest::newRow("MP3_vbrExtremeFast")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset fast extreme "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";


    //*******************************************
    cfg.insert("Mp3/Preset",     "cbrInsane");

    QTest::newRow("MP3_cbrInsane")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset insane "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "cbrKbps");
    cfg.insert("Mp3/Bitrate",    64);

    QTest::newRow("MP3_cbrKbps64")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset cbr 64 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "cbrKbps");
    cfg.insert("Mp3/Bitrate",    128);

    QTest::newRow("MP3_cbrKbps128")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset cbr 128 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";


    //*******************************************
    cfg.insert("Mp3/Preset",     "abrKbps");
    cfg.insert("Mp3/Bitrate",    64);

    QTest::newRow("MP3_abrKbps64")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset 64 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "abrKbps");
    cfg.insert("Mp3/Bitrate",    128);

    QTest::newRow("MP3_abrKbps128")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "--preset 128 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";


    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrQuality");
    cfg.insert("Mp3/Quality",    0);

    QTest::newRow("MP3_vbrQuality0")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "-V 9 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrQuality");
    cfg.insert("Mp3/Quality",    4);

    QTest::newRow("MP3_vbrQuality4")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "-V 5 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";

    //*******************************************
    cfg.insert("Mp3/Preset",     "vbrQuality");
    cfg.insert("Mp3/Quality",    9);

    QTest::newRow("MP3_vbrQuality9")
            << "MP3"
            << cfg
            << "/opt/lame --silent "
               "-V 0 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tc ExactAudioCopy v0.99pb4 --tn 1/4 - OutFile.wav";



    //*******************************************
    // Ogg
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/oggenc",   "/opt/oggenc");
    cfg.insert("Ogg/UseQuality",    true);
    cfg.insert("Ogg/Quality",       5);
    cfg.insert("Ogg/MinBitrate",    "");
    cfg.insert("Ogg/NormBitrate",   "");
    cfg.insert("Ogg/MaxBitrate",    "");

    QTest::newRow("Ogg Quality 5")
            << "OGG"
            << cfg
            << "/opt/oggenc --quiet "
               "-q 5 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment COMMENT=ExactAudioCopy v0.99pb4 "
               "--comment DISCID=123456789 --tracknum 1 --comment TOTALTRACKS=4 --comment TRACKTOTAL=4 -o OutFile.wav -";


    //*******************************************
    cfg.insert("Ogg/Quality",       10);

    QTest::newRow("Ogg Quality 10")
            << "OGG"
            << cfg
            << "/opt/oggenc --quiet "
               "-q 10 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment COMMENT=ExactAudioCopy v0.99pb4 "
               "--comment DISCID=123456789 --tracknum 1 --comment TOTALTRACKS=4 --comment TRACKTOTAL=4 -o OutFile.wav -";


    //*******************************************
    cfg.insert("Programs/oggenc",   "/opt/oggenc");
    cfg.insert("Ogg/UseQuality",    false);
    cfg.insert("Ogg/MinBitrate",    "");
    cfg.insert("Ogg/NormBitrate",   "");
    cfg.insert("Ogg/MaxBitrate",    "");

    QTest::newRow("Ogg Bitrate 0 0 0")
            << "OGG"
            << cfg
            << "/opt/oggenc --quiet "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment COMMENT=ExactAudioCopy v0.99pb4 "
               "--comment DISCID=123456789 --tracknum 1 --comment TOTALTRACKS=4 --comment TRACKTOTAL=4 -o OutFile.wav -";

    //*******************************************
    cfg.insert("Programs/oggenc",   "/opt/oggenc");
    cfg.insert("Ogg/UseQuality",    false);
    cfg.insert("Ogg/MinBitrate",    64);
    cfg.insert("Ogg/NormBitrate",   128);
    cfg.insert("Ogg/MaxBitrate",    350);

    QTest::newRow("Ogg Bitrate 64 128 350")
            << "OGG"
            << cfg
            << "/opt/oggenc --quiet "
               "-b 128 -m 64 -M 350 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment COMMENT=ExactAudioCopy v0.99pb4 "
               "--comment DISCID=123456789 --tracknum 1 --comment TOTALTRACKS=4 --comment TRACKTOTAL=4 -o OutFile.wav -";



    //*******************************************
    // WavPack
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/wavpack",  "/opt/wavpack");
    cfg.insert("WV/Compression",   0);

    QTest::newRow("WavPack Quality 0")
            << "WV"
            << cfg
            << "/opt/wavpack -q "
               "-f "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 -w Track=1/4 - -o OutFile.wav";


    //*******************************************
    cfg.insert("WV/Compression",   1);

    QTest::newRow("WavPack Quality 1")
            << "WV"
            << cfg
            << "/opt/wavpack -q "
               "-h "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 -w Track=1/4 - -o OutFile.wav";


    //*******************************************
    cfg.insert("WV/Compression",   2);

    QTest::newRow("WavPack Quality 2")
            << "WV"
            << cfg
            << "/opt/wavpack -q "
               "-hh "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 -w Track=1/4 - -o OutFile.wav";

}


/************************************************

 ************************************************/
void TestFlacon::testOutFormatGainArgs()
{
    QFETCH(QString, formatId);
    QFETCH(SettingsValues, config);
    QFETCH(QString, expected);

    applySettings(config);

    foreach (OutFormat *format, OutFormat::allFormats())
    {
        if (format->id() != formatId)
            continue;

        QStringList args = format->gainArgs(QStringList() << "OutFile_01.wav" << "OutFile_02.wav" << "OutFile_03.wav");

        QString result = args.join(" ");
        if (result != expected)
        {
            QString msg = QString("Compared values are not the same\n   Format   %1\n   Actual:   %2\n   Expected: %3").arg(
                        formatId,
                        result,
                        expected);
            QFAIL(msg.toLocal8Bit());
        }
        return;
    }

    FAIL(QString("Unknown format \"%1\"").arg(formatId).toLocal8Bit());
}


/************************************************

 ************************************************/
void TestFlacon::testOutFormatGainArgs_data()
{
    QTest::addColumn<QString>("formatId");
    QTest::addColumn<SettingsValues>("config");
    QTest::addColumn<QString>("expected");

    SettingsValues cfg;

    //*******************************************
    // FLAC
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/metaflac", "/opt/metaflac");
    cfg.insert("Flac/ReplayGain",   "Track");

    QTest::newRow("Flac_Track")
            << "FLAC"
            << cfg
            << "/opt/metaflac --add-replay-gain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";


    //*******************************************
    cfg.insert("Flac/ReplayGain",   "Album");

    QTest::newRow("Flac_Album")
            << "FLAC"
            << cfg
            << "/opt/metaflac --add-replay-gain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";



    //*******************************************
    // MP3
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/mp3gain",  "/opt/mp3gain");
    cfg.insert("Mp3/ReplayGain",    "Track");

    QTest::newRow("Mp3_Track")
            << "MP3"
            << cfg
            << "/opt/mp3gain -a -c "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";


    //*******************************************
    cfg.insert("Mp3/ReplayGain",    "Album");

    QTest::newRow("Mp3_Track")
            << "MP3"
            << cfg
            << "/opt/mp3gain -a -c "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";


    //*******************************************
    // Ogg
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/vorbisgain",   "/opt/vorbisgain");
    cfg.insert("Ogg/ReplayGain",        "Track");

    QTest::newRow("Ogg Track")
            << "OGG"
            << cfg
            << "/opt/vorbisgain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    cfg.insert("Ogg/ReplayGain",        "Album");

    QTest::newRow("Ogg Album")
            << "OGG"
            << cfg
            << "/opt/vorbisgain --album "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";



    //*******************************************
    // WavPack
    //*******************************************
    cfg.clear();
    cfg.insert("Programs/wvgain",   "/opt/wvgain");
    cfg.insert("Ogg/ReplayGain",    "Track");

    QTest::newRow("WavPack Track")
            << "WV"
            << cfg
            << "/opt/wvgain "
               "-a "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    cfg.insert("Ogg/ReplayGain",    "Album");

    QTest::newRow("WavPack Album")
            << "WV"
            << cfg
            << "/opt/wvgain "
               "-a "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";
}


/************************************************
 *
 ************************************************/
void TestFlacon::testCueIndex()
{
    QString string = QTest::currentDataTag();
    QFETCH(QString, expected);

    bool cdQuality = string.toUpper().startsWith("CD");
    string = string.mid(2).trimmed();


    QString id1Str = string.section("-", 0, 0).trimmed();
    CueIndex idx1(id1Str);

    QString id2Str = string.section("-", 1, 1).trimmed();
    if (!id2Str.isEmpty())
    {
        CueIndex idx2(id2Str);
        idx1 = idx1 - idx2;
    }

    QString result = idx1.toString(cdQuality);
    QCOMPARE(result, expected);
}


/************************************************
 *
 ************************************************/
void TestFlacon::testCueIndex_data()
{
    QTest::addColumn<QString>("expected");


    QTest::newRow("CD")             << "00:00:00";
    QTest::newRow("HI")             << "00:00.000";

    QTest::newRow("CD 00:00:00")    << "00:00:00";
    QTest::newRow("HI 00:00:00")    << "00:00.000";

    QTest::newRow("CD 00:00:000")   << "00:00:00";
    QTest::newRow("HI 00:00:000")   << "00:00.000";
    QTest::newRow("HI 00:00.000")   << "00:00.000";

    QTest::newRow("CD 1:02:3")      << "01:02:30";
    QTest::newRow("HI 1:02:3")      << "01:02.300";
    QTest::newRow("HI 1:02.3")      << "01:02.300";

    QTest::newRow("CD 1:02:03")     << "01:02:03";
    QTest::newRow("HI 1:02:03")     << "01:02.030";
    QTest::newRow("HI 1:02.03")     << "01:02.030";

    QTest::newRow("HI 1:02:030")    << "01:02.030";
    QTest::newRow("HI 1:02.030")    << "01:02.030";

    QTest::newRow("CD 1:02:74")     << "01:02:74";
    QTest::newRow("HI 1:02:999")    << "01:02.999";
    QTest::newRow("HI 1:02.999")    << "01:02.999";

    QTest::newRow("CD 40:50:74 - 10:20:30")    << "30:30:44";
    QTest::newRow("CD 40:20:24 - 10:30:30")    << "29:49:69";

    QTest::newRow("HI 40:50:740 - 10:20:300")  << "30:30.440";
    QTest::newRow("HI 40:20:240 - 10:30:300")  << "29:49.940";
}

/************************************************

 ************************************************/
void TestFlacon::testFindAudioFile()
{
    QFETCH(QString, fileTag);
    QFETCH(QString, cueFileName);
    QFETCH(QString, audioFiles);
    QFETCH(QString, expected);

    static int dirNum = 0;
    dirNum++;
    QString dir = QString("%1testFindAudioFile/%2/").arg(mTmpDir).arg(dirNum, 4, 20, QChar('0'));
    QDir(dir).mkpath(".");
    clearDir(dir);



    QStringList fileTags = fileTag.split(",", QString::SkipEmptyParts);

    QString cueFile = dir + cueFileName;
    {
        QStringList cue;
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        for (int i=0; i<fileTags.count(); ++i)
        {
            cue << "FILE \"" + fileTags[i].trimmed() +"\" WAVE";
            cue << "  TRACK 01 AUDIO";
            cue << "    TITLE \"Song01\"";
            cue << "    INDEX 01 00:00:00";
        }
        writeTextFile(cueFile, cue);
    }

    foreach (QString f, audioFiles.split(","))
    {
        QFile(mAudio_cd_wav).link(dir + f.trimmed());
    }


    QStringList expectedLists = expected.split(",", QString::SkipEmptyParts);
    CueReader cue(cueFile);
    if (!cue.isValid())
        FAIL("Cue isn't valid:" + cue.errorString().toLocal8Bit());

    for (int i=0; i<cue.diskCount(); ++i)
    {
        Disk disk;
        disk.loadFromCue(cue.disk(i));
        QString expected = expectedLists.at(i).trimmed();
        if (expected == "''")
            expected = "";
        else
            expected = dir + expected;

        QString real = disk.audioFileName();
        QCOMPARE(real, expected);

    }
}


/************************************************

 ************************************************/
void TestFlacon::testFindAudioFile_data()
{
    QTest::addColumn<QString>("fileTag");
    QTest::addColumn<QString>("cueFileName");
    QTest::addColumn<QString>("audioFiles");
    QTest::addColumn<QString>("expected");

    QTest::newRow("1")
            << "Album.wav"
            << "Album.cue"
            << "Album.ape"
            << "Album.ape";


    QTest::newRow("2")
            << "Album.wav"
            << "Garbage.cue"
            << "Album.ape, Garbage.ape"
            << "Album.ape";


    QTest::newRow("3")
            << "Garbage.wav"
            << "Disk.cue"
            << "Disk.ape"
            << "Disk.ape";

    QTest::newRow("Multi disk => CueFile_1.ape")
            << "FileTag1.wav, FileTag2.wav"
            << "CueFile.cue"
            << "CueFile.ape," "CueFile_1.ape," "CueFile_2.ape"
            << "CueFile_1.ape," "CueFile_2.ape";

    QTest::newRow("4")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album_Side1.ape, Album_Side2.ape"
            << "Album_Side1.ape, Album_Side2.ape";

    QTest::newRow("5")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album Side1.ape, Album Side2.ape"
            << "Album Side1.ape, Album Side2.ape";

    QTest::newRow("6")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album_Disk1.ape, Album_Disk2.ape"
            << "Album_Disk1.ape, Album_Disk2.ape";

    QTest::newRow("Multi disk => CUE+Disk1")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album Disk1.ape, Album Disk2.ape"
            << "Album Disk1.ape, Album Disk2.ape";

    QTest::newRow("Multi disk => CUE+[Disk 1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk 1].ape, Album [Disk 2].ape"
            << "Album [Disk 1].ape, Album [Disk 2].ape";

    QTest::newRow("Multi disk => CUE+[Disk #1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk #1].ape, Album [Disk #2].ape"
            << "Album [Disk #1].ape, Album [Disk #2].ape";

    QTest::newRow("Multi disk => CUE+[Disk №1]")
            << "Garbage1.wav, Garbage2.wav"
            << "Album.cue"
            << "Album.ape, Album [Disk №1].ape, Album [Disk №2].ape"
            << "Album [Disk №1].ape, Album [Disk №2].ape";


    QTest::newRow("Multi disk => CUE+Disk 001")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Album Disk 001.ape,"
               "Album Disk 002.ape"
            << "Album Disk 001.ape,"
               "Album Disk 002.ape";

    QTest::newRow("")
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

    QTest::newRow("Not confuse 1 and 11")
            << "FileTag1.wav,"  "FileTag2.wav"
            << "CueFile.cue"
            << "CueFile_11.flac,"  "CueFile_2.flac"
            << "''," "CueFile_2.flac";

    QTest::newRow("Multi disk => Side 1")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Side 1.flac,"
               "Side 2.flac"
            << "Side 1.flac,"
               "Side 2.flac";

    QTest::newRow("Multi disk => Disk 0001")
            << "Garbage1.wav,"
               "Garbage2.wav"
            << "Album.cue"
            << "Album.ape,"
               "Disk 0001.ape,"
               "Disk 0002.ape"
            << "Disk 0001.ape,"
               "Disk 0002.ape";

    QTest::newRow("Multi dot")
            << "FileTag.wav"
            << "CueFile.cue"
            << "CueFile.ape.flac"
            << "CueFile.ape.flac";

    QTest::newRow("Multi dot")
            << "FileTag.wav"
            << "CueFile.cue"
            << "CueFile_AditionalText.flac"
            << "CueFile_AditionalText.flac";

    // Issue #31
    QTest::newRow("Bush - (1996) Razorblade Suitcase")
            << "Bush - (1996) Razorblade Suitcase.flac"
            << "Bush - (1996) Razorblade Suitcase.cue"
            << "3 Doors Down - (2000) The Better Life.flac,"
               "Bush - (1996) Razorblade Suitcase.flac"
            << "Bush - (1996) Razorblade Suitcase.flac";

    QTest::newRow("FileTag(1) and FileTag(2)")
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

struct TestCueFile_ {
    QString name;
    QString fileTag;
    QString fileTag2;
};
Q_DECLARE_METATYPE(TestCueFile_)


struct TestFindCueFileData {
    QList<TestCueFile_> cueFiles;
    QStringList audioFiles;
    QString chekAudioFile;
    QString expected;
};
Q_DECLARE_METATYPE(TestFindCueFileData)


/************************************************

 ************************************************/
void TestFlacon::testFindCueFile()
{
    QFETCH(TestFindCueFileData, test);

    static int dirNum = 0;
    dirNum++;
    QString dir = QString("%1testFindAudioFile/%2/").arg(mTmpDir).arg(dirNum, 4, 20, QChar('0'));
    QDir(dir).mkpath(".");
    clearDir(dir);


    foreach (QString f, test.audioFiles)
    {
        QFile(mAudio_cd_wav).link(dir + f.trimmed());
    }

    foreach (TestCueFile_ cueFile, test.cueFiles)
    {
        QStringList cue;
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        cue << "FILE \"" + cueFile.fileTag.trimmed() +"\" WAVE";
        cue << "  TRACK 01 AUDIO";
        cue << "    TITLE \"Song01\"";
        cue << "    INDEX 01 00:00:00";

        if (!cueFile.fileTag2.isEmpty())
        {
            cue << "FILE \"" + cueFile.fileTag2.trimmed() +"\" WAVE";
            cue << "  TRACK 01 AUDIO";
            cue << "    TITLE \"Song01\"";
            cue << "    INDEX 01 00:00:00";
        }
        writeTextFile(dir + cueFile.name, cue);
    }

    InputAudioFile audio(dir + test.chekAudioFile);

    Disk disk;
    disk.setAudioFile(audio);
    QString real = disk.cueFile();
    QString expected = dir + test.expected;
    QCOMPARE(real, expected);
}


/************************************************

 ************************************************/
void TestFlacon::testFindCueFile_data()
{

    QTest::addColumn<TestFindCueFileData>("test");
    TestFindCueFileData test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "1.cue";
    test.cueFiles.last().fileTag= "FILE \"1.wav\" WAVE";

    test.audioFiles << "1.wav";

    test.chekAudioFile = "1.wav";
    test.expected = "1.cue";
    QTest::newRow("1.cue 1.wav") << test;

    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "1.cue";
    test.cueFiles.last().fileTag= "FILE \"1.wav\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "2.cue";
    test.cueFiles.last().fileTag= "FILE \"2.wav\" WAVE";

    test.audioFiles << "1.wav";
    test.audioFiles << "2.wav";

    test.chekAudioFile = "1.wav";
    test.expected = "1.cue";
    QTest::newRow("1.cue 2.cue *1.wav* 2.wav") << test;


    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "1.cue";
    test.cueFiles.last().fileTag= "FILE \"1.wav\" WAVE";

    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "2.cue";
    test.cueFiles.last().fileTag= "FILE \"2.wav\" WAVE";

    test.audioFiles << "1.wav";
    test.audioFiles << "2.wav";

    test.chekAudioFile = "2.wav";
    test.expected = "2.cue";
    QTest::newRow("1.cue 2.cue 1.wav *2.wav*") << test;


    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "multi.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";
    test.cueFiles.last().fileTag2= "FILE \"2.wav\" WAVE";

    test.audioFiles << "multi_1.wav";
    test.audioFiles << "multi_2.wav";

    test.chekAudioFile = "multi_1.wav";
    test.expected = "multi.cue";
    QTest::newRow("multi.cue multi_1.wav multi_2.wav") << test;


    // -------------------------------------
    test = TestFindCueFileData();
    test.cueFiles.append(TestCueFile_());
    test.cueFiles.last().name = "multi.cue";
    test.cueFiles.last().fileTag = "FILE \"1.wav\" WAVE";
    test.cueFiles.last().fileTag2= "FILE \"2.wav\" WAVE";

    test.audioFiles << "multi_1.wav";
    test.audioFiles << "multi_2.wav";

    test.chekAudioFile = "multi_2.wav";
    test.expected = "multi.cue";
    QTest::newRow("multi.cue multi_1.wav multi_2.wav") << test;
}

QTEST_MAIN(TestFlacon)
