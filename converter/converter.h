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
class DiskPipeline;

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
    void startThread();

private:
    QDateTime mStartTime;
    int mThreadCount;
    QVector<DiskPipeline*> mDiskPiplines;

    bool mShowStatistic;

    bool check(OutFormat *format) const;
    bool createDirs();
};

#endif // CONVERTER_H
