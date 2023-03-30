/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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
#include <QTest>
#include "tools.h"
#include "../cuedata.h"
#include "types.h"

static QString toString(const CueData::Tags &tags)
{
    QString res;
    for (QByteArray k : tags.keys()) {
        res += QString("%1 = %2\n").arg(QString::fromLatin1(k)).arg(QString::fromLocal8Bit(tags.value(k)));
    }
    return res;
}

static QString toString(QSettings &settings)
{
    QString res;
    for (const QString &k : settings.allKeys()) {
        res += QString("%1 = %2\n").arg(k).arg(settings.value(k).toString());
    }
    return res;
}

void TestFlacon::testCueData()
{
    QFETCH(QString, cueFile);
    QFETCH(QString, expectedFile);

    QSettings expected(expectedFile, QSettings::IniFormat);

    try {
        CueData result(cueFile);

        // Check global tags ....................
        expected.beginGroup("GLOBAL");
        QCOMPARE(toString(result.globalTags()), toString(expected));
        expected.endGroup();

        // Check track tags .....................
        QCOMPARE(result.tracks().count(),
                 expected.childGroups().filter("TRACK").count());

        int n = 0;
        for (auto track : result.tracks()) {
            n++;
            expected.beginGroup(QString("TRACK %1").arg(n, 2, 10, QChar('0')));
            QCOMPARE(toString(track), toString(expected));
            expected.endGroup();
        }
    }
    catch (FlaconError &err) {
        FAIL(err.what());
    }
}

void TestFlacon::testCueData_data()
{
    QTest::addColumn<QString>("cueFile", nullptr);
    QTest::addColumn<QString>("expectedFile", nullptr);

    QString srcDir = mDataDir + "testCueData";

    foreach (QString f, QDir(srcDir).entryList(QStringList("*.cue"), QDir::Files, QDir::Name)) {
        QTest::newRow(f.toLocal8Bit())
                << (srcDir + "/" + f)
                << (srcDir + "/" + f + ".expected");
    }
}
