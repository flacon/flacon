#ifndef ENCODER_H
#define ENCODER_H

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

#include "worker.h"
#include "../profiles.h"
#include "coverimage.h"
#include "replaygain.h"

struct AVCodecContext;
struct AVFormatContext;
struct AVFilterGraph;
struct AVFilterContext;
struct AVStream;
struct AVCodec;

namespace Conv {

class Encoder : public Worker
{
    Q_OBJECT
public:
    explicit Encoder(QObject *parent = nullptr);
    ~Encoder();

    const Profile   &profile() const { return mProfile; }
    const ConvTrack &track() const { return mTrack; }
    QString          outFile() const { return mOutFile; }
    QString          inputFile() const { return mInputFile; }
    const QString   &embeddedCue() const { return mEmbeddedCue; }

    void setProfile(const Profile &profile) { mProfile = profile; };
    void setTrack(const ConvTrack &track) { mTrack = track; }
    void setInputFile(const QString &value) { mInputFile = value; }
    void setOutFile(const QString &value) { mOutFile = value; }
    void setEmbeddedCue(const QString &value) { mEmbeddedCue = value; }

    const CoverImage &coverImage() const { return mCoverImage; }
    void              setCoverImage(const CoverImage &value) { mCoverImage = value; }

    void run() override;

signals:
    void trackReady(const Conv::ConvTrack &track, const QString &outFileName, const ReplayGain::Result &trackGain);

private:
    Profile    mProfile;
    ConvTrack  mTrack;
    QString    mInputFile;
    QString    mOutFile;
    QString    mEmbeddedCue;
    CoverImage mCoverImage;

    bool                  mReplayGainEnabled = false;
    ReplayGain::TrackGain mTrackGain;

    void writeMetadata() const;

private:
    AVFormatContext *mInFmtCtx = nullptr;
    AVCodecContext  *mDecCtx   = nullptr;

    AVCodecContext  *mEncCtx    = nullptr;
    AVFormatContext *mOutFmtCtx = nullptr;

    AVFilterGraph   *mFilterGraph = nullptr;
    AVFilterContext *mFiltSrcCtx  = nullptr;
    AVFilterContext *mFiltSinkCtx = nullptr;

    AVStream *mInStream  = nullptr;
    AVStream *mOutStream = nullptr;

    int mAudioStreamIdx = -1;

    void setupInput();
    void setupEncoder(AVCodecID formatId, int bitsPerSample, int sampleRate);
    void setupOutput();
    void setupFilterGraph(bool deemph);
    void encode();

    int selectBestSampleRate(const AVCodec *codec, int preferredRate) const;
};

} // namespace
#endif // ENCODER_H
