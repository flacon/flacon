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
#include "profiles.h"

#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "Converter")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
Gain::Gain(const GainJob &job, QObject *parent) :
    Gain(GainJobs() << job, parent)
{
}

/************************************************
 *
 ************************************************/
Gain::Gain(const GainJobs &jobs, QObject *parent) :
    Worker(parent),
    mJobs(jobs)
{
}

/************************************************
 *
 ************************************************/
void Gain::run()
{
    QStringList files;
    for (const GainJob &job : mJobs) {
        emit trackProgress(job.track(), TrackState::CalcGain, 0);
        files << QDir::toNativeSeparators(job.file());
    }

    QStringList args = mJobs.first().format().gainArgs(files);
    QString     prog = args.takeFirst();

    qCDebug(LOG) << "Start gain:" << debugProgramArgs(prog, args);

    QProcess process;

    process.start(prog, args);
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        qWarning() << "Gain command failed: " << debugProgramArgs(prog, args);
        QString msg = tr("Gain error:\n") + QString::fromLocal8Bit(process.readAllStandardError());
        emit    error(mJobs.first().track(), msg);
    }

    for (const GainJob &job : mJobs) {
        emit trackProgress(job.track(), TrackState::WriteGain, 100);
        emit trackReady(job.track(), job.file());
    }
}
