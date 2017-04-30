#ifndef ENCODER_H
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


#define ENCODER_H

#include "converterthread.h"

class OutFormat;
class QProcess;

class Encoder: public ConverterThread
{
    Q_OBJECT
public:
    explicit Encoder(const OutFormat *format, Track *track, QObject *parent = 0);
    virtual ~Encoder();

    QString workDir() const { return mWorkDir; }
    Track *track() const  { return mTrack; }
    QString inputFile() const { return mInputFile; }
    QString outFile() const { return mOutFile; }

    bool isReadyStart() const { return mReadyStart; }

public slots:
    void inputDataReady(Track *track, const QString &fileName);

protected:
    void run();
    void doRun();
    void doStop();

private slots:
    void processBytesWritten(qint64 bytes);

private:
    Track *mTrack;
    QString mWorkDir;
    QString mInputFile;
    QString mOutFile;
    bool mReadyStart;
    QProcess *mProcess;

    int mTotal;
    int mReady;
    int mProgress;
    bool mDebug;

    void readInputFile();
    void readInputFile2(FILE *proc);
};

#endif // ENCODER_H
