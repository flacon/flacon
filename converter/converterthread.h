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


#ifndef CONVERTERTHREAD_H
#define CONVERTERTHREAD_H

#include "disk.h"
#include <QThread>
#include <QString>

class ConverterThread: public QThread
{
    Q_OBJECT
public:
    explicit ConverterThread(Disk *disk, QObject *parent = 0);
    virtual ~ConverterThread();

    Disk *disk() const { return mDisk; }
    virtual bool isReadyStart() const = 0;

    virtual QString workDir() const { return ""; }

signals:
    void trackReady(Track *track, const QString &outFileName);
    void trackProgress(Track *track, Track::Status status, int percent = -1);
    void trackError(Track *track, const QString &message);
    void readyStart();

public slots:
    virtual void inputDataReady(Track *track, const QString &fileName) = 0;
    virtual void stop();

protected:
    void run();
    virtual void doRun() = 0;
    virtual void doStop() = 0;
    void error(Track *track, const QString &message);
    bool deleteFile(const QString &fileName);

    void debugArguments(const QString &prog, const QStringList &args);
private:
    Disk *mDisk;
};

#endif // CONVERTERTHREAD_H
