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

#include <QTest>
#include <QVector>
#include <QDebug>
#include "types.h"
#include "textcodec.h"

void TestFlacon::testTextCodecNames()
{
    for (int mib : TextCodec::availableMibs()) {

        TextCodec codec = TextCodec::codecForMib(mib);
        QVERIFY(codec.isValid());
    }
}

static QByteArray readSrcData(const QString &fileName)
{
    QFile f(fileName);

    if (!f.open(QFile::ReadOnly)) {
        FAIL(QString("Can't read file %1:\n\t%2").arg(fileName, f.errorString()).toLocal8Bit());
    }

    QByteArray res = QByteArray::fromHex(f.readAll());
    return res;
}

static QByteArray readExpectedata(const QString &fileName)
{
    QFile f(fileName);

    if (!f.open(QFile::ReadOnly)) {
        FAIL(QString("Can't read file %1:\n\t%2").arg(fileName, f.errorString()).toLocal8Bit());
    }

    return f.readAll();
}

void TestFlacon::testTextCodecs()
{
    QFETCH(QString, srcFile);
    QString expectedFile = QFileInfo(srcFile).dir().path() + "/" + QFileInfo(srcFile).baseName() + ".txt";
    QString codecName    = QFileInfo(expectedFile).baseName().section(" - ", 1);

    QByteArray src      = readSrcData(srcFile);
    QString    expected = QString::fromUtf8(readExpectedata(expectedFile)).normalized(QString::NormalizationForm_KC);
    QString    actual;

    try {
        TextCodec codec = TextCodec::codecForName(codecName);
        actual          = codec.decode(src).normalized(QString::NormalizationForm_KC);
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }

    bool fail = false;
    for (int i = 0; i < std::min(expected.size(), actual.size()); ++i) {
        if (actual.at(i) != expected.at(i)) {
            qDebug().nospace() << " * actual[" << i << "] " << actual.at(i) << "  expected[" << i << "] " << expected.at(i);
            fail = true;
        }
    }

    if (fail) {
        QFAIL("Compared values are not the same");
    }

    QCOMPARE(actual.size(), expected.size());
}

void TestFlacon::testTextCodecs_data()
{
    QTest::addColumn<QString>("srcFile", nullptr);
    QString dataDir = mDataDir + "testTextCodecs";

    for (auto src : QDir(dataDir).entryList(QStringList("01*.src.hex"), QDir::Files, QDir::Name)) {
        QTest::newRow(src.toUtf8()) << dataDir + "/" + src;
    }
}
