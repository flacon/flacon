/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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

#include "consoleout.h"
#include <QTextStream>

/************************************************
 *
 ************************************************/
ConsoleOut::ConsoleOut(const Profile &profile, QObject *parent) :
    QObject(parent),
    mProfile(profile)
{
}

/************************************************
 *
 ************************************************/
void ConsoleOut::converterStarted()
{
    mStartTime = QDateTime::currentDateTime();
}

/************************************************
 *
 ************************************************/
void ConsoleOut::converterFinished()
{
    mFinishTime = QDateTime::currentDateTime();
}

/************************************************
 *
 ************************************************/
void ConsoleOut::trackProgress(const Track &track, TrackState state, Percent)
{
    QString status;
    switch (state) {
        case TrackState::Canceled:
            status = "Canceled";
            break;
        case TrackState::Error:
            status = "Error";
            break;
        case TrackState::Aborted:
            status = "Aborted";
            break;
        case TrackState::OK:
            status = "Done";
            break;
        default:
            return;
    }

    QTextStream(stdout)
            << status << " "
            << mProfile.resultFilePath(&track) << "\n";
}

/************************************************
 *
 ************************************************/
void ConsoleOut::printStatistic()
{
    int duration = mFinishTime.toSecsSinceEpoch() - mStartTime.toSecsSinceEpoch();
    if (!duration)
        duration = 1;

    int h = duration / 3600;
    int m = (duration - (h * 3600)) / 60;
    int s = duration - (h * 3600) - (m * 60);

    QString str;

    if (h)
        str = QStringLiteral("Encoding time %4h %3m %2s [%1 sec]").arg(duration).arg(s).arg(m).arg(h);
    else if (m)
        str = QStringLiteral("Encoding time %3m %2s [%1 sec]").arg(duration).arg(s).arg(m);
    else
        str = QStringLiteral("Encoding time %1 sec").arg(duration);

    QTextStream(stdout) << str << "\n";
}
