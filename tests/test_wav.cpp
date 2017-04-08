#include "../converter/wav.h"
#include "testflacon.h"
#include <QTest>
#include <QString>
#include<QBuffer>


void writeHexString(const QString &str, QIODevice *out)
{
    bool ok;
    int i =0;
    while (i<str.length()-1)
    {
        if (str.at(i).isSpace())
        {
            ++i;
            continue;
        }

        union {
            quint16 n16;
            char b;
        };
        n16 = str.mid(i, 2).toShort(&ok, 16);

        out->write(&b, 1);
        if (!ok)
            throw QString("Incorrect HEX data at %1:\n%2").arg(i).arg(str);
        i+=2;
    }
}

void TestFlacon::testWavHeader()
{
    QFETCH(QString, testdata);
    QFETCH(qint32, file_size);
    QFETCH(qint32, data_size);

    quint32 fileSize = file_size;
    quint32 dataSize = data_size;

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
    }
    catch (char const *err)
    {
        QFAIL(err);
    }


}

void TestFlacon::testWavHeader_data()
{
    QTest::addColumn<QString>("testdata");
    QTest::addColumn<qint32>("file_size");
    QTest::addColumn<qint32>("data_size");

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

                      << 38648108       // file size
                      << 38648064       // data size
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

                      << 38648142       // file size
                      << 38648064       // data size
                      ;

}
