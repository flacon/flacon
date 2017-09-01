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


#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "track.h"

class OutFormat;
class Disk;



class WorkerRequest {
public:
    WorkerRequest(const Track *track, const QString &fileName):
        mTrack(track),
        mFileName(fileName)
    {
    }

    const Track* track() const { return mTrack; }
    QString fileName() const { return mFileName; }

private:
    const Track *mTrack;
    QString mFileName;
};


class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);
    virtual ~Worker();
    virtual void run() = 0;

signals:
    void trackReady(const Track *track, const QString &outFileName);
    void trackProgress(const Track *track, Track::Status status, int percent);
    void error(const Track *track, const QString &message);
    void progress(const Track *track, int percent);

protected:
    bool createDir(const QString &dirName) const;
    bool deleteFile(const QString &fileName) const;

    void debugArguments(const QString &prog, const QStringList &args);
};


#endif // WORKER_H
