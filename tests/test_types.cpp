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

#include <QTest>
#include "flacontest.h"
#include "tools.h"

#include <QDebug>

char *toString(const DiskState &state)
{
    // clang-format off
    switch (state) {
        case DiskState::NotRunning: return qstrdup("NotRunning");
        case DiskState::Canceled:   return qstrdup("Canceled");
        case DiskState::Error:      return qstrdup("Error");
        case DiskState::Aborted:    return qstrdup("Aborted");
        case DiskState::OK:         return qstrdup("OK");
        case DiskState::Running:    return qstrdup("Running");
    }
    // clang-format on

    return qstrdup(QString("Unknown %1").arg(int(state)).toLocal8Bit().constData());
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCalcDiskState()
{
    {
        QList<TrackState> tracks;
        QCOMPARE(calcDiskState(tracks), DiskState::NotRunning);
    }

    {
        QList<TrackState> tracks;
        tracks << TrackState::OK;
        tracks << TrackState::OK;
        QCOMPARE(calcDiskState(tracks), DiskState::OK);
    }

    {
        QList<TrackState> tracks;
        tracks << TrackState::OK;
        tracks << TrackState::Canceled;
        QCOMPARE(calcDiskState(tracks), DiskState::Canceled);
    }

    {
        QList<TrackState> tracks;
        tracks << TrackState::OK;
        tracks << TrackState::Aborted;
        tracks << TrackState::Canceled;
        QCOMPARE(calcDiskState(tracks), DiskState::Canceled);
    }

    {
        QList<TrackState> tracks;
        tracks << TrackState::OK;
        tracks << TrackState::Aborted;
        QCOMPARE(calcDiskState(tracks), DiskState::Aborted);
    }
}
