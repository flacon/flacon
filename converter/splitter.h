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

#ifndef SPLITTER_H
#define SPLITTER_H

#include "convertertypes.h"
#include "worker.h"
#include "profiles.h"

namespace Conv {

class Splitter : public Worker
{
    Q_OBJECT
public:
    Splitter(Disc *disk, const ConvTracks &tracks, const QString &outDir, QObject *parent = nullptr);

    PreGapType pregapType() const { return mPregapType; }
    void       setPregapType(const PreGapType &pregapType);

public slots:
    void run() override;

signals:
    void trackReady(const Conv::ConvTrack &track, const QString &outFileName);

private:
    struct Job;

    const Disc      *mDisc = nullptr;
    const ConvTracks mTracks;
    const QString    mOutDir;
    PreGapType       mPregapType = PreGapType::AddToFirstTrack;

    void processTrack(const Job &job);
};

} // namespace
#endif // SPLITTER_H
