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
#include "../settings.h"
#include "outformat.h"
#include "../converter/wavheader.h"
#include "tools.h"
#include "project.h"
#include "../inputaudiofile.h"
#include "../converter/converter.h"

#include <QDir>
#include <QDebug>




class ConverterTester
{
public:
    ConverterTester(const QString &cueFile,
                    const QString &audioFile,
                    const QString &expectedCue,
                    const QString &resultFiles);


    void run(const QString &name);

    Disk *disk() { return mDisk; }

private:
    Disk *mDisk;
    QStringList mResultFiles;
    QString mExpectedCue;
    ConverterTester(const ConverterTester &other) {}
};




/************************************************

 ************************************************/
ConverterTester::ConverterTester(const QString &cueFile, const QString &audioFile, const QString &expectedCue, const QString &resultFiles)
{
    mResultFiles = resultFiles.split(';', QString::SkipEmptyParts);
    mExpectedCue = expectedCue;
    project->clear();
    mDisk = loadFromCue(cueFile);
    InputAudioFile audio(audioFile);
    mDisk->setAudioFile(audio);
    project->addDisk(mDisk);
}

/************************************************

 ************************************************/
void ConverterTester::run(const QString &name)
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
    if (!mExpectedCue.isEmpty())
        QFile::copy(mExpectedCue, outDir.absoluteFilePath("expected.cue"));

    QStringList missing;
    foreach(QString f, mResultFiles)
    {
        QString file = f.section(':', 0, 0).trimmed();
        QString hash = f.section(':', 1).trimmed();

        if (files.removeAll(file) == 0)
            missing << file;

        if (!hash.isEmpty())
        {
            compareAudioHash(outDir.absoluteFilePath(file), hash);
        }
    }


    QString msg = "";
    if (!missing.isEmpty())
        msg += QString("\nFiles not exists:\n  *%1").arg(missing.join("\n  *"));

    if (!files.isEmpty())
        msg += QString("\nFiles exists:\n  *%1").arg(files.join("\n  *"));


    if (!mExpectedCue.isEmpty())
    {
        //QString resCueFile;
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
        QFAIL(name.toLocal8Bit() + "\n" + msg.toLocal8Bit());

}


/************************************************
 *
 ************************************************/
void TestFlacon::testConvert()
{

    QFETCH(QString, cueFile);
    QFETCH(QString, audioFile);
    QFETCH(QString, expectedCue);
    QFETCH(QString, tags);
    QFETCH(QString, resultFiles);
    QFETCH(bool, createCue);
    QFETCH(int, preGapType);

    settings->setValue(Settings::OutFiles_Directory, dir());
    settings->setValue(Settings::OutFiles_Pattern, "%a/%n - %t");
    settings->setValue(Settings::OutFiles_Format,  "WAV");
    settings->setValue(Settings::PerTrackCue_FlaconTags, false);
    settings->setValue(Settings::PerTrackCue_Create,  createCue);
    settings->setValue(Settings::PerTrackCue_Pregap,  OutFormat::preGapTypeToString(OutFormat::PreGapType(preGapType)));
    settings->setValue(Settings::OutFiles_Directory, dir());
    settings->setValue(Settings::Encoder_TmpDir,      "");

    QString srcAudioFile  = audioFile.section(':', 0, 0);
    QString destAudioFile = audioFile.section(':', 1);
    if (!destAudioFile.isEmpty())
    {
        destAudioFile = dir() + "/" + destAudioFile;
        QFileInfo(destAudioFile).dir().mkpath(".");
        QFile::copy(srcAudioFile, destAudioFile);
    }
    else
    {
        destAudioFile = srcAudioFile;
    }

    ConverterTester conv(
                    cueFile,
                    destAudioFile,
                    expectedCue,
                    resultFiles
                    );

    foreach (QString line, tags.split(';'))
    {
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        int n = line.section(' ', 0, 0).toInt() - 1;
        Track *track = conv.disk()->track(n);

        foreach (QString s, line.section(' ', 1).split(','))
        {
            track->setTag(s.section(':', 0, 0).trimmed(), s.section(':', 1).trimmed());
        }
    }

    conv.run(QTest::currentDataTag());

}


/************************************************
 *
 ************************************************/
void TestFlacon::testConvert_data()
{
    QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("audioFile");
    QTest::addColumn<QString>("expectedCue");
    QTest::addColumn<QString>("resultFiles");
    QTest::addColumn<QString>("tags");
    QTest::addColumn<bool>("createCue");
    QTest::addColumn<int>("preGapType");

    QString inDir = mDataDir + "/testConvert/";



    QTest::newRow("01.1 With pregap and HTOA, w.o cue")
            << inDir + "01.cuecreator.cue"
            << mAudio_24x96_wav
            << ""
            << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
            << ""
            << false
            << int(OutFormat::PreGapExtractToFile);



    QTest::newRow("01.2 With pregap and HTOA, with cue")
            << inDir + "01.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "01.expected.cue"
            << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "00 - (HTOA).wav : afb9e4434b212bacdd62e585461af757;"
               "Artist-Album.cue;"
            << ""
            << true
            << int(OutFormat::PreGapExtractToFile);


    QTest::newRow("02.1 W.o pregap, w.o HTOA")
            << inDir + "02.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "02.expected.cue"
            << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "Artist-Album.cue;"
            << ""
            << true
            << int(OutFormat::PreGapAddToFirstTrack);


    QTest::newRow("02.2 W.o pregap, w.o HTOA")
            << inDir + "02.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "02.expected.cue"
            << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "Artist-Album.cue;"
            << ""
            << true
            << int(OutFormat::PreGapExtractToFile);



    QTest::newRow("03.1 With pregap, w.o HTOA")
            << inDir + "03.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "03.expected.cue"
            << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "Artist-Album.cue;"
            << ""
            << true
            << int(OutFormat::PreGapAddToFirstTrack);


    QTest::newRow("04.1 All tags")
            << inDir + "04.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "04.expected.cue"
            << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "Artist-Album.cue;"
            << ""
            << true
            << int(OutFormat::PreGapAddToFirstTrack);


    QTest::newRow("05.1 Cue w.o tags + tags form the separate file")
            << inDir + "05.cuecreator.cue"
            << mAudio_24x96_wav
            << inDir + "05.expected.cue"
            << "01 - Song Title 01.wav : 07bb5c5fbf7b9429c05e9b650d9df467;"
               "02 - Song Title 02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song Title 03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song Title 04.wav : 2e5df99b43b96c208ab26983140dd19f;"
               "Artist-Album.cue;"
            << "01 PERFORMER: Artist, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 01;"
               "02 PERFORMER: Artist, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 02;"
               "03 PERFORMER: Artist, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 03;"
               "04 PERFORMER: Artist, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 04;"
            << true
            << int(OutFormat::PreGapAddToFirstTrack);


    QTest::newRow("06.1 Garbage in the CUE")
            << inDir + "06.garbageInTags.cue"
            << mAudio_24x96_wav
            << ""
            << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
            << ""
            << false
            << int(OutFormat::PreGapExtractToFile);


    QTest::newRow("07.1 with symbols and space")
            << inDir + "07.path(with.symbols and space )/07.path(with.symbols and space ).cue"
            << mAudio_24x96_wav +
               ":Test_07.path(with.symbols and space )/Audio file (with.symbols and space ).wav"
            << ""
            << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
            << ""
            << false
            << int(OutFormat::PreGapExtractToFile);


    QTest::newRow("08.1 Test_8/Музыка")
            << inDir + "01.cuecreator.cue"
            << mAudio_24x96_wav
            << ""
            << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d;"
               "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140;"
               "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3;"
               "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f;"
            << ""
            << false
            << int(OutFormat::PreGapExtractToFile);

}
