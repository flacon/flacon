/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2018
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
#include "../cue.h"
#include "tools.h"
#include <QDebug>
#include <QDir>
#include <QSettings>

/************************************************
 *
 ************************************************/
QFile &operator<<(QFile &file, const QString &value)
{
    file.write(value.toLocal8Bit());
    return file;
}

/************************************************
 *
 ************************************************/
QFile &operator<<(QFile &file, const int &value)
{
    return file << QStringLiteral("%1").arg(value);
}

/************************************************
 *
 ************************************************/
static void write(const Cue &cue, const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::WriteOnly | QFile::Truncate);

    int t = -1;
    for (const Cue::Track &track : cue.tracks()) {
        t++;
        f << QStringLiteral("[DISC 01 / TRACK %2]\n").arg(t + 1, 2, 10, QChar('0'));

        f << "\t"
          << "FILE        = " << QString::fromUtf8(track.fileTag()) << "\n";
        f << "\t"
          << "INDEX CD 00 = " << track.cueIndex00().toString(true) << "\n";
        f << "\t"
          << "INDEX CD 01 = " << track.cueIndex01().toString(true) << "\n";
        f << "\t"
          << "INDEX HI 00 = " << track.cueIndex00().toString(false) << "\n";
        f << "\t"
          << "INDEX HI 01 = " << track.cueIndex01().toString(false) << "\n";
        f << "\t"
          << "TITLE       = " << QString::fromUtf8(track.titleTag()) << "\n";
        f << "\t"
          << "ALBUM       = " << QString::fromUtf8(cue.albumTag()) << "\n";
        f << "\t"
          << "PERFORMER   = " << QString::fromUtf8(track.performerTag()) << "\n";
        f << "\t"
          << "DATE        = " << QString::fromUtf8(track.dateTag()) << "\n";
        f << "\t"
          << "DISCID      = " << QString::fromUtf8(cue.discIdTag()) << "\n";
        f << "\t"
          << "GENRE       = " << QString::fromUtf8(track.genreTag()) << "\n";
        f << "\t"
          << "TRACKNUM    = " << QString::number(track.trackNumTag()) << "\n";
        f << "\t"
          << "TRACKCOUNT  = " << QString::number(cue.tracks().count()) << "\n";
        f << "\t"
          << "DISCNUM     = " << QString::number(cue.discNumTag()) << "\n";
        f << "\t"
          << "DISCCOUNT   = " << QString::number(cue.discCountTag()) << "\n";

        f << "\n";
    }

    f.close();
}

/************************************************
 *
 ************************************************/
static void compare(const QString &resFile, const QString &expectedFile)
{
    QSettings result(resFile, QSettings::IniFormat);
    QSettings expected(expectedFile, QSettings::IniFormat);

    foreach (auto key, expected.allKeys()) {
        if (result.value(key).toString() == expected.value(key).toString())
            continue;

        QString msg = QStringLiteral("Compared values are not the same\n\tKey\t%1\n\tActual\t%2\n\tExpected\t%3")
                              .arg(key)
                              .arg(result.value(key).toString())
                              .arg(expected.value(key).toString());

        QFAIL(msg.toLocal8Bit());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCue()
{
    QFETCH(QString, cue);

    QString srcDir = mDataDir + "testCue";

    QString cueFile      = dir() + "/" + cue;
    QString expectedFile = cueFile + ".expected";
    QString resultFile   = cueFile + ".result";

    QFile::copy(srcDir + "/" + cue, cueFile);
    QFile::copy(srcDir + "/" + cue + ".expected", expectedFile);

    try {
        Cue cue(cueFile);
        write(cue, resultFile);

        compare(resultFile, expectedFile);
    }
    catch (FlaconError &err) {
        FAIL(err.what());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCue_data()
{
    QTest::addColumn<QString>("cue", nullptr);
    QString srcDir = mDataDir + "testCue";

    foreach (QString f, QDir(srcDir).entryList(QStringList("*.cue"), QDir::Files, QDir::Name)) {
        QTest::newRow(f.toLocal8Bit()) << f;
    }
}
