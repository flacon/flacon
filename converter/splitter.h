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

#include "outformat.h"
#include "converterthread.h"

class Disk;
class Track;
class QProcess;

class Splitter: public ConverterThread
{
    Q_OBJECT
public:
    Splitter(Disk *disk, QObject *parent = 0);

    bool isReadyStart() const;
    QString workDir() const { return mWorkDir; }

public slots:
    void inputDataReady(Track *track, const QString &fileName);

protected:
    void doRun();
    void doStop();

private slots:
    void decoderProgress(double percent);

private:
    QString mWorkDir;
    QString mFilePrefix;
    QProcess *mProcess;
    bool mPreGapExists;

    Track *mTrack;
};

#endif // SPLITTER_H
