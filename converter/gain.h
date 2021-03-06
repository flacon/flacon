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

namespace Conv {

class GainJob
{
public:
    GainJob(const ConvTrack &track, const QString &file, const GainOptions &options) :
        mTrack(track),
        mFile(file),
        mOptions(options)
    {
    }

    GainJob(const GainJob &other) = default;

    ConvTrack   track() const { return mTrack; }
    QString     file() const { return mFile; }
    GainOptions options() const { return mOptions; }

private:
    ConvTrack   mTrack;
    QString     mFile;
    GainOptions mOptions;
};

using GainJobs = QList<GainJob>;

class Gain : public Worker
{
    Q_OBJECT
public:
    explicit Gain(const GainJob &job, QObject *parent = nullptr);
    explicit Gain(const GainJobs &jobs, QObject *parent = nullptr);

    void run() override;

private:
    QList<GainJob> mJobs;
};

} // namespace
#endif // GAIN_H
