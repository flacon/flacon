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

#include <QTest>
#include "testflacon.h"
#include "convertertest.h"
#include <QProcess>
#include "types.h"

/************************************************
 *
 ************************************************/
void TestFlacon::testConvert()
{
    QFETCH(QString, dataDir);
    try {
        ConverterTest tst(dataDir, dir(), mTmpDir);
        tst.run();
        tst.check();
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::testConvert_data()
{
    if (QProcessEnvironment::systemEnvironment().contains("FLACON_SKIP_CONVERT_TEST")) {
        QTest::qSkip("Skipping testConvert", __FILE__, __LINE__);
    }

    QString curDir = QDir::currentPath();

    QTest::addColumn<QString>("dataDir", nullptr);
    QString dataDir = mDataDir + "testConvert";

    for (auto dir : QDir(dataDir).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(dir.toUtf8()) << dataDir + "/" + dir;
    }
    QDir::setCurrent(curDir);
}
