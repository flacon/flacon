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
#include "../profiles.h"
class QProcess;

namespace Conv {

class Encoder : public Worker
{
    Q_OBJECT
public:
    explicit Encoder(QObject *parent = nullptr);

    const OutFormat *outFormat() const { return mProfile.outFormat(); }

    const Profile   &profile() const { return mProfile; }
    const ConvTrack &track() const { return mTrack; }
    QString          outFile() const { return mOutFile; }
    QString          inputFile() const { return mInputFile; }
    QString          coverFile() const { return mCoverFile; }

    void setProfile(const Profile &profile);
    void setTrack(const ConvTrack &track) { mTrack = track; }
    void setInputFile(const QString &value) { mInputFile = value; }
    void setOutFile(const QString &value) { mOutFile = value; }
    void setCoverFile(const QString &value) { mCoverFile = value; }

    virtual QString     encoderProgramName() const { return ""; }
    virtual QStringList encoderArgs() const = 0;

public slots:
    void run() override;

protected:
    int bitsPerSample(const InputAudioFile &audio) const;
    int sampleRate(const InputAudioFile &audio) const;

private slots:
    void processBytesWritten(qint64 bytes);

private:
    Profile   mProfile;
    ConvTrack mTrack;
    QString   mInputFile;
    QString   mOutFile;
    QString   mCoverFile;

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

} // namespace
#endif // ENCODER_H
