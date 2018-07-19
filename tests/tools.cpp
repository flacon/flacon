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


#include "tools.h"

#include <QTest>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QCryptographicHash>
#include <QIODevice>
#include <QDebug>
#include "../settings.h"
#include "../cue.h"
#include "../disk.h"


/************************************************
 *
 ************************************************/
QString calcAudioHash(const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray ba = f.read(1024);
    int n = ba.indexOf("data");
    f.seek(n + 8);

    QCryptographicHash hash(QCryptographicHash::Md5);
    while (!f.atEnd())
    {
        ba = f.read(1024 * 1024);
        hash.addData(ba);
    }

    return hash.result().toHex();
}


/************************************************
 *
 ************************************************/
TestCueFile::TestCueFile(const QString &fileName):
    mFileName(fileName)
{
}


/************************************************
 *
 ************************************************/
void TestCueFile::setWavFile(const QString &value)
{
    mWavFile = value;
}


/************************************************
 *
 ************************************************/
void TestCueFile::addTrack(const QString &index0, const QString &index1)
{
    mTracks << TestCueTrack(index0, index1);
}


/************************************************
 *
 ************************************************/
void TestCueFile::addTrack(const QString &index1)
{
    addTrack("", index1);
}


/************************************************
 *
 ************************************************/
void TestCueFile::write()
{
    QFile f(mFileName);
    if (!f.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QString("Can't create cue file '%1': %2").arg(mFileName).arg(f.errorString()).toLocal8Bit());

    QTextStream cue(&f);

    cue << QString("FILE \"%1\" WAVE\n").arg(mWavFile);
    for (int i=0; i < mTracks.count(); ++i)
    {
        TestCueTrack track = mTracks.at(i);

        cue << QString("\nTRACK %1 AUDIO\n").arg(i + 1);
        if (track.index0 != "")
            cue << QString("  INDEX 00 %1\n").arg(track.index0);

        if (track.index1 != "")
            cue << QString("  INDEX 01 %1\n").arg(track.index1);
    }

    f.close();
}


/************************************************
 *
 ************************************************/
bool compareAudioHash(const QString &file1, const QString &expected)
{
    if (calcAudioHash(file1) != expected)
    {
        FAIL(QString("Compared hases are not the same for:\n"
                     "    [%1] %2\n"
                     "    [%3] %4\n")

                    .arg(calcAudioHash(file1))
                    .arg(file1)

                    .arg(expected)
                    .arg("expected")

                    .toLocal8Bit());
        return false;
    }
    return true;
}


/************************************************
 *
 ************************************************/
void writeHexString(const QString &str, QIODevice *out)
{
    bool ok;
    int i =0;
    while (i<str.length()-1)
    {
        if (str.at(i).isSpace())
        {
            ++i;
            continue;
        }

        union {
            quint16 n16;
            char b;
        };
        n16 = str.mid(i, 2).toShort(&ok, 16);

        out->write(&b, 1);
        if (!ok)
            throw QString("Incorrect HEX data at %1:\n%2").arg(i).arg(str);
        i+=2;
    }
}

/************************************************
 *
 ************************************************/
void createWavFile(const QString &fileName, int duration, StdWavHeader::Quality quality)
{
    if (QFileInfo(fileName).exists())
        return;

    QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate))
            QFAIL(QString("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());


        int dataSize = StdWavHeader::bytesPerSecond(quality) * duration;
    file.write(StdWavHeader(dataSize, quality).toByteArray());

    quint32 x=123456789, y=362436069, z=521288629;
    union {
        quint32 t;
        char    bytes[4];
    };

    for (uint i=0; i<(dataSize/ sizeof(quint32)); ++i)
    {
        // xorshf96 ...................
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        // xorshf96 ...................

        file.write(bytes, 4);
    }

    file.close();
}


/************************************************
 *
 ************************************************/
void encodeAudioFile(const QString &wavFileName, const QString &outFileName)
{
    if (QFileInfo(outFileName).exists())
        return;

    QString program;
    QStringList args;

    QString ext = QFileInfo(outFileName).suffix();

    if(ext == "ape")
    {
        program = "mac";
        args << wavFileName;
        args << outFileName;
        args << "-c2000";

    }

    else if(ext == "flac")
    {
        program = "flac";
        args << "--silent";
        args << "--force";
        args << "-o" << outFileName;
        args << wavFileName;
    }

    else if(ext == "wv")
    {
        program = "wavpack";
        args << wavFileName;
        args << "-y";
        args << "-q";
        args << "-o" << outFileName;
    }

    else if(ext == "tta")
    {
        program = "ttaenc";
        args << "-o" << outFileName;
        args << "-e";
        args << wavFileName;
        args << "/";
    }

    else
    {
        QFAIL(QString("Can't create file '%1': unknown file format").arg(outFileName).toLocal8Bit());
    }

#if 1
    QProcess proc;
    proc.start(program, args);
    proc.waitForFinished(3 * 60 * 10000);
    if (proc.exitStatus() != 0)
        QFAIL(QString("Can't encode to file '%1':").arg(outFileName).toLocal8Bit() + proc.readAllStandardError());
#else
    QProcess proc;
    if (proc.execute(program, args) != 0)
        QFAIL(QString("Can't encode to file '%1':").arg(outFileName).toLocal8Bit() + proc.readAllStandardError());
#endif

    if (!QFileInfo(outFileName).isFile())
        QFAIL(QString("Can't encode to file '%1' (file don't exists'):").arg(outFileName).toLocal8Bit() + proc.readAllStandardError());
}


/************************************************
 *
 ************************************************/
void testFail(const QString &message, const char *file, int line)
{
    QTest::qFail(message.toLocal8Bit().data(), file, line);
}


/************************************************
 *
 ************************************************/
Disk *loadFromCue(const QString &cueFile)
{
    try
    {
        QVector<CueDisk> cue = CueReader().load(cueFile);
        Disk *res = new Disk();
        res->loadFromCue(cue.first(), true);
        return res;
    }
    catch (FlaconError &err)
    {
        FAIL(err.what());
    }
    return nullptr;
}
