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
static void write(const QVector<CueDisk> &cue, const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::WriteOnly | QFile::Truncate);

    for (int d=0; d<cue.count(); ++d)
    {
        auto disk = cue.at(d);

        int t = -1;
        foreach (const Track &track, disk)
        {
            t++;

            if (t != 0)
                f << "\n";

            QString trackTxt = QString("DISK.%1 TRACK.%2 ").arg(d+1, 2, 10, QChar('0')).arg(t+1, 2, 10, QChar('0'));
            f << trackTxt << "FILE: "       << track.tag(TagId::File)           << "\n";
            f << trackTxt << "INDEX 00: "   << track.cueIndex(0).toString(true) << "\n";
            f << trackTxt << "INDEX 01: "   << track.cueIndex(1).toString(true) << "\n";
            f << trackTxt << "TITLE: "      << track.tag(TagId::Title)          << "\n";
            f << trackTxt << "ALBUM: "      << track.tag(TagId::Album)          << "\n";
            f << trackTxt << "PERFORMER: "  << track.tag(TagId::Performer)      << "\n";
            f << trackTxt << "DATE: "       << track.tag(TagId::Date)           << "\n";
            f << trackTxt << "DISKID: "     << track.tag(TagId::DiscId)         << "\n";
        }
    }

    f.close();
}


/************************************************
 *
 ************************************************/
void TestFlacon::testCueReader()
{

    QString srcDir = mDataDir + "testCueReader";

    foreach(QString f, QDir(srcDir).entryList(QStringList("*.cue"), QDir::Files, QDir::Name))
    {
        QString cueFile      = dir() + "/" + f;
        QString expectedFile = cueFile + ".expected";
        QString resultFile   = cueFile + ".result";

        QFile::copy(srcDir + "/" + f, cueFile);
        QFile::copy(srcDir + "/" + f + ".expected", expectedFile);

        try
        {
            CueReader reader;
            auto cue = reader.load(cueFile);
            write(cue, resultFile);

            QString err;
            if (!TestFlacon::compareCue(resultFile, expectedFile, &err, true))
                FAIL(err.toLocal8Bit());
        }
        catch (FlaconError &err)
        {
            FAIL(err.message());
        }
    }

}
