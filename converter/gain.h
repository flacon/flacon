/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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

#include "converterthread.h"
#include <QList>

class Disk;
class Track;
class OutFormat;
class QProcess;

class Gain: public ConverterThread
{
    Q_OBJECT
public:
    explicit Gain(const OutFormat *format, Disk *disk, Track *track, QObject *parent = 0);
    virtual ~Gain();

    bool isReadyStart() const;

    QList<Track*> tracks() const { return mTracks; }

public slots:
    void inputDataReady(Track *track, const QString &fileName);

protected:
    void doRun();
    void doStop();

private:
    const OutFormat *mFormat;
    QList<Track*> mTracks;
    QHash<Track*, QString> mInputFiles;
    QProcess *mProcess;
    bool mDebug;
};

#endif // GAIN_H
