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
int timeToBytes(CueTime time, WavHeader wav)
{
    if (wav.isCdQuality())
    {
        return (int)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else
    {
        return (int)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}

Decoder::Decoder(const Format &format, QObject *parent) :
    QObject(parent),
    mFormat(format),
    mProcess(NULL),
    mFile(NULL)
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
    if (!mFormat.decoderProgramName().isEmpty())
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

    QString program = settings->programName(mFormat.decoderProgramName());
    mProcess->setReadChannel(QProcess::StandardOutput);
    connect(mProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(readStandardError()));


    mProcess->start(QDir::toNativeSeparators(program), mFormat.decoderArgs(mInputFile));
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
    }
}


/************************************************
 *
 ************************************************/
bool Decoder::extract(const CueTime &start, const CueTime &end, QIODevice *outDevice)
{
    try
    {
        QIODevice *input;
        if (mProcess)
            input = mProcess;
        else
            input = mFile;

        mErrorString = "";

        quint32 bs = timeToBytes(start, mWavHeader) + mWavHeader.dataStartPos();
        quint32 be = timeToBytes(end,   mWavHeader) + mWavHeader.dataStartPos();

        outDevice->write(StdWavHeader(be - bs, mWavHeader).toByteArray());

        int pos = mPos;

        // Skip bytes from current to start of track ......
        int len = bs - mPos;
        if (len < 0)
        {
            mErrorString = "[Decoder] Incorrect start time.";
            return false;
        }

        if (!mustSkip(input, len))
        {
            mErrorString = "[Decoder] Can't skip to start of track.";
            return false;
        }

        pos += len;
        // Skip bytes from current to start of track ......

        // Read bytes from start to end of track ..........
        len = be - bs;
        if (len < 0)
        {
            mErrorString = "[Decoder] Incorrect start or end time.";
            return false;
        }
        pos += len;

        char buf[MAX_BUF_SIZE];
        while (input->bytesAvailable() || input->waitForReadyRead(1000))
        {
            int n = qMin(MAX_BUF_SIZE, len);
            n = input->read(buf, n);
            len -= n;

            if (outDevice->write(buf, n) != n)
            {
                mErrorString = "[Decoder] " + outDevice->errorString();
                return false;
            }

            if (len == 0)
                break;
        }
        // Read bytes from start to end of track ..........

        mPos= pos;
        return true;
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
bool Decoder::extract(const CueTime &start, const CueTime &end, const QString &fileName)
{
    QFile file(fileName);
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
        QString s = mFormat.filterDecoderStderr(lines[i]);
        if (!s.isEmpty())
            qWarning("[Decoder] %s", s.toLocal8Bit().data());
    }

    if (!mErrBuff.endsWith('\n'))
        mErrBuff = lines.last();
}
