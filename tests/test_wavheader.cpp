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
#include "types.h"
#include <QTest>
#include <QString>
#include <QBuffer>


/************************************************
 *
 ************************************************/
void TestFlacon::testReadWavHeader()
{
    QFETCH(QString, testdata);
    QFETCH(QString, file_size);
    QFETCH(QString, data_size);
    QFETCH(QString, duration);

    quint32 expectedFileSize = file_size.toLongLong();
    quint32 expectedDataSize = data_size.toLongLong();
    QTime   expectedDuration = QTime::fromString(duration, "hh:mm:ss.zzz");

    try
    {
        QBuffer data;
        data.open(QBuffer::ReadWrite);
        writeHexString(testdata, &data);

        data.seek(0);
        WavHeader header(&data);

        QCOMPARE(header.toByteArray().toHex(), data.buffer().toHex());

        QCOMPARE(header.fileSize(), expectedFileSize);
        QCOMPARE(header.dataSize(), expectedDataSize);
        QCOMPARE(QTime::fromMSecsSinceStartOfDay(header.duration()), expectedDuration);

        // Test copy operator
        WavHeader copyOperator = header;
        QCOMPARE(copyOperator.toByteArray().toHex(), data.buffer().toHex());


        // Test copy constructor
        WavHeader copyConstructor(header);
        QCOMPARE(copyConstructor.toByteArray().toHex(), data.buffer().toHex());
    }
    catch (FlaconError &err)
    {
        FAIL(err.what());
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
    QTest::addColumn<QString>("testdata",  nullptr);
    QTest::addColumn<QString>("file_size", nullptr);
    QTest::addColumn<QString>("data_size", nullptr);
    QTest::addColumn<QString>("duration",  nullptr);


    QTest::newRow("01") <<
                         "52 49 46 46"      // RIFF
                         "24 B9 4D 02"      // file size - 8
                         "57 41 56 45"      // WAVE

                         "66 6D 74 20"      // "fmt "
                         "10 00 00 00"      // Chunk size
                         "01 00"            // AudioFormat
                         "02 00"            // NumChannels

                         "44 AC 00 00"      // mSampleRate
                         "10 B1 02 00"      // mByteRate
                         "04 00"            // mBlockAlign
                         "10 00"            // mBitsPerSample

                         "64 61 74 61"      // data
                         "00 B9 4D 02"      // data size

                      << "38648108"         // file size
                      << "38648064"         // data size
                      << "00:03:39.093"     // duration
                      ;


    QTest::newRow("02") <<
                         "52 49 46 46"      // RIFF
                         "46 B9 4D 02"      // file size - 8
                         "57 41 56 45"      // WAVE

                         "66 6D 74 20"      // "fmt "
                         "10 00 00 00"      // Chunk size
                         "01 00"            // AudioFormat
                         "02 00"            // NumChannels

                         "44 AC 00 00"      // mSampleRate
                         "10 B1 02 00"      // mByteRate
                         "04 00"            // mBlockAlign
                         "10 00"            // mBitsPerSample

                         "4C 49 53 54"      // LIST
                         "1A 00 00 00"      // Chunk size
                         "49 4E 46 4F"
                         "49 53 46 54"
                         "0E 00 00 00"
                         "4C 61 76 66"
                         "35 37 2E 34"
                         "31 2E 31 30"
                         "30 00 "

                         "64 61 74 61"      // data
                         "00 B9 4D 02"      // data size

                      << "38648142"         // file size
                      << "38648064"         // data size
                      << "00:03:39.093"     // duration
                      ;


    QTest::newRow("03") <<
                          "52 49 46 46"     // RIFF
                          "24 A3 23 A8"     // file size - 8
                          "57 41 56 45"     // WAVE

                          "66 6D 74 20"     // "fmt "
                          "10 00 00 00"     // Chunk size
                          "01 00"           // AudioFormat
                          "02 00"           // NumChannels

                          "00 EE 02 00"     // mSampleRate
                          "00 94 11 00"     // mByteRate
                          "06 00"           // mBlockAlign
                          "18 00"           // mBitsPerSample

                          "64 61 74 61"     // data
                          "00 A3 23 A8"     // data size

                       << "2820907820"      // file size
                       << "2820907776"      // data size
                       << "00:40:48.704"    // duration
                          ;


    QTest::newRow("04") <<
                          "52 49 46 46"     // RIFF
                          "D0 9B 9B AF"     // file size - 8
                          "57 41 56 45"     // WAVE

                          "66 6D 74 20"     // "fmt "
                          "28 00 00 00"     // Chunk size 40
                                "FE FF"     // AudioFormat
                                "02 00"     // NumChannels

                          "00 77 01 00"     // mSampleRate
                          "00 CA 08 00"     // mByteRate
                                "06 00"     // mBlockAlign
                                "18 00"     // mBitsPerSample

                                "16 00"     // Size of the extension
                                "18 00"     // Number of valid bits
                          "03 00 00 00"     // Speaker position mask
                          "01 00 00 00"     // GUID, including the data format code
                          "00 00 10 00"     //
                          "80 00 00 AA"     //
                          "00 38 9B 71"     //

                          "64 61 74 61"     // data
                          "94 9B 9B AF"     // data size

                       << "2946210776"      // file size
                       << "2946210708"      // data size
                       << "01:25:14.949"    // duration
                          ;

    QTest::newRow("05") <<
                          "52 49 46 46"     // RIFF
                          "D0 9B 9B AF"     // file size - 8
                          "57 41 56 45"     // WAVE

                          "66 6D 74 20"     // "fmt "
                          "28 00 00 00"     // Chunk size 40
                                "FE FF"     // AudioFormat
                                "02 00"     // NumChannels

                          "00 77 01 00"     // mSampleRate
                          "00 CA 08 00"     // mByteRate
                                "06 00"     // mBlockAlign
                                "18 00"     // mBitsPerSample

                                "16 00"     // Size of the extension
                                "18 00"     // Number of valid bits
                          "03 00 00 00"     // Speaker position mask
                          "01 00 00 00"     // GUID, including the data format code
                          "00 00 10 00"     //
                          "80 00 00 AA"     //
                          "00 38 9B 71"     //

                          "64 61 74 61"     // data
                          "94 9B 9B AF"     // data size

                       << "2946210776"      // file size
                       << "2946210708"      // data size
                       << "01:25:14.949"    // duration
                          ;

    QTest::newRow("05") <<
                           "52 49 46 46"
                           "2A DF 02 00"
                           "57 41 56 45"

                           "66 6D 74 20"    // Chunk ID
                           "28 00 00 00"    // Chunk size
                                 "FE FF"    //Format code
                                 "02 00"
                           "40 1F 00 00"
                           "00 FA 00 00"
                                 "08 00"
                                 "20 00"
                                 "16 00"    // Size of the extension
                                 "20 00"    // Number of valid bits
                           "03 00 00 00"    // Speaker position mask
                           "03 00 00 00"    // GUID, including the data format code
                           "00 00 10 00"
                           "80 00 00 AA"
                           "00 38 9B 71"

                           "66 61 63 74"    // fact chunk
                           "04 00 00 00"    // Chunk size
                           "C5 5B 00 00"

                           "64 61 74 61"    // data chunk
                           "28 DE 02 00"    // Chunk size

                        << "188210"         // file size
                        << "187944"         // data size
                        << "00:00:02.936";  // duration


    QTest::newRow("06 zero duration") <<
                            "52 49 46 46"   // RIFF
                            "24 00 00 00"   // ChunkSize 36
                            "57 41 56 45"   // Format WAVE

                            "66 6D 74 20"   // Chunk ID
                            "10 00 00 00"   // Chunk size
                                  "01 00"   // Format code
                                  "01 00"   // NumChannels
                            "80 BB 00 00"   // SampleRate
                            "00 77 01 00"   // ByteRate
                                  "02 00"   // BlockAlign
                                  "10 00"   // BitsPerSample

                            "64 61 74 61"   // data Chunk ID
                            "00 00 00 00"   // Chunk size

                        << "44"             // file size
                        << "0"              // data size
                        << "00:00:00.000";  // duration

}


/************************************************
 *
 ************************************************/
void TestFlacon::testResizeWavHeader()
{
    QFETCH(QString, header);
    QFETCH(QString, dataSize);
    QFETCH(QString, expectedChunkSize);
    QFETCH(QString, expectedDataSize);

    QByteArray expChunkSize = writeHexString(expectedChunkSize);
    QByteArray expDataSize  = writeHexString(expectedDataSize);
    quint32    dSize = dataSize.toLongLong();

    try
    {
        QBuffer data;
        data.open(QBuffer::ReadWrite);
        writeHexString(header, &data);

        data.seek(0);
        WavHeader wavHeader(&data);
        wavHeader.resizeData(dSize);

        QByteArray res = wavHeader.toByteArray();
        QByteArray resChunkSize = res.mid(4, 4);
        QByteArray resDataSize  = res.mid(res.length() - 4, 4);

        QCOMPARE(resDataSize.toHex(),  expDataSize.toHex());
        QCOMPARE(resChunkSize.toHex(), expChunkSize.toHex());
    }
    catch (FlaconError &err)
    {
        FAIL(err.what());
    }
    catch (char const *err)
    {
        QFAIL(err);
    }
}


/************************************************
 *
 ************************************************/
void TestFlacon::testResizeWavHeader_data()
{
    QTest::addColumn<QString>("header",            nullptr);
    QTest::addColumn<QString>("dataSize",          nullptr);
    QTest::addColumn<QString>("expectedChunkSize", nullptr);
    QTest::addColumn<QString>("expectedDataSize",  nullptr);

    QTest::newRow("01")
            << "52 49 46 46"    // RIFF
               "10 FF 00 00"    // file size - 8
               "57 41 56 45"    // WAVE

               "66 6D 74 20"    // "fmt "
               "10 00 00 00"    // Chunk size
               "01 00"          // AudioFormat
               "02 00"          // NumChannels

               "44 AC 00 00"    // mSampleRate
               "10 B1 02 00"    // mByteRate
               "04 00"          // mBlockAlign
               "10 00"          // mBitsPerSample

               "64 61 74 61"    // data
               "00 FF 00 00"    // data size

            << "38648064"       // new duration
            << "24 B9 4D 02"    // expected chunk size
            << "00 B9 4D 02";   // expected data size

}
