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

class Gain : public Worker
{
    Q_OBJECT
public:
    virtual QString programName() const = 0;

    explicit Gain(const Profile &profile, QObject *parent = nullptr);
    void addTrack(const ConvTrack &track, const QString &file);

    void run() override;

    QString programPath() const;

    virtual QStringList programArgs(const QStringList &files, const GainType gainType) const = 0;

private:
    struct Job
    {
        ConvTrack track;
        QString   file;
    };

    Profile    mProfile;
    QList<Job> mJobs;
};

class NoGain : public Gain
{
public:
    using Conv::Gain::Gain;
    QString programName() const override
    {
        return "";
    }

protected:
    QStringList programArgs(const QStringList &, const GainType) const override
    {
        return QStringList();
    }
};

} // namespace
#endif // GAIN_H
