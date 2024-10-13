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

#include "flacontest.h"
#include <QTest>
#include "tools.h"
#include <QString>
#include <QDebug>
#include <QDir>
#include "settings.h"
#include <QStandardPaths>

/************************************************
 *
 ************************************************/
void TestFlacon::testLoadProfiles()
{
    QFETCH(QString, input);
    QFETCH(QString, expect);

    QString srcDir = mDataDir + "testLoadProfiles";

    QString baseName     = dir() + "/" + QFileInfo(input).baseName();
    QString inFile       = baseName + ".in";
    QString confFile     = baseName + ".conf";
    QString expectedFile = baseName + ".expected";
    QString resultFile   = baseName + ".result";

    if (QFileInfo(srcDir + "/" + input).exists()) {
        QFile::copy(srcDir + "/" + input, inFile);
        QFile::copy(srcDir + "/" + input, confFile);
    }
    QFile::copy(srcDir + "/" + expect, expectedFile);

    TestSettings settings(confFile);
    QSettings    expected(expectedFile, QSettings::IniFormat);
    TestSettings result(resultFile);

    result.writeProfiles(settings.readProfiles());
    result.sync();

    QStringList keys;
    keys << expected.allKeys();
    keys << result.allKeys();
    keys.removeDuplicates();

    bool pass = true;
    foreach (auto key, keys) {
        if (!key.startsWith("Profiles")) {
            continue;
        }

        QString exp = expected.value(key).toString();

        if (exp == ":Music") {
            exp = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
            exp.replace(QDir::homePath(), "~");
        }

        if (result.value(key).toString() == exp)
            continue;

        QString msg = QString("Compared values are not the same\n\tKey\t%1\n\tActual\t%2\n\tExpected\t%3")
                              .arg(key)
                              .arg(result.value(key).toString())
                              .arg(expected.value(key).toString());

        qWarning() << msg.toLocal8Bit();
        pass = false;
    }

    if (!pass)
        QFAIL("Result not the same");
}

/************************************************
 *
 ************************************************/
void TestFlacon::testLoadProfiles_data()
{
    QTest::addColumn<QString>("input", nullptr);
    QTest::addColumn<QString>("expect", nullptr);
    QString srcDir = mDataDir + "testLoadProfiles";

    foreach (QString f, QDir(srcDir).entryList(QStringList("*.expected"), QDir::Files, QDir::Name)) {
        QString name = QFileInfo(f).baseName();
        QTest::newRow(name.toLocal8Bit()) << QFileInfo(f).baseName() + ".in" << f;
    }
}
