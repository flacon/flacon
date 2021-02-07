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

#ifndef GAIN_H
#define GAIN_H

#include "worker.h"
#include <QList>
#include "profiles.h"

class Disc;
class Track;

class Gain : public Worker
{
    Q_OBJECT
public:
    explicit Gain(const Profile &profile, QObject *parent = nullptr);

    void addTrack(const Track *track, const QString &file) { mTracks.append({ track, file }); }

    void run() override;

private:
    struct GainTrack
    {
        const Track *track;
        QString      file;
    };

    QVector<GainTrack> mTracks;
    const Profile      mProfile;
};

#endif // GAIN_H
