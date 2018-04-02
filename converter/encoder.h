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

#include <QProcess>

#include "worker.h"

class QProcess;
class OutFormat;

class Encoder: public Worker
{
    Q_OBJECT
public:
    Encoder(const WorkerRequest request, const OutFormat *format, QObject *parent = 0);

    QString outFile() const { return mOutFile; }

public slots:
    void run() override;

signals:


private slots:
    void processBytesWritten(qint64 bytes);


private:
    const WorkerRequest mRequest;
    const OutFormat *mFormat;
    QString mOutFile;
    quint64 mTotal;
    quint64 mReady;
    int mProgress;

    void readInputFile(QProcess *process);
    void runWav();
};

#endif // ENCODER_H
