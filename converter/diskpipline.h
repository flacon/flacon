/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#ifndef DISKPIPLINE_H
#define DISKPIPLINE_H

#include <QObject>
#include "track.h"
#include "converterenv.h"

class Disk;

class DiskPipeline : public QObject
{
    Q_OBJECT
public:
    explicit DiskPipeline(const Disk *disk, const ConverterEnv &env,  QObject *parent = 0);
    virtual ~DiskPipeline();

    int startWorker(int *splitterCount, int *count);
    void stop();
    bool isRunning() const;

signals:
    void readyStart();
    void threadFinished();
    void threadQuit();

private slots:
    void trackProgress(const Track *track, int percent);
    void splitterError(const Track *track, const QString &message);
    void splitterTrackReady(const Track *track, const QString &outFileName);

    void encoderTrackReady(const Track *track, const QString &outFileName);

private:
    class Data;
    Data *mData;
};

#endif // DISKPIPLINE_H
