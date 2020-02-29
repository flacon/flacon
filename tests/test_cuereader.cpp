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


#include "testflacon.h"
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
    return file << QString("%1").arg(value);
}


/************************************************
 *
 ************************************************/
static void write(const QVector<CueDisc> &cue, const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::WriteOnly | QFile::Truncate);

    for (int d=0; d<cue.count(); ++d)
    {
        auto disc = cue.at(d);

        int t = -1;
        foreach (const Track &track, disc)
        {
            t++;
            f << QString("[DISC %1 / TRACK %2]\n").arg(d+1, 2, 10, QChar('0')).arg(t+1, 2, 10, QChar('0'));

            f << "\t" << "FILE        = " << track.tag(TagId::File)               << "\n";
            f << "\t" << "INDEX CD 00 = " << track.cueIndex(0).toString(true)     << "\n";
            f << "\t" << "INDEX CD 01 = " << track.cueIndex(1).toString(true)     << "\n";
            f << "\t" << "INDEX HI 00 = " << track.cueIndex(0).toString(false)    << "\n";
            f << "\t" << "INDEX HI 01 = " << track.cueIndex(1).toString(false)    << "\n";
            f << "\t" << "TITLE       = " << track.tag(TagId::Title)              << "\n";
            f << "\t" << "ALBUM       = " << track.tag(TagId::Album)              << "\n";
            f << "\t" << "PERFORMER   = " << track.tag(TagId::Artist)             << "\n";
            f << "\t" << "DATE        = " << track.tag(TagId::Date)               << "\n";
            f << "\t" << "DISCID      = " << track.tag(TagId::DiscId)             << "\n";
            f << "\t" << "GENRE       = "  << track.tag(TagId::Genre)             << "\n";
            f << "\t" << "TRACKNUM    = " << QString::number(track.trackNum())    << "\n";
            f << "\t" << "TRACKCOUNT  = " << QString::number(track.trackCount())  << "\n";
            f << "\t" << "DISCNUM     = "  << QString::number(track.discNum())    << "\n";
            f << "\t" << "DISCCOUNT   = "  << QString::number(track.discCount())  << "\n";


            f << "\n";
        }
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

    foreach (auto key, expected.allKeys())
    {
        if (result.value(key).toString() == expected.value(key).toString())
            continue;

        QString msg = QString("Compared values are not the same\n\tKey\t%1\n\tActual\t%2\n\tExpected\t%3")
                .arg(key)
                .arg(result.value(key).toString())
                .arg(expected.value(key).toString());

        QFAIL(msg.toLocal8Bit());
    }

}


/************************************************
 *
 ************************************************/
void TestFlacon::testCueReader()
{
    QFETCH(QString, cue);

    QString srcDir = mDataDir + "testCueReader";

    QString cueFile      = dir() + "/" + cue;
    QString expectedFile = cueFile + ".expected";
    QString resultFile   = cueFile + ".result";

    QFile::copy(srcDir + "/" + cue, cueFile);
    QFile::copy(srcDir + "/" + cue + ".expected", expectedFile);

    try
    {
        CueReader reader;
        auto cue = reader.load(cueFile);
        write(cue, resultFile);

        compare(resultFile, expectedFile);
    }
    catch (FlaconError &err)
    {
        FAIL(err.what());
    }
}


/************************************************
 *
 ************************************************/
void TestFlacon::testCueReader_data()
{
    QTest::addColumn<QString>("cue", nullptr);
    QString srcDir = mDataDir + "testCueReader";

    foreach(QString f, QDir(srcDir).entryList(QStringList("*.cue"), QDir::Files, QDir::Name))
    {
        QTest::newRow(f.toLocal8Bit()) << f;

    }
}
