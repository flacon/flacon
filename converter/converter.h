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


#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QDateTime>
#include "disk.h"

class OutFormat;
class ConverterThread;
class Disk;
class Track;

class Converter : public QObject
{
    Q_OBJECT
public:
    explicit Converter(QObject *parent = 0);

    bool isRunning();
    bool canConvert() const;

    bool showStatistic() const { return mShowStatistic; }
    void setShowStatistic(bool value);

signals:
    void finished();

public slots:
    void start();
    void stop();

private slots:
    void threadError(Track *track, const QString &message);
    void startThread();
    void threadFinished();
    void trackReady(Track *track, const QString &outFileName);
    void setTrackProgress(Track *track, Track::Status status, int percent);

private:
    QDateTime mStartTime;
    int mThreadCount;
    QList<ConverterThread*> mThreads;
    bool mShowStatistic;

    bool check(OutFormat *format) const;
    void createDiscThreads(Disk *disk, const OutFormat *format);
    void createTrackThreads(Track *track, const OutFormat *format, ConverterThread *prevThread, ConverterThread *nextThread);
    bool createDirs();
};

#endif // CONVERTER_H
