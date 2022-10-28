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
#include "coverimage.h"
#include "replaygain.h"

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
    const QString   &embeddedCue() const { return mEmbeddedCue; }

    void setProfile(const Profile &profile);
    void setTrack(const ConvTrack &track) { mTrack = track; }
    void setInputFile(const QString &value) { mInputFile = value; }
    void setOutFile(const QString &value) { mOutFile = value; }
    void setEmbeddedCue(const QString &value) { mEmbeddedCue = value; }

    const CoverImage &coverImage() const { return mCoverImage; }
    void              setCoverImage(const CoverImage &value);

    virtual QString     programName() const { return ""; }
    virtual QStringList programArgs() const = 0;

public slots:
    void run() override;

signals:
    void trackReady(const Conv::ConvTrack &track, const QString &outFileName, const ReplayGain::Result &trackGain);

protected:
    QString programPath() const;

private slots:
    void processBytesWritten(qint64 bytes);

private:
    Profile   mProfile;
    ConvTrack mTrack;
    QString   mInputFile;
    QString   mOutFile;
    QString   mEmbeddedCue;

    CoverImage mCoverImage;

    bool                  mReplayGainEnabled = false;
    ReplayGain::TrackGain mTrackGain;

    quint64 mTotal    = 0;
    quint64 mReady    = 0;
    int     mProgress = 0;

    void readInputFile(QProcess *process);
    void copyFile();

    QProcess *createEncoderProcess();
    QProcess *createRasmpler(const QString &outFile);
    QProcess *createDemph(const QString &outFile);
    void      writeMetadata() const;
};

} // namespace
#endif // ENCODER_H
