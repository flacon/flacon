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
#include <QBuffer>
#include <QDebug>
#include "../settings.h"
#include "../cue.h"
#include "../disc.h"
#include "../converter/decoder.h"

class HashDevice: public QIODevice {
public:
    HashDevice(QCryptographicHash::Algorithm method, QObject *parent = nullptr):
        QIODevice(parent),
        mHash(method),
        mInHeader(true)
    {
    }

    QByteArray result() const { return mHash.result(); }


protected:
    qint64 readData(char*, qint64) { return -1; }

    qint64 writeData(const char *data, qint64 len)
    {
        if (mInHeader)
        {
            mBuf.append(data, len);
            int n = mBuf.indexOf("data");
            if (n > -1 && n < mBuf.length() - 8)
            {
                mInHeader = false;
                mHash.addData(mBuf.data() + n + 8, mBuf.length() - n - 8);
            }
            return len;
        }

        mHash.addData(data, len);
        return len;
    }

private:
    QByteArray mBuf;
    QCryptographicHash mHash;
    bool mInHeader;
};


/************************************************
 *
 ************************************************/
QString calcAudioHash(const QString &fileName)
{
    Decoder decoder;
    try
    {
        decoder.open(fileName);
    }
    catch (FlaconError &err)
    {
        FAIL(QString("Can't open input file '%1': %2").arg(fileName, err.what()).toLocal8Bit());
        return "";
    }

    if (!decoder.audioFormat())
    {
        FAIL("Unknown format");
        decoder.close();
        return "";
    }

    HashDevice hash(QCryptographicHash::Md5);
    hash.open(QIODevice::WriteOnly);
    decoder.extract(CueTime(), CueTime(), &hash);
    decoder.close();

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
    for (QString line : str.split('\n'))
    {
        for (int i=0;  i<line.length()-1;)
        {
            // Skip comments
            if (line.at(i) == '/')
                break;

            if (line.at(i).isSpace())
            {
                ++i;
                continue;
            }


            union {
                quint16 n16;
                char b;
            };

            bool ok;
            n16 = line.mid(i, 2).toShort(&ok, 16);
            if (!ok)
                throw QString("Incorrect HEX data at %1:\n%2").arg(i).arg(line);

            out->write(&b, 1);
            i+=2;
        }
    }
}


/************************************************
 *
 ************************************************/
QByteArray writeHexString(const QString &str)
{
    QBuffer data;
    data.open(QBuffer::ReadWrite);
    writeHexString(str, &data);
    return data.buffer();
}


/************************************************
 *
 ************************************************/
static void writeTestWavData(QIODevice *device, quint64 dataSize)
{
    static const int BUF_SIZE = 1024 * 1024;

    quint32 x=123456789, y=362436069, z=521288629;
    union {
        quint32 t;
        char    bytes[4];
    };

    QByteArray buf;

    buf.reserve(4 * 1024 * 1024);
    buf.reserve(BUF_SIZE);
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

        buf.append(bytes, 4);
        if (buf.size() >= BUF_SIZE)
        {
            device->write(buf);
            buf.resize(0);
        }
    }

    device->write(buf);
}


/************************************************
 *
 ************************************************/
void createWavFile(const QString &fileName, const QString &header, const int duration)
{
    QBuffer wavHdr;
    wavHdr.open(QBuffer::ReadWrite);
    writeHexString(header, &wavHdr);

    if (duration) {
        quint32 bitsPerSample =
                (quint8(wavHdr.buffer()[28]) << 0) +
                (quint8(wavHdr.buffer()[29]) <<  8) +
                (quint8(wavHdr.buffer()[30]) << 16) +
                (quint8(wavHdr.buffer()[31]) << 24);

        quint32 ckSize = bitsPerSample * duration + wavHdr.buffer().size() - 8 + 8;
        wavHdr.buffer()[4] = quint8(ckSize >>  0);
        wavHdr.buffer()[5] = quint8(ckSize >>  8);
        wavHdr.buffer()[6] = quint8(ckSize >> 16);
        wavHdr.buffer()[7] = quint8(ckSize >> 24);
    }


    // See http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    quint32 ckSize =
            (quint8(wavHdr.buffer()[4]) <<  0) +
            (quint8(wavHdr.buffer()[5]) <<  8) +
            (quint8(wavHdr.buffer()[6]) << 16) +
            (quint8(wavHdr.buffer()[7]) << 24);

    quint32 dataSize = ckSize - wavHdr.size();

    QFile file(fileName);

    if (file.exists() && file.size() == ckSize + 8)
        return;

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QString("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());

    file.write(wavHdr.buffer());
    file.write("data", 4);
    char buf[4];
    buf[0] = quint8(dataSize);
    buf[1] = quint8(dataSize >> 8);
    buf[2] = quint8(dataSize >> 16);
    buf[3] = quint8(dataSize >> 24);
    file.write(buf, 4);

    writeTestWavData(&file, dataSize);
    file.close();
}


/************************************************
 *
 ************************************************/
class TestWavHeader: public WavHeader
{
public:
    TestWavHeader(quint16 bitsPerSample, quint32 sampleRate, quint16 numChannels, uint durationSec)
    {
        mFormat = WavHeader::Format_PCM;
        mNumChannels = numChannels;
        mSampleRate = sampleRate;
        mBitsPerSample = bitsPerSample;
        mFmtSize = 16;
        mByteRate = mSampleRate * mBitsPerSample / 8 * mNumChannels;
        mBlockAlign = mBitsPerSample * mNumChannels / 8;

        mDataStartPos = 12 + 8 + mFmtSize;
        mDataSize = durationSec * mByteRate;

        mFileSize = mDataStartPos + mDataSize;

        mExtSize = 0;
    }
};


/************************************************
 *
 ************************************************/
void createWavFile(const QString &fileName, quint16 bitsPerSample, quint32 sampleRate, uint durationSec)
{
    TestWavHeader header(bitsPerSample, sampleRate, 2, durationSec);

    QFile file(fileName);

    if (file.exists() && file.size() != header.fileSize() + 8)
        return;


    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QString("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());


    file.write(header.toByteArray());
    writeTestWavData(&file, header.dataSize());
    file.close();
}


/************************************************
 *
 ************************************************/
void encodeAudioFile(const QString &wavFileName, const QString &outFileName)
{
    if (QFileInfo(outFileName).exists() && QFileInfo(outFileName).size() > 1024)
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


    bool ok = true;
    QProcess proc;
    proc.start(program, args);
    ok = proc.waitForStarted(3 * 1000);
    ok = ok && proc.waitForFinished(5 * 60 * 1000) ;
    ok = ok && (proc.exitStatus() == 0);

    if (!ok)
    {
        QFAIL(QString("Can't encode %1 %2:")
              .arg(program)
              .arg(args.join(" "))
              .toLocal8Bit()
              + proc.readAllStandardError());
    }

    if (!QFileInfo(outFileName).isFile())
    {
        QFAIL(QString("Can't encode to file '%1' (file don't exists'):")
              .arg(outFileName).toLocal8Bit()
              + proc.readAllStandardError());
    }
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
        res->loadFromCue(cue.first());
        return res;
    }
    catch (FlaconError &err)
    {
        FAIL(err.what());
    }
    return nullptr;
}


/************************************************
 *
 ************************************************/
void TestSettings::apply(const QMap<QString, QVariant> &values)
{
    for (auto i=values.constBegin(); i!=values.constEnd(); ++i) {
        setValue(i.key(), i.value());
    }
    sync();
}
