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
 *
 ************************************************/
Disk *loadFromCue(const QString &cueFile)
{
    CueReader cueReader(cueFile);
    cueReader.load();
    Disk *res = new Disk();
    res->loadFromCue(cueReader.disk(0));
    return res;
}


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
void TestFlacon::initTestCase()
{
    QSettings::setPath(QSettings::IniFormat,    QSettings::UserScope, QString::fromLocal8Bit(TEST_OUT_DIR));
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QString::fromLocal8Bit(TEST_OUT_DIR));

    QString shntool = settings->findProgram("shntool");
    if (shntool.isEmpty())
        FAIL(QString("Program \"%1\" not found.").arg("shntool").toLocal8Bit());

    settings->setValue(Settings::Prog_Shntool, shntool);
    settings->sync();

    mFfmpeg = settings->findProgram("avconv");
    if (mFfmpeg.isEmpty())
        mFfmpeg = settings->findProgram("ffmpeg");

    if (mFfmpeg.isEmpty())
        FAIL(QString("Program \"%1\" not found.").arg("avconv/ffmpeg").toLocal8Bit());

    mCdAudioFile = mTmpDir + "CD_10Min.wav";
    createAudioFile(mFfmpeg, mCdAudioFile, 600, true);

    mHdAudioFile = mTmpDir + "HD_10Min.wav";
    createAudioFile(mFfmpeg, mHdAudioFile, 600, false);
}


/************************************************

 ************************************************/
bool TestFlacon::createAudioFile(const QString &program, const QString &fileName, int duration, bool cdQuality)
{
    if (QFileInfo(fileName).exists())
        return true;

    QStringList args;
    args << "-y"; //  Overwrite output files."
    args << "-ar" << (cdQuality ? "44100" : " 48000");
    args << "-f" << "s16le";
    args << "-acodec" << "pcm_s16le";
    args << "-ac" << "2";
    args << "-i" << "/dev/urandom";
    args << "-t" << QString("%1").arg(duration);
    args << fileName;

    QProcess proc;
    //proc.setProcessChannelMode(QProcess::ForwardedChannels);

    proc.start(program, args);
    proc.waitForStarted();
    if (proc.state() == QProcess::NotRunning)
    {
        return false;
    }

    proc.waitForFinished(-1);
    return proc.exitCode() == 0;
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
        QString cueFile = mTmpDir + "testTrackResultFileName.cue";

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
ConverterTester::ConverterTester(const QString &cueFile, const QString &audioFile, const QString &expectedCue, const QString &resultFiles)
{
    mResultFiles = resultFiles.split(';', QString::SkipEmptyParts);
    mExpectedCue = expectedCue;
    project->clear();
    mDisk = loadFromCue(cueFile);
    mDisk->setAudioFile(audioFile);
    project->addDisk(mDisk);
}

/************************************************

 ************************************************/
void ConverterTester::run()
{
    Converter conv;
    QEventLoop loop;
    loop.connect(&conv, SIGNAL(finished()), &loop, SLOT(quit()));
    conv.start();
    if (!conv.isRunning())
    {
        QString msg;
        mDisk->canConvert(&msg);
        QFAIL(QString("Can't start converter: \"%1\"").arg(msg).toLocal8Bit());
    }

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

    //## 6 ################################################
    // Garbage in the CUE
    {
        outDir = resultDir + "Test_6";
        settings->setValue(Settings::PerTrackCue_Create,  false);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
        settings->setValue(Settings::OutFiles_Directory, outDir);

        ConverterTester conv(
                    inDir + "06.garbageInTags.cue",
                    mHdAudioFile,
                    "",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    );

        conv.run();
    }
    //## 6 #################################################


    //## 7 #################################################
    // With pregap and HTOA
    {
        QString dir = resultDir + "Test_07.path(with.symbols and space )";
        QDir().mkpath(dir);
        QString audioFile = dir + "CD_10Min.wav";
        createAudioFile(mFfmpeg, audioFile, 600, true);



        outDir = resultDir + "Test_7";
        settings->setValue(Settings::PerTrackCue_Create,  false);
        settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
        settings->setValue(Settings::OutFiles_Directory,  dir);
        settings->setValue(Settings::Encoder_TmpDir,      "");
        ConverterTester conv(
                    inDir + "07.path(with.symbols and space )/07.path(with.symbols and space ).cue",
                    audioFile,
                    "",
                    "01 - Song01.wav;"
                    "02 - Song02.wav;"
                    "03 - Song03.wav;"
                    "04 - Song04.wav;"
                    );

        conv.run();
    }
    //## 7 #################################################


    //## 8 #################################################
    // With pregap and HTOA
    {
        outDir = resultDir + QString::fromUtf8("Test_8/Музыка");
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
    //## 8 #################################################

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
    delete disk;
}


/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName_data()
{
    QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");

    QString cueFile = mTmpDir + "testTrackResultFileName.cue";

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

    Disk *disk = loadFromCue(mDataDir + "simple.cue");

    if (!audioFile.isEmpty())
        disk->setAudioFile(audioFile);

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
    delete disk;
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
            << mTmpDir + "/Artist/2013 - Album/01 - Song01.wav"
            << mCdAudioFile;

    QTest::newRow("6")
            << ""
            << "%a/%y - %A/%n - %t"
            << mTmpDir + "/Artist/2013 - Album/01 - Song01.wav"
            << mCdAudioFile;
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
        QFile(mCdAudioFile).link(dir + f.trimmed());
    }


    QStringList expectedLists = expected.split(",", QString::SkipEmptyParts);
    CueReader cue(cueFile);
    cue.load();
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
}





QTEST_MAIN(TestFlacon)
