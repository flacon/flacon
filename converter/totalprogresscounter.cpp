/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include "totalprogresscounter.h"
#include "converter.h"
#include "discpipline.h"

using namespace Conv;

/************************************************
 *
 ************************************************/
TotalProgressCounter::TotalProgressCounter(QObject *parent) :
    QObject(parent)
{
}

/************************************************
 *
 ************************************************/
void TotalProgressCounter::init(const Conv::Converter &converter)
{
    mTotalDuration = 0.0;
    mResult        = 0.0;

    for (const DiscPipeline *pipline : converter.diskPiplines()) {
        for (const Track &track : pipline->tracks()) {
            TrackData data;

            data.duration = track.duration();
            mTotalDuration += data.duration;

            mTracks.insert(Key(track.disc(), track.index()), data);
        }
    }
}

/************************************************
 *
 ************************************************/
void TotalProgressCounter::setTrackProgress(const Track &track, TrackState state, int percent)
{
    if (state != TrackState::Encoding) {
        return;
    }

    mTracks[Key(track.disc(), track.index())].percent = percent;

    Duration done = 0;
    for (const TrackData &data : mTracks) {
        done += data.duration * data.percent;
    }

    double prev = mResult;
    mResult     = double(done) / mTotalDuration;

    if (int(prev * 10) != int(mResult * 10)) {
        emit changed(mResult);
    }
}
