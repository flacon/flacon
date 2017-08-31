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
#include "types.h"
#include "../settings.h"
#include "outformat.h"
#include "../converter/wavheader.h"
#include "tools.h"
#include "project.h"
#include "../inputaudiofile.h"
#include "../converter/converter.h"

#include <QDir>
#include <QDebug>



struct TestConvertRequest {
    QString cueFile;
    QString audioFile;
    QString expectedCue;
    QStringList resultFiles;
    QStringList tags;

    void clear() {
        cueFile.clear();
        audioFile.clear();
        expectedCue.clear();
        resultFiles.clear();
        tags.clear();

    }
};
Q_DECLARE_METATYPE(TestConvertRequest)
Q_DECLARE_METATYPE(QList<TestConvertRequest>)

/************************************************
 *
 ************************************************/
void consoleErroHandler(const QString &message)
{
    QString msg(message);
    msg.remove(QRegExp("<[^>]*>"));
    qWarning() << "Converter error:" << msg;
}

/************************************************
 *
 ************************************************/
void TestFlacon::testConvert()
{
    QFETCH(bool, createCue);
    QFETCH(int, preGapType);
    QFETCH(QString, tmpDir);
    QFETCH(QList<TestConvertRequest>, requests);

    settings->setOutFormat("WAV");
    settings->setCreateCue(createCue);
    settings->setPregapType(PreGapType(preGapType));
    settings->setTmpDir(tmpDir);
    settings->setOutFileDir(dir());
    settings->setOutFilePattern("%a/%n - %t");

    Project::installErrorHandler(consoleErroHandler);
    project->clear();
    foreach (TestConvertRequest req, requests)
    {
        QString srcAudioFile  = req.audioFile.section(':', 0, 0);
        QString destAudioFile = req.audioFile.section(':', 1);

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

        Disk *disk = loadFromCue(req.cueFile);
        InputAudioFile audio(destAudioFile);
        disk->setAudioFile(audio);
        project->addDisk(disk);

        foreach (QString line, req.tags)
        {
            line = line.trimmed();
            if (line.isEmpty())
                continue;

            int n = line.section(' ', 0, 0).toInt() - 1;
            Track *track = disk->track(n);

            foreach (QString s, line.section(' ', 1).split(','))
            {
                track->setTag(s.section(':', 0, 0).trimmed(), s.section(':', 1).trimmed());
            }
        }
    }

    Converter conv;
    conv.setShowStatistic(false);
    QEventLoop loop;
    loop.connect(&conv, SIGNAL(finished()), &loop, SLOT(quit()));
    conv.start();


    if (!conv.isRunning())
    {
        for (int i=0; i<project->count(); i++)
        {
            QString msg;
            if (! project->disk(i)->canConvert(&msg))
            {
                QFAIL(QString("Can't start converter: \"%1\"").arg(msg).toLocal8Bit());
            }
        }
    }

    loop.exec();

    // Checks __________________________________________________
    for (int i=0; i<project->count(); i++)
    {
        TestConvertRequest req = requests.at(i);
        QDir outDir = QFileInfo(project->disk(i)->track(0)->resultFilePath()).dir();
        QStringList files = outDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::Files);

        QFile::copy(req.cueFile, outDir.absoluteFilePath("source_%1.cue").arg(i, 2, 10, QChar('0')));

        if (! req.expectedCue.isEmpty())
            QFile::copy(req.expectedCue, outDir.absoluteFilePath("expected_%1.cue").arg(i, 2, 10, QChar('0')));

        QStringList missing;
        foreach(QString f, req.resultFiles)
        {
            QString file = f.section(':', 0, 0).trimmed();
            QString hash = f.section(':', 1).trimmed();

            if (files.removeAll(file) == 0)
                missing << file;

            if (!hash.isEmpty() && QFileInfo(outDir.absoluteFilePath(file)).exists())
            {
               compareAudioHash(outDir.absoluteFilePath(file), hash);
            }
        }


        QString msg = "";
        if (!missing.isEmpty())
            msg += QString("\nFiles not exists in %1:\n  *%2")
                    .arg(outDir.absolutePath())
                    .arg(missing.join("\n  *"));

        if (!files.isEmpty())
            msg += QString("\nFiles exists in %1:\n  *%2")
                    .arg(outDir.absolutePath())
                    .arg(files.join("\n  *"));

        if (!req.expectedCue.isEmpty())
        {
            files = outDir.entryList(QStringList() << "*.cue", QDir::Files);
            if (files.isEmpty())
            {
                msg += "\nResult CUE file not found.";
            }
            else
            {
                QString err;
                if (!TestFlacon::compareCue(outDir.absoluteFilePath(files.first()), req.expectedCue, &err))
                    msg += "\n" + err;
            }
        }

        if (!msg.isEmpty())
            QFAIL(QTest::currentDataTag() + '\n' + msg.toLocal8Bit());
    }
    project->clear();
}



/************************************************
 *
 ************************************************/
void TestFlacon::testConvert_data()
{
    QTest::addColumn<bool>("createCue");
    QTest::addColumn<int>("preGapType");
    QTest::addColumn<QString>("tmpDir");
    QTest::addColumn<QList<TestConvertRequest> >("requests");

    TestConvertRequest req;
    QList<TestConvertRequest> requests;


    QString inDir = mDataDir + "/testConvert/";
    QString name;

    //=====================================================
    name ="01.1 With pregap and HTOA, without cue";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "01.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "01.2 With pregap and HTOA, with cue";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "01.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "00 - (HTOA).wav : afb9e4434b212bacdd62e585461af757"
                    << "Artist_01-Album.cue";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "02.1 Without pregap, without HTOA";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "02.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "Artist_02-Album.cue";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::AddToFirstTrack)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "02.2 Without pregap, without HTOA";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "02.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "Artist_02-Album.cue";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "03.1 With pregap, without HTOA";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "03.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "Artist_03-Album.cue";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::AddToFirstTrack)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "04.1 All tags";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "04.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "Artist_04-Album.cue";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::AddToFirstTrack)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "05.1 Cue without tags + tags form the separate file";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "05.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song Title 01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song Title 02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song Title 03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song Title 04.wav : 2e5df99b43b96c208ab26983140dd19f"
                    << "Artist_05-Album.cue";
    req.tags        << "01 PERFORMER: Artist_05, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 01"
                    << "02 PERFORMER: Artist_05, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 02"
                    << "03 PERFORMER: Artist_05, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 03"
                    << "04 PERFORMER: Artist_05, ALBUM: Album, GENRE: Genre, DATE: 2013, TITLE: Song Title 04";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << true
            << int(PreGapType::AddToFirstTrack)
            << ""
            << requests;
    //=====================================================



    //=====================================================
    name = "06.1 Garbage in the CUE";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "06.garbageInTags.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";

    requests << req;
    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "07.1 with symbols and space";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "07.path(with.symbols and space )/07.path(with.symbols and space ).cue";
    req.audioFile   = mAudio_24x96_wav +
                      ":Test_07.path(with.symbols and space )/Audio file (with.symbols and space ).wav";
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";

    requests << req;
    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "08.1 Test_8/Музыка";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "01.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";

    requests << req;
    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::ExtractToFile)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name = "09.1 Multi";
        requests.clear();

    req.clear();
    req.cueFile     = inDir + "01.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";
    requests << req;


    req.clear();
    req.cueFile     = inDir + "02.cuecreator.cue";
    req.audioFile   = mAudio_24x96_flac;
    req.expectedCue = inDir + "02.expected.cue";
    req.resultFiles << "01 - Song01.wav : 07bb5c5fbf7b9429c05e9b650d9df467"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";
    requests << req;


    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::AddToFirstTrack)
            << ""
            << requests;
    //=====================================================


    //=====================================================
    name ="10.1 Temporary dir";
    requests.clear();

    req.clear();
    req.cueFile     = inDir + "01.cuecreator.cue";
    req.audioFile   = mAudio_24x96_wav;
    req.expectedCue = "";
    req.resultFiles << "01 - Song01.wav : a86e2d59bf1e272b5ab7e9a16009455d"
                    << "02 - Song02.wav : 1a199e8e2badff1e643a9f1697ac4140"
                    << "03 - Song03.wav : 71db07cb54faee8545cbed90fe0be6a3"
                    << "04 - Song04.wav : 2e5df99b43b96c208ab26983140dd19f";
    requests << req;

    QTest::newRow(name.toLocal8Bit())
            << false
            << int(PreGapType::ExtractToFile)
            << mTmpDir + "/tmp/tmp/really_tmp"
            << requests;
    //=====================================================
}
