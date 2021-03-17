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
#include "profiles.h"
class QProcess;

namespace Conv {

struct EncoderJob
{
    ConvTrack track;
    QString   inputFile;
    QString   outFile;
    int       bitsPerSample = 0;
    int       sampleRate    = 0;

    EncoderFormat format;
};

class Encoder : public Worker
{
    Q_OBJECT
public:
    Encoder(const EncoderJob &job, QObject *parent = nullptr);

public slots:
    void run() override;

private slots:
    void processBytesWritten(qint64 bytes);

private:
    const EncoderJob mJob;

    quint64 mTotal    = 0;
    quint64 mReady    = 0;
    int     mProgress = 0;

    void readInputFile(QProcess *process);
    void runWav();
    void initResampler(QProcess *process, const QString &outFile, bool debug);
    void check(QProcess *process);
    void runOneProcess(QProcess *process);
    void runTwoProcess(QProcess *resampler, QProcess *encoder);
};

} //namespace
#endif // ENCODER_H
