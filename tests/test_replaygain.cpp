/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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
#include "convertertest.h"
#include "types.h"

/************************************************
 *
 ************************************************/
void TestFlacon::testReplayGain()
{
    QFETCH(QString, dataDir);
    try {
        ConverterTest tst(dataDir, dir(), mTmpDir);
        tst.run();
        tst.checkReplayGain();
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::testReplayGain_data()
{
    QString curDir = QDir::currentPath();

    QTest::addColumn<QString>("dataDir", nullptr);
    QString dataDir = mDataDir + "testReplayGain";

    for (auto dir : QDir(dataDir).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(dir.toUtf8()) << dataDir + "/" + dir;
    }
    QDir::setCurrent(curDir);
}
