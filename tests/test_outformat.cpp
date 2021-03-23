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

#include <QTest>
#include "testflacon.h"
#include <QDebug>

#include "../formats/outformat.h"

namespace {

class TestOutFormat : OutFormat
{
public:
    using OutFormat::calcQuality;
};

}

/************************************************
 *
 ************************************************/
void TestFlacon::testOutFormatCalcQuality()
{
    QFETCH(int, input);
    QFETCH(int, preferences);
    QFETCH(int, maxFormat);

    QFETCH(int, expected);

    int res = TestOutFormat::calcQuality(input, preferences, maxFormat);

    QCOMPARE(res, expected);
}

/************************************************
 *
 ************************************************/
void TestFlacon::testOutFormatCalcQuality_data()
{
    QTest::addColumn<int>("preferences", nullptr);
    QTest::addColumn<int>("maxFormat", nullptr);
    QTest::addColumn<int>("input", nullptr);
    QTest::addColumn<int>("expected", nullptr);

    //                      preferences maxFormat   input   expected
    QTest::newRow("Bits 01") << 0 << 16 << 16 << 0;
    QTest::newRow("Bits 02") << 0 << 16 << 24 << 16;
    QTest::newRow("Bits 03") << 0 << 16 << 32 << 16;
    /*
    QTest::newRow("Bits 04") << 16 << 16 << 16 << 0;
    QTest::newRow("Bits 05") << 16 << 16 << 24 << 16;
    QTest::newRow("Bits 06") << 16 << 16 << 32 << 16;

    QTest::newRow("Bits 07") << 24 << 16 << 16 << 0;
    QTest::newRow("Bits 08") << 24 << 16 << 24 << 16;
    QTest::newRow("Bits 09") << 24 << 16 << 32 << 16;

    QTest::newRow("Bits 10") << 32 << 16 << 16 << 0;
    QTest::newRow("Bits 11") << 32 << 16 << 24 << 16;
    QTest::newRow("Bits 12") << 32 << 16 << 32 << 16;

    QTest::newRow("Bits 13") << 0 << 24 << 16 << 0;
    QTest::newRow("Bits 14") << 0 << 24 << 24 << 0;
    QTest::newRow("Bits 15") << 0 << 24 << 32 << 24;

    QTest::newRow("Bits 16") << 16 << 24 << 16 << 0;
    QTest::newRow("Bits 17") << 16 << 24 << 24 << 16;
    QTest::newRow("Bits 18") << 16 << 24 << 32 << 16;

    QTest::newRow("Bits 19") << 24 << 24 << 16 << 0;
    QTest::newRow("Bits 20") << 24 << 24 << 24 << 0;
    QTest::newRow("Bits 21") << 24 << 24 << 32 << 24;

    QTest::newRow("Bits 22") << 32 << 24 << 16 << 0;
    QTest::newRow("Bits 23") << 32 << 24 << 24 << 0;
    QTest::newRow("Bits 24") << 32 << 24 << 32 << 24;

    QTest::newRow("Bits 25") << 0 << 32 << 16 << 0;
    QTest::newRow("Bits 26") << 0 << 32 << 24 << 0;
    QTest::newRow("Bits 27") << 0 << 32 << 32 << 0;

    QTest::newRow("Bits 28") << 16 << 32 << 16 << 0;
    QTest::newRow("Bits 29") << 16 << 32 << 24 << 16;
    QTest::newRow("Bits 30") << 16 << 32 << 32 << 16;

    QTest::newRow("Bits 31") << 24 << 32 << 16 << 0;
    QTest::newRow("Bits 32") << 24 << 32 << 24 << 0;
    QTest::newRow("Bits 33") << 24 << 32 << 32 << 24;

    QTest::newRow("Bits 34") << 32 << 32 << 16 << 0;
    QTest::newRow("Bits 35") << 32 << 32 << 24 << 0;
    QTest::newRow("Bits 36") << 32 << 32 << 32 << 0;
*/
}
