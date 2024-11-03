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

#include "flacontest.h"
#include "tools.h"
#include "../formats_in/informat.h"
#include "../settings.h"

#include <QTest>
#include <QString>
#include <QBuffer>
#include <QDebug>

/************************************************
 *
 ************************************************/
void TestFlacon::testFormatWavLast()
{
    if (InputFormat::allFormats().last()->ext() != "wav")
        QFAIL("Last format is not wav in allFormats.");
}

/************************************************
 *
 ************************************************/
void TestFlacon::testFormat()
{
    QFETCH(QString, testdata);
    QFETCH(QString, ext);

    QBuffer data;
    data.open(QBuffer::ReadWrite);
    writeHexString(testdata, &data);
    data.seek(0);

    const InputFormat *format = InputFormat::formatForFile(&data);
    if (!format)
        QFAIL("Can't find format");

    if (format->ext() != ext)
        QFAIL(QStringLiteral("Incorrect format found:\n    found    '%1',\n    expected '%2' ").arg(format->ext(), ext).toLocal8Bit());
}

/************************************************
 *
 ************************************************/
void TestFlacon::testFormat_data()
{
    QTest::addColumn<QString>("testdata", nullptr);
    QTest::addColumn<QString>("ext", nullptr);

    QTest::newRow("01 WAV")
            << "52 49 46 46" // RIFF
            << "wav";

    QTest::newRow("02 FLAC")
            << "66 4C 61 43" // fLaC
            << "flac";
}

/************************************************
 *
 ************************************************/
void TestFlacon::testFormatFromFile()
{
    QFETCH(QString, filename);
    QFETCH(QString, ext);

    const InputFormat *format = InputFormat::formatForFile(filename);
    if (!format)
        QFAIL("Can't find format");

    if (format->ext() != ext)
        QFAIL(QStringLiteral("Incorrect format found:\n    found    '%1',\n    expected '%2' ").arg(format->ext(), ext).toLocal8Bit());
}

/************************************************
 *
 ************************************************/
void TestFlacon::testFormatFromFile_data()
{
    QTest::addColumn<QString>("filename", nullptr);
    QTest::addColumn<QString>("ext", nullptr);

    // WAV ______________________________________
    QTest::newRow("WAV 01") << mAudio_cd_wav << "wav";
    QTest::newRow("WAV 02") << mAudio_24x96_wav << "wav";

    // FLAC _____________________________________
    QTest::newRow("FLAC 01") << mAudio_cd_flac << "flac";
    QTest::newRow("FLAC 02") << mAudio_24x96_flac << "flac";

    // APE ______________________________________
    QTest::newRow("APE 01") << mAudio_cd_ape << "ape";
    QTest::newRow("APE 02") << mAudio_24x96_ape << "ape";

    // TTA ______________________________________
    QTest::newRow("TTA 01") << mAudio_cd_tta << "tta";
    QTest::newRow("TTA 02") << mAudio_24x96_tta << "tta";

    // WV ______________________________________
    QTest::newRow("WV 01") << mAudio_cd_wv << "wv";
    QTest::newRow("WV 02") << mAudio_24x96_wv << "wv";
}
