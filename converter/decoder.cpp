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


#include "decoder.h"
#include "../cue.h"
#include "../settings.h"

#include <QIODevice>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QDebug>

#define MAX_BUF_SIZE            4096



/************************************************
 *
 ************************************************/
qint64 timeToBytes(CueTime time, const WavHeader &wav)
{
    if (wav.isCdQuality())
    {
        return (qint64)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else
    {
        return (qint64)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}


/************************************************
 *
 ************************************************/
Decoder::Decoder(QObject *parent) :
    QObject(parent),
    mFormat(NULL),
    mProcess(NULL),
    mFile(NULL),
    mPos(0)
{

}


/************************************************
 *
 ************************************************/
Decoder::Decoder(const AudioFormat &format, QObject *parent) :
    QObject(parent),
    mFormat(&format),
    mProcess(NULL),
    mFile(NULL),
    mPos(0)
{

}


/************************************************
 *
 ************************************************/
Decoder::~Decoder()
{
    close();
    delete mFile;
    delete mProcess;
}


/************************************************
 *
 ************************************************/
bool Decoder::open(const QString fileName)
{
    mInputFile = fileName;
    if (!mFormat)
        mFormat = AudioFormat::formatForFile(fileName);

    if (!mFormat)
    {
        mErrorString = "Unknown format";
        return false;
    }

    if (!mFormat->decoderProgramName().isEmpty())
        return openProcess();
    else
        return openFile();
}


/************************************************
 *
 ************************************************/
bool Decoder::openFile()
{
    mFile = new QFile(mInputFile, this);
    if (!mFile->open(QFile::ReadOnly))
    {
         mErrorString = mFile->errorString();
        return false;
    }

    try
    {
        mWavHeader.load(mFile);
        mPos = mWavHeader.dataStartPos();
        return true;
    }
    catch (char const *err)
    {
        mErrorString = err;
        return false;
    }
}


/************************************************
 *
 ************************************************/
bool Decoder::openProcess()
{
    mProcess = new QProcess(this);

    QString program = settings->programName(mFormat->decoderProgramName());
    mProcess->setReadChannel(QProcess::StandardOutput);
    connect(mProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(readStandardError()));


    mProcess->start(QDir::toNativeSeparators(program), mFormat->decoderArgs(mInputFile));
    bool res = mProcess->waitForStarted();
    if(!res)
    {
        mErrorString = QString("[Decoder] Can't start '%1': %2").arg(program, mProcess->errorString());
        return false;
    }


    try
    {
        mWavHeader.load(mProcess);
        mPos = mWavHeader.dataStartPos();
        return true;
    }
    catch (char const *err)
    {
        mErrorString = err;
        return false;
    }
}

/************************************************
 *
 ************************************************/
void Decoder::close()
{
    if (mFile)
        mFile->close();

    if (mProcess)
    {
        mProcess->terminate();
        mProcess->waitForFinished();
        mProcess->close();
    }
}



/************************************************
 *
 ************************************************/
void mustWrite(const char *buf, qint64 maxSize, QIODevice *outDevice)
{
    qint64 done = 0;
    while (done < maxSize)
    {
        outDevice->waitForBytesWritten(10000);
        qint64 n = outDevice->write(buf + done, maxSize - done);
        if (n < 0)
            throw QString("Can't write %1 bytes. %2").arg(maxSize - done).arg(outDevice->errorString());

        done += n;
    }
}


/************************************************
 *
 ************************************************/
bool Decoder::extract(const CueTime &start, const CueTime &end, QIODevice *outDevice)
{
    try
    {
        emit progress(0);

        QIODevice *input;
        if (mProcess)
            input = mProcess;
        else
            input = mFile;

        mErrorString = "";

        quint64 bs = timeToBytes(start, mWavHeader) + mWavHeader.dataStartPos();
        quint64 be = 0;

        if (end.isNull())
            be = mWavHeader.dataStartPos() + mWavHeader.dataSize();
        else
            be = timeToBytes(end,   mWavHeader) + mWavHeader.dataStartPos();

        outDevice->write(StdWavHeader(be - bs, mWavHeader).toByteArray());

        qint64 pos = mPos;

        // Skip bytes from current to start of track ......
        qint64 len = bs - mPos;
        if (len < 0)
            throw "Incorrect start time.";

        if (!mustSkip(input, len))
            throw "Can't skip to start of track.";

        pos += len;
        // Skip bytes from current to start of track ......

        // Read bytes from start to end of track ..........
        len = be - bs;
        if (len < 0)
            throw "Incorrect start or end time.";

        pos += len;
        qint64 remains = len;
        int percent = 0;

        char buf[MAX_BUF_SIZE];
        while (remains > 0)
        {
            input->bytesAvailable() || input->waitForReadyRead(10000);

            qint64 n = qMin(qint64(MAX_BUF_SIZE), remains);
            n = input->read(buf, n);
            if (n<0)
                throw QString("Can't read %1 bytes").arg(remains);

            remains -= n;

            // Write to OutDevice .........................
            mustWrite(buf, n, outDevice);


            // Calc progrress .............................
            if (remains == 0)
            {
                emit progress(100);
                break;
            }
            else
            {
                int prev = percent;
                percent = (len - remains) * 100.0 / len;
                if (percent != prev)
                {
                    emit progress(percent);
                }
            }
        }
        // Read bytes from start to end of track ..........
        mPos= pos;
        return true;
    }

    catch (QString &err)
    {
        mErrorString = "[Decoder] " + QString(err);
        return false;
    }

    catch (char const *err)
    {
        mErrorString = "[Decoder] " + QString(err);
        return false;
    }
}

/************************************************
 *
 ************************************************/
bool Decoder::extract(const CueTime &start, const CueTime &end, const QString &outFileName)
{
    QFile file(outFileName);
    if (! file.open(QFile::WriteOnly | QFile::Truncate))
    {
        mErrorString = file.errorString();
        return false;
    }

    bool res = extract(start, end, &file);
    file.close();

    return res;
}


/************************************************
 *
 ************************************************/
void Decoder::readStandardError()
{
    mErrBuff += mProcess->readAllStandardError();

    QList<QByteArray> lines = mErrBuff.split('\n');
    int e = (mErrBuff.endsWith('\n')) ? lines.length() : lines.length() - 1;
    for (int i=0 ; i < e; ++i)
    {
        QString s = mFormat->filterDecoderStderr(lines[i]);
        if (!s.isEmpty())
            qWarning("[Decoder] %s", s.toLocal8Bit().data());
    }

    if (!mErrBuff.endsWith('\n'))
        mErrBuff = lines.last();
}
