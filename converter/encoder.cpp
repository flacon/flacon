/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
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
#include <QDebug>



Encoder::Encoder(const Track *track, const QString &inFile, const ConverterEnv &env, QObject *parent):
    Worker(parent),
    mTrack(track),
    mProcess(NULL),
    mInputFile(inFile),
    mEnv(env),
    mTotal(0),
    mReady(0),
    mProgress(0)
{
    mOutFile = track->resultFilePath();
}


/************************************************

 ************************************************/
Encoder::~Encoder()
{

}


/************************************************

 ************************************************/
void Encoder::run()
{
    bool debug  = QProcessEnvironment::systemEnvironment().contains("FLACON_DEBUG_ENCODER");

    {
        QFile f(outFile());

        if (f.exists() && !f.remove())
        {
            error(mTrack, tr("I can't delete file:\n%1\n%2").arg(f.fileName()).arg(f.errorString()));
            return;
        }
    }

    // Input file already WAV, so for WAV output format we just rename file.
    if (mEnv.format->id() == "WAV")
    {
        runWav();
        return;
    }

    mProcess = new QProcess();
    QStringList args = mEnv.format->encoderArgs(mTrack, QDir::toNativeSeparators(outFile()));
    QString prog = args.takeFirst();

    if (debug)
        debugArguments(prog, args);

    connect(mProcess, SIGNAL(bytesWritten(qint64)),
            this, SLOT(processBytesWritten(qint64)));

    mProcess->start(QDir::toNativeSeparators(prog), args);
    mProcess->waitForStarted();

    readInputFile();
    mProcess->closeWriteChannel();

    mProcess->waitForFinished(-1);
    if (mProcess->exitCode() != 0)
    {
        debugArguments(prog, args);
        QString msg = tr("QQEncoder error:\n") +
                QString::fromLocal8Bit(mProcess->readAllStandardError());
        error(mTrack, msg);
    }

    deleteFile(mInputFile);

    emit trackReady(mTrack, mOutFile);
    delete mProcess;
    mProcess = NULL;
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
        emit trackProgress(mTrack, mProgress);
    }
}


/************************************************

 ************************************************/
void Encoder::readInputFile()
{
    QFile file(mInputFile);
    if (!file.open(QFile::ReadOnly))
    {
        error(mTrack, tr("I can't read %1 file", "Encoder error. %1 is a file name.").arg(mInputFile));
    }

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
void Encoder::runWav()
{
    QFile srcFile(mInputFile);
    bool res =  srcFile.rename(mOutFile);

    if (!res)
    {
        error(mTrack,
              tr("I can't rename file:\n%1 to %2\n%3").arg(
                  mInputFile,
                  mOutFile,
                  srcFile.errorString()));
    }

    emit trackProgress(mTrack, 100);
    emit trackReady(mTrack, mOutFile);
}
