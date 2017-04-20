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


#include "../converter/wavheader.h"
#include "testflacon.h"
#include "tools.h"
#include <QTest>
#include <QString>
#include<QBuffer>


/************************************************
 *
 ************************************************/
void TestFlacon::testReadWavHeader()
{
    QFETCH(QString, testdata);
    QFETCH(QString, file_size);
    QFETCH(QString, data_size);
    QFETCH(QString, duration);

    quint64 fileSize = file_size.toLongLong();
    quint64 dataSize = data_size.toLongLong();
    quint64 uduration = duration.toLongLong();

    try
    {
        QBuffer data;
        data.open(QBuffer::ReadWrite);
        writeHexString(testdata, &data);
        data.seek(0);


        WavHeader header;
        header.load(&data);

        QCOMPARE(header.fileSize(), fileSize);
        QCOMPARE(header.dataSize(), dataSize);
        QCOMPARE(header.duration(), uduration);
    }
    catch (char const *err)
    {
        QFAIL(err);
    }


}


/************************************************
 *
 ************************************************/
void TestFlacon::testReadWavHeader_data()
{
    QTest::addColumn<QString>("testdata");
    QTest::addColumn<QString>("file_size");
    QTest::addColumn<QString>("data_size");
    QTest::addColumn<QString>("duration");

    QTest::newRow("1") <<
                         "52 49 46 46"  // RIFF
                         "24 B9 4D 02"  // file size - 8
                         "57 41 56 45"  // WAVE

                         "66 6D 74 20"  // "fmt "
                         "10 00 00 00"  // Chunk size
                         "01 00"        // AudioFormat
                         "02 00"        // NumChannels

                         "44 AC 00 00"  // mSampleRate
                         "10 B1 02 00"  // mByteRate
                         "04 00"        // mBlockAlign
                         "10 00"        // mBitsPerSample

                         "64 61 74 61"  // data
                         "00 B9 4D 02"  // data size
                         "00"

                      << "38648108"     // file size
                      << "38648064"     // data size
                      << "219093"       // duration   3 min 39 s 93 ms
                      ;


    QTest::newRow("2") <<
                         "52 49 46 46"  // RIFF
                         "46 B9 4D 02"  // file size - 8
                         "57 41 56 45"  // WAVE

                         "66 6D 74 20"  // "fmt "
                         "10 00 00 00"  // Chunk size
                         "01 00"        // AudioFormat
                         "02 00"        // NumChannels

                         "44 AC 00 00"  // mSampleRate
                         "10 B1 02 00"  // mByteRate
                         "04 00"        // mBlockAlign
                         "10 00"        // mBitsPerSample

                         "4C 49 53 54"  // LIST
                         "1A 00 00 00"  // Chunk size
                         "49 4E 46 4F"
                         "49 53 46 54"
                         "0E 00 00 00"
                         "4C 61 76 66"
                         "35 37 2E 34"
                         "31 2E 31 30"
                         "30 00 "

                         "64 61 74 61"  // data
                         "00 B9 4D 02"  // data size
                         "00"

                      << "38648142"     // file size
                      << "38648064"     // data size
                      << "219093"       // duration   3 min 39 s 93 ms
                      ;


    QTest::newRow("3") <<
                          "52 49 46 46" // RIFF
                          "24 A3 23 A8" // file size - 8
                          "57 41 56 45" // WAVE

                          "66 6D 74 20" // "fmt "
                          "10 00 00 00" // Chunk size
                          "01 00"       // AudioFormat
                          "02 00"       // NumChannels

                          "00 EE 02 00" // mSampleRate
                          "00 94 11 00" // mByteRate
                          "06 00"       // mBlockAlign
                          "18 00"       // mBitsPerSample

                          "64 61 74 61" // data
                          "00 A3 23 A8" // data size

                       << "2820907820"  // file size
                       << "2820907776"  // data size
                       << "2448704"     // duration   40 min 48 s 704 ms
                          ;

}




