/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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
#include "../disk.h"
#include "../settings.h"
#include "../project.h"
#include "../inputaudiofile.h"
#include "converter/converter.h"
#include "outformat.h"

#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>
#include <QProcess>


#define protected public;

#define FAIL(message) \
do {\
    QTest::qFail(message, __FILE__, __LINE__);\
} while (0)

/************************************************

 ************************************************/
TestFlacon::TestFlacon(QObject *parent) :
    QObject(parent),
    mTmpDir(TEST_OUT_DIR),
    mDataDir(TEST_DATA_DIR)
{
    // ffmpeg -ar 48000 -f s16le -acodec pcm_s16le -ac 2 -i /dev/urandom  -t 600 frnd.wav
    // avconv -ar 48000 -f s16le -acodec pcm_s16le -ac 2 -i /dev/urandom  -t 600 arnd.wav
}


/************************************************

 ************************************************/
void TestFlacon::initTestCase()
{

    QSettings::setPath(QSettings::IniFormat,    QSettings::UserScope, QString::fromLocal8Bit(TEST_OUT_DIR));
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QString::fromLocal8Bit(TEST_OUT_DIR));

    settings->setValue(Settings::Prog_Shntool, "/usr/bin/shntool");


    settings->sync();

    mCdAudioFile = mTmpDir + "CD_10Min.wav";
    createAudioFile(mCdAudioFile, 600, true);

    mHdAudioFile = mTmpDir + "HD_10Min.wav";
    createAudioFile(mHdAudioFile, 600, true);
}


/************************************************

 ************************************************/
void TestFlacon::createAudioFile(const QString &fileName, int duration, bool cdQuality)
{
    if (QFileInfo(fileName).exists())
        return;

    QStringList args;
    args << "-y"; //  Overwrite output files."
    args << "-ar" << "48000"; //44100
    //args << "-ar" << "44100";
    args << "-f" << "s16le";
    args << "-acodec" << "pcm_s16le";
    args << "-ac" << "2";
    args << "-i" << "/dev/urandom";
    args << "-t" << QString("%1").arg(duration);
    args << fileName;

    QProcess proc;
    //proc.setProcessChannelMode(QProcess::ForwardedChannels);

    proc.start("avconv", args);
    proc.waitForStarted();

    if (proc.state() == QProcess::NotRunning)
    {
        proc.start("ffmpeg", args);
        proc.waitForStarted();
    }

    proc.waitForFinished(-1);
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
bool TestFlacon::compareCue(const QString &result, const QString &expected, QString *error)
{
    QFile resFile(result);
    resFile.open(QFile::ReadOnly);
    QByteArray resData = resFile.readAll();
    resFile.close();

    QFile expFile(expected);
    expFile.open(QFile::ReadOnly);
    QByteArray expData = expFile.readAll();
    expFile.close();

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
void TestFlacon::testSafeString()
{
    QCOMPARE(Disk::safeString("A|B/C|D\\E:F*G?H"), QString("A-B-C-D-E-F-G-H"));
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName_data()
{
    QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");

    QString cueFile = TEST_OUT_DIR "testTrackResultFileName.cue";

    {
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
    }


    QTest::newRow("1")
            << cueFile
            << "%a/%y - %A/%n - %t"
            << "Artist/2013 - Album/01 - Song01.wav";

    QTest::newRow("2")
            << cueFile
            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("3")
            << cueFile
            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("4")
            << cueFile
            << "/%%/%Q/%N/%n/%A/%a/%t/%y/%g/%%"
            << "/%/%Q/04/01/Album/Artist/Song01/2013/Genre/%.wav";


    QTest::newRow("5")
            << cueFile
            << "%%Q/%%N/%%n/%%A/%%a/%%t/%%y/%%g"
            << "%Q/%N/%n/%A/%a/%t/%y/%g.wav";

    QTest::newRow("6")
            << cueFile
            << "%%%Q/%%%N/%%%n/%%%A/%%%a/%%%t/%%%y/%%%g/%%%"
            << "%%Q/%04/%01/%Album/%Artist/%Song01/%2013/%Genre/%%.wav";

    cueFile = TEST_OUT_DIR "testTrackResultFileName2.cue";
    {
        QStringList cue;
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
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
    }


    QTest::newRow("1")
            << cueFile
            << "{}/{Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "{}/{Text}/01/01 Text//.wav";


    QTest::newRow("2")
            << cueFile
            << "Test{Text/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Test{Text/01/01 Text//.wav";


    QTest::newRow("3")
            << cueFile
            << "Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Text}/01/01 Text//.wav";

}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName()
{
    QFETCH(QString, cueFile);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    settings->setValue(Settings::OutFiles_Pattern, pattern);
    settings->setValue(Settings::OutFiles_Format, "WAV");

    Disk disk;
    disk.loadFromCue(cueFile);

    QString result = disk.track(0)->resultFileName();
    //QCOMPARE(result, expected);

    if (result != expected)
    {
        QString msg = QString("Compared values are not the same\n   Pattern   %1\n   Actual:   %2\n   Expected: %3").arg(
                    pattern,
                    result,
                    expected);
        QFAIL(msg.toLocal8Bit());
    }

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


    QTest::newRow("1")
            << "/home/user/music"
            << "%a/%y - %A/%n - %t"
            << "/home/user/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("2")
            << "~/music"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("3")
            << "/music"
            << "%a/%y - %A/%n - %t"
            << "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("4")
            << ""
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/Artist/2013 - Album/01 - Song01.wav"
            << "";


    QTest::newRow("5")
            << "."
            << "%a/%y - %A/%n - %t"
            << TEST_OUT_DIR "/Artist/2013 - Album/01 - Song01.wav"
            << mCdAudioFile;


    QTest::newRow("6")
            << ""
            << "%a/%y - %A/%n - %t"
            << TEST_OUT_DIR "/Artist/2013 - Album/01 - Song01.wav"
            << mCdAudioFile;
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath()
{
    //QFETCH(QString, cueFile);
    QFETCH(QString, outDir);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);
    QFETCH(QString, audioFile);

    settings->setValue(Settings::OutFiles_Directory, outDir);
    settings->setValue(Settings::OutFiles_Pattern, pattern);
    settings->setValue(Settings::OutFiles_Format, "WAV");

    Disk disk;
    disk.loadFromCue(TEST_DATA_DIR "simple.cue");

    if (!audioFile.isEmpty())
        disk.setAudioFile(audioFile);

    QString result = disk.track(0)->resultFilePath();
    if (QFileInfo(result).absoluteFilePath() != QFileInfo(expected).absoluteFilePath())
    {
        QString msg = QString("Compared values are not the same\n   Actual:   %1 [%2]\n   Expected: %3").arg(
                    QFileInfo(result).absoluteFilePath(), result,
                    expected);
        QFAIL(msg.toLocal8Bit());
    }
    //QCOMPARE(result, expected);
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
void TestFlacon::testTrackSetCodepages()
{
    static int count = 0;
    count++;

    QFETCH(QString, cueFile);
    QFETCH(QString, sampleFile);
    QFETCH(QString, codepageBefore);
    QFETCH(QString, codepageAfter);

    QString outFilePattern = QString("%1%2.%3.%4")
            .arg(TEST_OUT_DIR)
            .arg(count, 2, 10, QChar('0'))
            .arg(QTest::currentDataTag());

    QString testDataDir = TEST_DATA_DIR;

    QString testCueFile = outFilePattern.arg("cue");
    QString expectedFile = outFilePattern.arg("expected");
    QString resultFile = outFilePattern.arg("result");

    if (QFileInfo(testCueFile).exists())
        QFile(testCueFile).remove();

    if (!QFile::copy(testDataDir + cueFile, testCueFile))
        QFAIL(QString("Can't copy file %1 to %2").arg(testDataDir + cueFile, testCueFile).toLocal8Bit().data());

    Disk disk;

    if (!codepageBefore.isEmpty())
        settings->setValue(Settings::Tags_DefaultCodepage, codepageBefore);
    else
        settings->setValue(Settings::Tags_DefaultCodepage, "UTF-8");

    disk.loadFromCue(testCueFile);

    if (!codepageAfter.isEmpty())
        disk.setTextCodecName(codepageAfter);

    QStringList expected = this->readFile(TEST_DATA_DIR + sampleFile);

    QStringList result;
    // Result *************************
    //result << "GENRE:" << tracks.genre() << "\n";
    //resultSl << "ALBUM:" << tracks.album() << "\n";
    result << "DISCID:" << disk.discId() << "\n";

    for(int i=0; i<disk.count(); ++i)
    {
        Track *track = disk.track(i);
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
}


/************************************************

 ************************************************/
void TestFlacon::testConvert()
{
    QString resultDir = mTmpDir + QString("/converted/");
    clearDir(resultDir);

    settings->setValue(Settings::OutFiles_Directory, resultDir);
    settings->setValue(Settings::OutFiles_Pattern, "%a/%n - %t");
    settings->setValue(Settings::OutFiles_Format,  "WAV");
    settings->setValue(Settings::PerTrackCue_FlaconTags, false);

    QString inDir = mDataDir + "/testConvert/";
    QString outDir;
    //## 1.1 ##############################################
    // With pregap and HTOA
    {
        outDir = resultDir + "Test_1.1";
        settings->setValue(Settings::PerTrackCue_Create,  false);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "01.cuecreator.cue",
                    mHdAudioFile,
                    "",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    );

        conv.run();
    }
    //## 1.1 ##############################################

    //## 1.2 ##############################################
    // With pregap and HTOA
    {
        outDir = resultDir + "Test_1.2";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "01.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "01.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "00 - (HTOA).wav;"
                    "Artist-Album.cue;"
                    );

        conv.run();
    }
    //## 1.2 ##############################################

    //## 2.1 ##############################################
    // W/o pregap, w/o HTOA
    {
        outDir = resultDir + "Test_2.1";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapAddToFirstTrack));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "02.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "02.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "Artist-Album.cue;"
                    );

        conv.run();
    }
    //## 2.1 ##############################################

    //## 2.2 ##############################################
    // W/o pregap, w/o HTOA
    {
        outDir = resultDir + "Test_2.2";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
        settings->setValue(Settings::OutFiles_Directory, outDir);


        ConverterTester conv(
                    inDir + "02.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "02.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "Artist-Album.cue;"
                    );

        conv.run();
    }
    //## 2.2 ##############################################

    //## 3 ################################################
    // With pregap, w/o HTOA
    {
        outDir = resultDir + "Test_3";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapAddToFirstTrack));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "03.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "03.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "Artist-Album.cue;"
                    );

        conv.run();
    }
    //## 3 ################################################

    //## 4 ################################################
    // All tags
    {
        outDir = resultDir + "Test_4";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapAddToFirstTrack));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "04.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "04.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "Artist-Album.cue;"
                    );

        conv.run();
    }
    //## 4 ################################################

    //## 5 ################################################
    // Cue w/o tags + tags form the separate file
    {
        outDir = resultDir + "Test_5";
        settings->setValue(Settings::PerTrackCue_Create,  true);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapAddToFirstTrack));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "05.cuecreator.cue",
                    mHdAudioFile,
                    inDir + "05.expected.cue",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    "Artist-Album.cue;"
                    );

        for (int i=0; i<conv.disk()->count(); ++i)
        {
            Track *track = conv.disk()->track(i);
            track->setArtist("Artist");
            track->setAlbum("Album");
            track->setTitle(QString("Song%1").arg(i+1, 2, 10, QChar('0')));
            track->setGenre("Genre");
            track->setDate("2013");
        }

        conv.run();
    }
    //## 5 ################################################

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
ConverterTester::ConverterTester(const QString &cueFile, const QString &audioFile, const QString &expectedCue, const QString &resultFiles)
{
    mResultFiles = resultFiles.split(';', QString::SkipEmptyParts);
    mExpectedCue = expectedCue;
    project->clear();
    mDisk = new Disk();
    project->addDisk(mDisk);

    mDisk->loadFromCue(cueFile);
    mDisk->setAudioFile(audioFile);

}


/************************************************

 ************************************************/
void ConverterTester::run()
{
    Converter conv;
    QEventLoop loop;
    loop.connect(&conv, SIGNAL(finished()), &loop, SLOT(quit()));
    conv.start();
    loop.exec();

    QDir outDir = QFileInfo(mDisk->track(0)->resultFilePath()).dir();
    QStringList files = outDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files);
    project->clear();

    QStringList missing;
    foreach(QString f, mResultFiles)
    {
        if (files.removeAll(f) == 0)
            missing << f;
    }


    QString msg = "";
    if (!missing.isEmpty())
        msg += QString("\nFiles not exists:\n  *%1").arg(missing.join("\n  *"));

    if (!files.isEmpty())
        msg += QString("\nFiles exists:\n  *%1").arg(files.join("\n  *"));


    if (!mExpectedCue.isEmpty())
    {
        QString resCueFile;
        files = outDir.entryList(QStringList() << "*.cue", QDir::Files);
        if (files.isEmpty())
        {
            msg += "\nResult CUE file not found.";
        }
        else
        {
            QString err;
            if (!TestFlacon::compareCue(outDir.absoluteFilePath(files.first()), mExpectedCue, &err))
                msg += "\n" + err;
        }
    }

    if (!msg.isEmpty())
        QFAIL(msg.toLocal8Bit());

}





QTEST_MAIN(TestFlacon)
