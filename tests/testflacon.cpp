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


#include <QTest>
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


QTEST_GUILESS_MAIN(TestFlacon)
