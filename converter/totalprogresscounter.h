#ifndef TOTALPROGRESSCOUNTER_H
#define TOTALPROGRESSCOUNTER_H

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

#include <QObject>
#include <QMap>
#include "track.h"
#include "disc.h"

namespace Conv {
class Converter;
}

class TotalProgressCounter : public QObject
{
    Q_OBJECT
public:
    explicit TotalProgressCounter(QObject *parent = nullptr);

    void init(const Conv::Converter &converter);
    void setTrackProgress(const Track &track, TrackState state, int percent);

    double result() const { return mResult; }

signals:
    void changed(double percent);

private:
    struct TrackData
    {
        Percent  percent  = 0;
        Duration duration = 0;
    };

    using Key = std::pair<Disk *, TrackNum>;
    QMap<Key, TrackData> mTracks;

    Duration mTotalDuration = 0;
    double   mResult        = 0.0;
};

#endif // TOTALPROGRESSCOUNTER_H
