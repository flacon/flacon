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
};


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder()
{
    QString dir = makeTestDir();

    QFETCH(QStringList, data);
    QString inputFile = data.first();

    QVector<TestTrack> tracks;
    for (int i=1; i < data.count(); i+=2)
    {
        TestTrack track = {data.at(i), data.at(i+1) };
        tracks << track;
    }


    // Flacon decoder ___________________________
    const Format *format = Format::formatForFile(inputFile);
    if (!format)
        QFAIL("Unknown format");

    Decoder decoder(*format);
    if (!decoder.open(inputFile))
        QFAIL(QString("Can't open input file '%1': %2").arg(inputFile, decoder.errorString()).toLocal8Bit());

    for (int i=0; i < tracks.count(); ++i)
    {
        TestTrack track = tracks.at(i);

        QString flaconFile = QString("%1/%2-flacon.wav").arg(dir).arg(i + 1, 3, 10, QChar('0'));

        bool res = decoder.extract(
                    CueTime(track.start),
                    CueTime(track.end),
                    flaconFile);

        if (!res)
            QFAIL(QString("Can't extract file '%1' [%2-%3]: %4")
                  .arg(inputFile)
                  .arg(track.start, track.end)
                  .arg(decoder.errorString()).toLocal8Bit());
    }
    decoder.close();


    // ShnSplitter ______________________________
    TestCueFile cueFile(QString("%1/cue.cue").arg(dir));
    cueFile.setWavFile(inputFile);

    for (int i=0; i < tracks.count(); ++i)
    {
        TestTrack track = tracks.at(i);
        cueFile.addTrack(track.start);
    }
    cueFile.addTrack(tracks.last().end);
    cueFile.write();

    QStringList shnFiles = shnSplit(cueFile.fileName(), inputFile);



    // Checks ___________________________________
    if (shnFiles.length() < tracks.count())
        QFAIL("Not all files was extracted.");

    for (int i=0; i < tracks.count(); ++i)
    {
        compareAudioHash(
                    QString("%1/%2-flacon.wav").arg(dir).arg(i + 1, 3, 10, QChar('0')),
                    shnFiles.at(i));
    }


}


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder_data()
{
    QTest::addColumn<QStringList>("data");


    // Wav ______________________________________
    QTest::newRow("WAV 001 cd")
            << (QStringList()
                << mAudio_cd_wav
                << "00:00:00" << "00:30:00"
                << "00:30:00" << "01:30:00"
                << "01:30:00" << "02:30:00"
                );

    QTest::newRow("WAV 002 cd")
            << (QStringList()
                << mAudio_cd_wav
                << "00:00:10" << "00:30:00"
                << "00:30:00" << "01:30:20"
                << "01:30:20" << "02:30:30"
                );

    QTest::newRow("WAV 003 24x96")
            << (QStringList()
                << mAudio_24x96_wav
                << "00:00:000" << "00:30:000"
                << "00:30:000" << "01:30:000"
                << "01:30:000" << "02:30:000"
                );


    // Flac _____________________________________
    QTest::newRow("FLAC 001 cd")
            << (QStringList()
                << mAudio_cd_flac
                << "00:00:00" << "00:30:00"
                << "00:30:00" << "01:30:00"
                << "01:30:00" << "02:30:00"
                );

    QTest::newRow("FLAC 002 cd")
            << (QStringList()
                << mAudio_cd_flac
                << "00:00:10" << "00:30:00"
                << "00:30:00" << "01:30:20"
                << "01:30:20" << "02:30:30"
                );

    QTest::newRow("FLAC 003 24x96")
            << (QStringList()
                << mAudio_24x96_flac
                << "00:00:000" << "00:30:000"
                << "00:30:000" << "01:30:000"
                << "01:30:000" << "02:30:000"
                );


    // APE ______________________________________
    QTest::newRow("APE 001 cd")
            << (QStringList()
                << mAudio_cd_ape
                << "00:00:00" << "00:30:00"
                << "00:30:00" << "01:30:00"
                << "01:30:00" << "02:30:00"
                );

    QTest::newRow("APE 002 cd")
            << (QStringList()
                << mAudio_cd_ape
                << "00:00:10" << "00:30:00"
                << "00:30:00" << "01:30:20"
                << "01:30:20" << "02:30:30"
                );

    QTest::newRow("APE 003 24x96")
            << (QStringList()
                << mAudio_24x96_ape
                << "00:00:000" << "00:30:000"
                << "00:30:000" << "01:30:000"
                << "01:30:000" << "02:30:000"
                );


    // TTA ______________________________________
    QTest::newRow("TTA 001 cd")
            << (QStringList()
                << mAudio_cd_tta
                << "00:00:00" << "00:30:00"
                << "00:30:00" << "01:30:00"
                << "01:30:00" << "02:30:00"
                );

    QTest::newRow("TTA 002 cd")
            << (QStringList()
                << mAudio_cd_tta
                << "00:00:10" << "00:30:00"
                << "00:30:00" << "01:30:20"
                << "01:30:20" << "02:30:30"
                );

    QTest::newRow("TTA 003 24x96")
            << (QStringList()
                << mAudio_24x96_tta
                << "00:00:000" << "00:30:000"
                << "00:30:000" << "01:30:000"
                << "01:30:000" << "02:30:000"
                );


    // WV ______________________________________
    QTest::newRow("WV 001 cd")
            << (QStringList()
                << mAudio_cd_wv
                << "00:00:00" << "00:30:00"
                << "00:30:00" << "01:30:00"
                << "01:30:00" << "02:30:00"
                );

    QTest::newRow("WV 002 cd")
            << (QStringList()
                << mAudio_cd_wv
                << "00:00:10" << "00:30:00"
                << "00:30:00" << "01:30:20"
                << "01:30:20" << "02:30:30"
                );

    QTest::newRow("WV 003 24x96")
            << (QStringList()
                << mAudio_24x96_wv
                << "00:00:000" << "00:30:000"
                << "00:30:000" << "01:30:000"
                << "01:30:000" << "02:30:000"
                );

}


