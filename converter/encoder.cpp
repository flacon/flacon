/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
 *
 * Copyright: 2012-2013
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


#include "encoder.h"
#include "outformat.h"

#include <QFileInfo>
#include <QDir>
#include <QProcess>

/************************************************

 ************************************************/
Encoder::Encoder(const OutFormat *format, Track *track, QObject *parent):
    ConverterThread(track->disk(), parent),
    mFormat(format),
    mTrack(track),
    mProcess(0),
    mTotal(0),
    mReady(0),
    mProgress(0)
{
    mWorkDir = QFileInfo(track->resultFilePath()).dir().absolutePath();
    mOutFile = track->resultFilePath();
    mReadyStart = false;

    mDebug = QProcessEnvironment::systemEnvironment().contains("FLACON_DEBUG_ENCODER");
}


/************************************************

 ************************************************/
Encoder::~Encoder()
{
}


/************************************************

 ************************************************/
void Encoder::inputDataReady(Track *track, const QString &fileName)
{
    if (track != this->track())
        return;

    mReadyStart = true;
    mInputFile = fileName;

    emit trackProgress(track, Track::Queued);
    emit readyStart();
}


/************************************************

 ************************************************/
void Encoder::doStop()
{
    if (mProcess)
    {
        mProcess->closeReadChannel(QProcess::StandardError);
        mProcess->closeReadChannel(QProcess::StandardOutput);
        mProcess->closeWriteChannel();
        mProcess->terminate();
    }
}


/************************************************

 ************************************************/
void Encoder::run()
{
    ConverterThread::run();
    emit trackReady(mTrack, mOutFile);
}



/************************************************

 ************************************************/
void Encoder::doRun()
{
    {
        QFile f(outFile());

        if (f.exists() && !f.remove())
        {
            error(track(), tr("I can't delete file:\n%1\n%2").arg(f.fileName()).arg(f.errorString()));
            return;
        }
    }


    QStringList args = mFormat->encoderArgs(track(), outFile());
    if (mDebug)
        debugArguments(args);

    QString prog = args.takeFirst();

    mProcess = new QProcess();
    connect(mProcess, SIGNAL(bytesWritten(qint64)), this, SLOT(processBytesWritten(qint64)));

    mProcess->start(prog, args);
    mProcess->waitForStarted();

    readInputFile();
    mProcess->closeWriteChannel();

    mProcess->waitForFinished(-1);
    if (mProcess->exitCode() != 0)
    {
        QString msg = tr("Encoder error:\n") +
                QString::fromLocal8Bit(mProcess->readAllStandardError());
        error(track(), msg);
    }

    QProcess *proc = mProcess;
    mProcess = 0;
    delete proc;

    deleteFile(inputFile());
}

/************************************************

 ************************************************/
void Encoder::readInputFile()
{
    QFile file(mInputFile);
    if (!file.open(QFile::ReadOnly)) // | QFile::Unbuffered))
    {
        error(track(), tr("I can't read %1 file").arg(mInputFile));
    }

    mReady = 0;
    mProgress = -1;
    mTotal = file.size();

    int bufSize = int(mTotal / 200);
    QByteArray buf;

    while (!file.atEnd())
    {
        buf = file.read(bufSize);
        mProcess->write(buf);
    }
}

/************************************************

 ************************************************/
void Encoder::processBytesWritten(qint64 bytes)
{
    mReady += bytes;
    int p = ((mReady * 100.0) / mTotal);
    if (p != mProgress)
    {
        mProgress = p;
        emit trackProgress(track(), Track::Encoding, mProgress);
    }
}
