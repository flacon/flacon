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


#include "gain.h"
#include "outformat.h"

#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QDebug>

/************************************************
 *
 ************************************************/
Gain::Gain(const WorkerRequest &request, const OutFormat *format, QObject *parent):
    Worker(parent),
    mFormat(format)
{
    mRequests << request;
}


/************************************************
 *
 ************************************************/
Gain::Gain(const QList<WorkerRequest> &requests, const OutFormat *format, QObject *parent):
    Worker(parent),
    mFormat(format)
{
    mRequests << requests;
}


/************************************************
 *
 ************************************************/
void Gain::run()
{
    bool debug = QProcessEnvironment::systemEnvironment().contains("FLACON_DEBUG_GAIN");

    foreach (WorkerRequest req, mRequests)
        emit trackProgress(req.track(), TrackState::CalcGain, 0);


    QStringList files;
    foreach (WorkerRequest req, mRequests)
        files << QDir::toNativeSeparators(req.inputFile());

    QStringList args = mFormat->gainArgs(files);
    QString prog = args.takeFirst();

    if (debug)
        debugArguments(prog, args);

    QProcess process;

    process.start(prog, args);
    process.waitForFinished(-1);

    if (process.exitCode() != 0)
    {
        QTextStream(stderr) << "Gain command: ";
        debugArguments(prog, args);
        QString msg = tr("Gain error:\n") +
                QString::fromLocal8Bit(process.readAllStandardError());
        error(mRequests.first().track(), msg);
    }

    foreach (WorkerRequest req, mRequests)
    {
        emit trackProgress(req.track(), TrackState::WriteGain, 100);
        emit trackReady(req.track(), req.inputFile());
    }
}
