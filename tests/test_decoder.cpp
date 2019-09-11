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
#include "../converter/decoder.h"
#include "../formats/wav.h"
#include "../formats/flac.h"

#include <QTest>
#include <QVector>
#include <QDebug>

struct TestTrack {
    QString start;
    QString end;
    QString hash;
};


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder()
{
    QFETCH(QStringList, data);
    QString inputFile = data.first();

    QVector<TestTrack> tracks;
    for (int i=1; i < data.count(); i+=3)
    {
        TestTrack track = {data.at(i), data.at(i+1), data.at(i+2)};
        tracks << track;
    }


    // Flacon decoder ___________________________
    Decoder decoder;
    try
    {
        decoder.open(inputFile);
    }
    catch (FlaconError &err)
    {
        QFAIL(QString("Can't open input file '%1': %2").arg(inputFile, err.what()).toLocal8Bit());
    }

    if (!decoder.audioFormat())
        QFAIL("Unknown format");


    for (int i=0; i < tracks.count(); ++i)
    {
        TestTrack track = tracks.at(i);

        QString flaconFile = QString("%1/%2-flacon.wav").arg(dir()).arg(i + 1, 3, 10, QChar('0'));

        try
        {
            decoder.extract(CueTime(track.start), CueTime(track.end), flaconFile);
        }
        catch (FlaconError &err)
        {
            QFAIL(QString("Can't extract file '%1' [%2-%3]: %4")
                  .arg(inputFile)
                  .arg(track.start, track.end)
                  .arg(err.what()).toLocal8Bit());
        }
    }
    decoder.close();


    // Checks ___________________________________
    for (int i=0; i < tracks.count(); ++i)
    {
        compareAudioHash(
                    QString("%1/%2-flacon.wav").arg(dir()).arg(i + 1, 3, 10, QChar('0')),
                    tracks.at(i).hash);
    }


}


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder_data()
{
    QTest::addColumn<QStringList>("data", nullptr);


    // Wav ______________________________________
    QTest::newRow("WAV 001 cd")
            << (QStringList()
                << mAudio_cd_wav
                << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73"
                << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df"
                << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2"
                );

    QTest::newRow("WAV 002 cd")
            << (QStringList()
                << mAudio_cd_wav
                << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc"
                << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02"
                << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87"
                );

    QTest::newRow("WAV 003 24x96")
            << (QStringList()
                << mAudio_24x96_wav
                << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd"
                << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62"
                << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0"
                );



    // Flac _____________________________________
    QTest::newRow("FLAC 001 cd")
            << (QStringList()
                << mAudio_cd_flac
                << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73"
                << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df"
                << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2"
                );

    QTest::newRow("FLAC 002 cd")
            << (QStringList()
                << mAudio_cd_flac
                << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc"
                << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02"
                << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87"
                );

    QTest::newRow("FLAC 003 24x96")
            << (QStringList()
                << mAudio_24x96_flac
                << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd"
                << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62"
                << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0"
                );


    // APE ______________________________________
    QTest::newRow("APE 001 cd")
            << (QStringList()
                << mAudio_cd_ape
                << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73"
                << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df"
                << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2"
                );

    QTest::newRow("APE 002 cd")
            << (QStringList()
                << mAudio_cd_ape
                << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc"
                << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02"
                << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87"
                );

    QTest::newRow("APE 003 24x96")
            << (QStringList()
                << mAudio_24x96_ape
                << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd"
                << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62"
                << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0"
                );


    // TTA ______________________________________
    QTest::newRow("TTA 001 cd")
            << (QStringList()
                << mAudio_cd_tta
                << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73"
                << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df"
                << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2"
                );

    QTest::newRow("TTA 002 cd")
            << (QStringList()
                << mAudio_cd_tta
                << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc"
                << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02"
                << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87"
                );

    QTest::newRow("TTA 003 24x96")
            << (QStringList()
                << mAudio_24x96_tta
                << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd"
                << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62"
                << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0"
                );


    // WV ______________________________________
    QTest::newRow("WV 001 cd")
            << (QStringList()
                << mAudio_cd_wv
                << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73"
                << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df"
                << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2"
                );

    QTest::newRow("WV 002 cd")
            << (QStringList()
                << mAudio_cd_wv
                << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc"
                << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02"
                << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87"
                );

    QTest::newRow("WV 003 24x96")
            << (QStringList()
                << mAudio_24x96_wv
                << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd"
                << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62"
                << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0"
                );

}


