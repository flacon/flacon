/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#include "decoder.h"
#include "../cue.h"

#include <QIODevice>
#include <QLoggingCategory>
#include <limits>
#include <QFile>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

namespace {
Q_LOGGING_CATEGORY(LOG, "Decoder")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
class FFWavHeader : public WavHeader
{
public:
    FFWavHeader(AVCodecContext *decoder) :
        WavHeader()
    {
        m64Bit         = false;
        mFmtSize       = FmtChunkExt;
        mFormat        = Format_Extensible;
        mNumChannels   = decoder->ch_layout.nb_channels;
        mSampleRate    = decoder->sample_rate;
        mBitsPerSample = av_get_bytes_per_sample(decoder->sample_fmt) * 8;
        mBlockAlign    = (mNumChannels * mBitsPerSample) / 8;
        mByteRate      = mSampleRate * mBlockAlign;

        mValidBitsPerSample = mBitsPerSample;

        mExtSize     = 22;
        mChannelMask = 0;

        mSubFormat = QByteArray("\x00\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71", 16);
        switch (decoder->sample_fmt) {
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP: // IEEE float
                mSubFormat[0] = 03;
                mSubFormat[1] = 00;
                break;

            case AV_SAMPLE_FMT_U8:
            case AV_SAMPLE_FMT_U8P:
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S16P:
            case AV_SAMPLE_FMT_S32:
            case AV_SAMPLE_FMT_S32P:
            case AV_SAMPLE_FMT_S64:
            case AV_SAMPLE_FMT_S64P: // PCM
                mSubFormat[0] = 01;
                mSubFormat[1] = 00;
                break;

            default:
                qCWarning(LOG) << "Unknown sample_fmt:" << decoder->sample_fmt;
                throw FlaconError("The audio file may be corrupted or an unsupported audio format.");
        }

        mDataSize = 0;
        mDataStartPos =
                12 + // header
                4 +  // SubchunkID
                4 +  // SubchunkSize
                mFmtSize;
    }
};

/************************************************
 *
 ************************************************/
QList<Decoder::Format>
Decoder::allFormats()
{
    // clang-format off
    return {
        // name         file ext
        { "APE",        "ape"  },
        { "FLAC",       "flac" },
        { "TTA",        "tta"  },
        { "MP3",        "mp3"  },
        { "WAV",        "wav"  },
        { "WAVE64",     "w64"  },
        { "WavPack",    "wv"   },
    };
    // clang-format on
}

/************************************************
 *
 ************************************************/
QStringList Decoder::allFormatsExts()
{
    QStringList res;
    for (auto fmt : Conv::Decoder::allFormats()) {
        res << QStringLiteral("*.%1").arg(fmt.ext);
    }

    return res;
}

static const int MAX_BUF_SIZE  = 4096;
static const int READ_DELAY    = 1000;
static const int UNKNOWN_COUNT = std::numeric_limits<int>::max();

class RaiiPacketUnref
{
public:
    RaiiPacketUnref(AVPacket *packet) :
        mPacket(packet) { }

    ~RaiiPacketUnref()
    {
        if (mPacket) {
            av_packet_unref(mPacket);
        }
    }

private:
    AVPacket *mPacket = nullptr;
};

class RaiiFrameUnref
{
public:
    RaiiFrameUnref(AVFrame *frame) :
        mFrame(frame) { }

    ~RaiiFrameUnref()
    {
        if (mFrame) {
            av_frame_unref(mFrame);
        }
    }

private:
    AVFrame *mFrame = nullptr;
};

/************************************************
 *
 ************************************************/
static QString ffErrorString(int ffCode)
{
    char buffer[1024];
    av_make_error_string(buffer, 1024, ffCode);
    return QString::fromUtf8(buffer);
}

/************************************************
 *
 ************************************************/
static int64_t timeToBytes(const CueTime &time, const WavHeader &wav)
{
    if (wav.isCdQuality()) {
        return (int64_t)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else {
        return (int64_t)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}

/************************************************
 *
 ************************************************/
Decoder::Decoder(QObject *parent) :
    QObject(parent)
{
}

/************************************************
 *
 ************************************************/
Decoder::~Decoder()
{
    close();
}

/************************************************
 *
 ************************************************/
void Decoder::open(const QString &fileName)
{
    mInputFile = fileName;

    if (!mPacket) {
        mPacket = av_packet_alloc();
    }

    if (!mFrame) {
        mFrame = av_frame_alloc();
    }

    int err = 0;
    err     = avformat_open_input(&mFormatContext, mInputFile.toLocal8Bit(), nullptr, nullptr);
    if (err < 0) {
        qCWarning(LOG) << "Error calling avformat_open_input. err:" << err;
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    err = avformat_find_stream_info(mFormatContext, nullptr);
    if (err < 0) {
        qCWarning(LOG) << "Error calling avformat_find_stream_info. err:" << err;
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    // Find the first audio stream
    mStreamIndex = av_find_best_stream(mFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (mStreamIndex < 0) {
        qCWarning(LOG) << "No audio stream found.";
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    const AVCodec *codec = avcodec_find_decoder(mFormatContext->streams[mStreamIndex]->codecpar->codec_id);
    if (!codec) {
        qCWarning(LOG) << "No input codec found.";
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    mDecoderContext = avcodec_alloc_context3(codec);
    if (!mDecoderContext) {
        qCWarning(LOG) << "Error calling avcodec_alloc_context3. err:";
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    err = avcodec_parameters_to_context(mDecoderContext, mFormatContext->streams[mStreamIndex]->codecpar);
    if (mStreamIndex < 0) {
        qCWarning(LOG) << "Error calling avcodec_parameters_to_context. err:" << err;
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    err = avcodec_open2(mDecoderContext, codec, nullptr);
    if (mStreamIndex < 0) {
        qCWarning(LOG) << "Error calling avcodec_open2. err:" << err;
        throw FlaconError(tr("The audio file may be corrupted or an unsupported audio format.", "Error message."));
    }

    mWavHeader = FFWavHeader(mDecoderContext);

    return;
}

/************************************************
 *
 ************************************************/
void Decoder::close()
{
    if (mDecoderContext) {
        avcodec_free_context(&mDecoderContext);
        mDecoderContext = nullptr;
    }

    if (mFormatContext) {
        avformat_close_input(&mFormatContext);
        mFormatContext = nullptr;
    }

    if (mFrame) {
        av_frame_free(&mFrame);
        mFrame = nullptr;
    }

    if (mPacket) {
        av_packet_free(&mPacket);
        mPacket = nullptr;
    }
}

/************************************************
 *
 ************************************************/
bool Decoder::readFrame()
{
    auto writeFrame = av_sample_fmt_is_planar(mDecoderContext->sample_fmt) ? writePlanarFrame : writeInterleavedFrame;

    while (true) {
        [[maybe_unused]] RaiiPacketUnref raiPacketUnref(mPacket);

        int err = av_read_frame(mFormatContext, mPacket);
        if (err == AVERROR_EOF) {
            return false;
        }

        if (err < 0) {
            qCWarning(LOG) << QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos) << ": " << ffErrorString(err);
            throw FlaconError(QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos));
        }

        if (mPacket->stream_index != mStreamIndex) {
            continue;
        }

        err = avcodec_send_packet(mDecoderContext, mPacket);
        if (err < 0) {
            qCWarning(LOG) << QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos) << ": " << ffErrorString(err);
            throw FlaconError(QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos));
        }

        while (true) {
            [[maybe_unused]] RaiiFrameUnref raiPacketUnref(mFrame);

            err = avcodec_receive_frame(mDecoderContext, mFrame);
            if (err == AVERROR_EOF) {
                break;
            }

            if (err == AVERROR(EAGAIN)) {
                break;
            }

            if (err < 0) {
                qCWarning(LOG) << QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos) << ": " << ffErrorString(err);
                throw FlaconError(QStringLiteral("Decoding error after %1 samples.").arg(mDecoderPos));
            }

            mDecoderPos += writeFrame(mFrame, &mFrameBuff);
        }
        return true;
    }
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::extract(const CueTime &startTime, const CueTime &endTime, QIODevice *outDevice)
{
    int     percent    = 0;
    int64_t aproxTotal = aproximateBytes(endTime);
    emit    progress(0);

    auto updateProgrss = [&](int done) {
        int prev = percent;
        percent  = std::min(100.0, double(done) * 100.0 / aproxTotal);
        if (percent != prev) {
            emit progress(percent);
        }
    };

    uint64_t res = 0;

    int skip = timeToBytes(startTime, mWavHeader) - (mDecoderPos - mFrameBuff.size());
    int rest = (!endTime.isNull()) ? timeToBytes(endTime, mWavHeader) - (mDecoderPos - mFrameBuff.size()) - skip : UNKNOWN_COUNT;

    while (rest > 0) {
        if (mFrameBuff.isEmpty()) {
            if (!readFrame()) {
                break; // EOF
            }
        }

        if (skip > mFrameBuff.size()) {
            skip -= mFrameBuff.size();
            updateProgrss(aproxTotal - skip - rest);
            mFrameBuff.clear();
            continue;
        }

        int count  = rest < mFrameBuff.size() ? rest - skip : mFrameBuff.size() - skip;
        int cnt    = outDevice->write(mFrameBuff.mid(skip, count));
        mFrameBuff = mFrameBuff.mid(skip + count);

        if (cnt != count) {
            throw FlaconError(QStringLiteral("Can't write %1 bytes. %2").arg(res).arg(outDevice->errorString()));
        }
        rest -= count;
        res += count;
        skip = 0;
        updateProgrss(aproxTotal - rest);
    }

    if (!endTime.isNull()) {
        uint64_t bs = timeToBytes(startTime, mWavHeader);
        uint64_t be = timeToBytes(endTime, mWavHeader);

        if (res != (be - bs)) {
            throw FlaconError("The decoder returned an incorrect number of bytes.");
        }
    }

    emit progress(100);
    return res;
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::extract(const CueTime &startTime, const CueTime &endTime, const QString &outFileName)
{
    QFile file(outFileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        throw FlaconError(tr("I can't write file <b>%1</b>:<br>%2",
                             "Error string, %1 is a filename, %2 error message")
                                  .arg(file.fileName())
                                  .arg(file.errorString()));

    uint64_t res = extract(startTime, endTime, &file);
    file.close();
    return res;
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::writeInterleavedFrame(AVFrame *frame, QByteArray *buf)
{
    int     sampleSize = av_get_bytes_per_sample((AVSampleFormat)frame->format);
    int     channels   = frame->ch_layout.nb_channels;
    int64_t size       = frame->nb_samples * channels * sampleSize;

    buf->append(reinterpret_cast<char *>(frame->data[0]), size);
    return size;
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::writePlanarFrame(AVFrame *frame, QByteArray *buf)
{
    int     sampleSize = av_get_bytes_per_sample((AVSampleFormat)frame->format);
    int     channels   = frame->ch_layout.nb_channels;
    int64_t size       = frame->nb_samples * channels * sampleSize;
    buf->reserve(buf->size() + size);

    uint64_t res = 0;
    for (int i = 0; i < frame->nb_samples; ++i) {
        for (int ch = 0; ch < channels; ++ch) {
            buf->append(reinterpret_cast<char *>(frame->data[ch] + i * sampleSize), sampleSize);
        }
    }

    return res;
}

/************************************************
 *
 ************************************************/
uint64_t Decoder::aproximateBytes(const CueTime &endTime) const
{
    if (!endTime.isNull()) {
        return timeToBytes(endTime, mWavHeader) - mDecoderPos;
    }

    int64_t end = (((duration() * (double)mWavHeader.byteRate()) / 1000.0) + 0.5);
    return end - mDecoderPos;
}

/************************************************
 *
 ************************************************/
int64_t Decoder::bytesCount(const CueTime &startTime, const CueTime &endTime) const
{
    if (endTime.isNull()) {
        return 0;
    }

    return timeToBytes(endTime, mWavHeader) - timeToBytes(startTime, mWavHeader);
}

/************************************************
 *
 ************************************************/
mSec Decoder::duration() const
{
    return mFormatContext ? mFormatContext->duration / 1000 : 0;
}

/************************************************
 *
 ************************************************/
QString Decoder::formatName() const
{
    return mDecoderContext ? mDecoderContext->codec->name : "";
}

/************************************************
 *
 ************************************************/
AVCodecID Decoder::formatId() const
{
    return mDecoderContext ? mDecoderContext->codec->id : AV_CODEC_ID_NONE;
}
