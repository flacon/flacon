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

#include <QIODevice>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "Decoder")
}

using namespace Conv;

static const int MAX_BUF_SIZE = 4096;
static const int READ_DELAY   = 1000;

/************************************************
 *
 ************************************************/
static qint64 timeToBytes(const CueTime &time, const WavHeader &wav)
{
    if (wav.isCdQuality()) {
        return (qint64)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else {
        return (qint64)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}

/************************************************
 *
 ************************************************/
Decoder::Decoder(QObject *parent) :
    QObject(parent),
    mFormat(nullptr),
    mProcess(nullptr),
    mFile(nullptr),
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
void Decoder::open(const QString &fileName)
{
    mInputFile = fileName;
    if (!mFormat) {
        mFormat = InputFormat::formatForFile(fileName);
    }

    if (!mFormat) {
        qCWarning(LOG) << "The audio file may be corrupted: Unknown InputFormat";
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    if (mFormat->decoderProgram()) {
        return openProcess();
    }
    else {
        return openFile();
    }
}

/************************************************
 *
 ************************************************/
void Decoder::openFile()
{
    mFile = new QFile(mInputFile, this);
    if (!mFile->open(QFile::ReadOnly)) {
        throw FlaconError(mFile->errorString());
    }

    try {
        mWavHeader = WavHeader(mFile);
        mPos       = mWavHeader.dataStartPos();
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "The audio file may be corrupted:" << err.what();
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }
}

/************************************************
 *
 ************************************************/
void Decoder::openProcess()
{
    ExtProgram *program = mFormat->decoderProgram();

    if (program->path().isEmpty()) {
        throw FlaconError(tr("The %1 program is not installed.<br>Verify that all required programs are installed and in your preferences.",
                             "Error message. %1 - is an program name")
                                  .arg(program->name()));
    }

    if (!QFileInfo::exists(program->path())) {
        throw FlaconError(tr("The %1 program is installed according to your settings, but the binary file can’t be found.<br>"
                             "Verify that all required programs are installed and in your preferences.",
                             "Error message. %1 - is an program name")
                                  .arg(program->name()));
    }
    mProcess = new QProcess(this);
    mProcess->setReadChannel(QProcess::StandardOutput);

    mProcess->start(QDir::toNativeSeparators(program->path()), mFormat->decoderArgs(mInputFile));
    bool res = mProcess->waitForStarted();
    if (!res) {
        throw FlaconError(QString("Can't start '%1': %2")
                                  .arg(program->path(), mProcess->errorString()));
    }

    try {
        mWavHeader = WavHeader(mProcess);
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "The audio file may be corrupted:" << err.what();
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }
    mPos = mWavHeader.dataStartPos();
}

/************************************************
 *
 ************************************************/
void Decoder::close()
{
    if (mFile)
        mFile->close();

    if (mProcess) {
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
    while (done < maxSize) {
        outDevice->waitForBytesWritten(10000);
        qint64 n = outDevice->write(buf + done, maxSize - done);
        if (n < 0)
            throw FlaconError(QString("Can't write %1 bytes. %2")
                                      .arg(maxSize - done)
                                      .arg(outDevice->errorString()));

        done += n;
    }
}

/************************************************
 *
 ************************************************/
bool mustSkip(QIODevice *device, qint64 size, int msecs = READ_DELAY)
{
    static const int BUF_SIZE = 4096;

    if (size == 0)
        return true;

    char   buf[BUF_SIZE];
    qint64 left = size;
    while (left > 0) {
        device->bytesAvailable() || device->waitForReadyRead(msecs);
        qint64 n = device->read(buf, qMin(qint64(BUF_SIZE), left));
        if (n < 0)
            return false;

        left -= n;
    }

    return true;
}

/************************************************
 *
 ************************************************/
void Decoder::extract(const CueTime &start, const CueTime &end, QIODevice *outDevice, bool writeHeader)
{
    try {
        emit progress(0);

        QIODevice *input;
        if (mProcess)
            input = mProcess;
        else
            input = mFile;

        quint64 bs = timeToBytes(start, mWavHeader) + mWavHeader.dataStartPos();
        quint64 be = 0;

        if (end.isNull())
            be = mWavHeader.dataStartPos() + mWavHeader.dataSize();
        else
            be = timeToBytes(end, mWavHeader) + mWavHeader.dataStartPos();

        if (writeHeader) {
            WavHeader hdr = mWavHeader;
            hdr.resizeData(be - bs);
            outDevice->write(hdr.toLegacyWav());
        }

        qint64 pos = mPos;

        // Skip bytes from current to start of track ......
        qint64 len = bs - mPos;
        if (len < 0)
            throw FlaconError("Incorrect start time.");

        if (!mustSkip(input, len))
            throw FlaconError("Can't skip to start of track.");

        pos += len;
        // Skip bytes from current to start of track ......

        // Read bytes from start to end of track ..........
        len = be - bs;
        if (len < 0)
            throw FlaconError("Incorrect start or end time.");

        pos += len;
        qint64 remains = len;
        int    percent = 0;

        char buf[MAX_BUF_SIZE];
        while (remains > 0) {
            input->bytesAvailable() || input->waitForReadyRead(10000);

            qint64 n = qMin(qint64(MAX_BUF_SIZE), remains);
            n        = input->read(buf, n);
            if (n < 0)
                throw FlaconError(QString("Can't read %1 bytes").arg(remains));

            remains -= n;

            // Write to OutDevice .........................
            mustWrite(buf, n, outDevice);

            // Calc progrress .............................
            if (remains == 0) {
                emit progress(100);
                break;
            }
            else {
                int prev = percent;
                percent  = (len - remains) * 100.0 / len;
                if (percent != prev) {
                    emit progress(percent);
                }
            }
        }
        // Read bytes from start to end of track ..........
        mPos = pos;
    }
    catch (FlaconError &err) {
        close();
        throw err;
    }
}

/************************************************
 *
 ************************************************/
void Decoder::extract(const CueTime &start, const CueTime &end, const QString &outFileName)
{
    QFile file(outFileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        throw FlaconError(tr("I can't write file <b>%1</b>:<br>%2",
                             "Error string, %1 is a filename, %2 error message")
                                  .arg(file.fileName())
                                  .arg(file.errorString()));

    extract(start, end, &file);
    file.close();
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::bytesCount(const CueTime &start, const CueTime &end) const
{

    quint64 bs = timeToBytes(start, mWavHeader) + mWavHeader.dataStartPos();
    quint64 be = 0;

    if (end.isNull()) {
        be = mWavHeader.dataStartPos() + mWavHeader.dataSize();
    }
    else {
        be = timeToBytes(end, mWavHeader) + mWavHeader.dataStartPos();
    }

    return be - bs;
}
