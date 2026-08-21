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

#include "encoder.h"
#include <QString>
#include <QCoreApplication>
#include "formats_out/metadatawriter.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(LOG, "Encoder")
}

using namespace Conv;

namespace {
/**************************************
 *
 **************************************/
QString ffErrorStr(int errNum, const QString &contextMessage)
{
    char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    av_strerror(errNum, errBuf, sizeof(errBuf));
    return QString("%1 (%2)").arg(contextMessage, QString::fromUtf8(errBuf));
}

/**************************************
 *
 **************************************/
AVSampleFormat selectBestSampleFormat(const AVCodec *encoder, int reqBps)
{
    const AVSampleFormat *supportedFmts = nullptr;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 3, 100)
    int numFmts = 0;
    avcodec_get_supported_config(nullptr, encoder, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0,
                                 reinterpret_cast<const void **>(&supportedFmts), &numFmts);
#else
    supportedFmts = encoder->sample_fmts;
#endif

    // If the encoder accepts absolutely any format (rare, but it happens in PCM)
    if (!supportedFmts) {
        return (reqBps > 16) ? AV_SAMPLE_FMT_S32 : AV_SAMPLE_FMT_S16;
    }

    QList<AVSampleFormat> preferred;

    if (reqBps > 16) {
        // We look for 32-bit int, then float, then planar versions
        preferred << AV_SAMPLE_FMT_S32 << AV_SAMPLE_FMT_S32P
                  << AV_SAMPLE_FMT_FLT << AV_SAMPLE_FMT_FLTP
                  << AV_SAMPLE_FMT_S16 << AV_SAMPLE_FMT_S16P;
    }
    else {
        // We look for a 16-bit int, then a planar, then the rest
        preferred << AV_SAMPLE_FMT_S16 << AV_SAMPLE_FMT_S16P
                  << AV_SAMPLE_FMT_FLT << AV_SAMPLE_FMT_FLTP
                  << AV_SAMPLE_FMT_S32 << AV_SAMPLE_FMT_S32P;
    }

    for (AVSampleFormat fmt : preferred) {
        for (const AVSampleFormat *p = supportedFmts; *p != -1; ++p) {
            if (*p == fmt) {
                return fmt;
            }
        }
    }

    return supportedFmts[0];
}

}

/**************************************
 *
 **************************************/
// void configureEncoderParams(AVCodecContext *encCtx) const
// {
//     switch (encCtx->codec_id) {
//         case AV_CODEC_ID_FLAC: {
//             // int quality = mProfile.outFormat()->quality(mProfile); // 0-8 или 0-12
//             int quality = 10;
//             av_opt_set_int(encCtx->priv_data, "compression_level", quality, 0);
//             break;
//         }
//         case AV_CODEC_ID_MP3: {
//             // e.g., VBR vs CBR, quality, bitrate
//             // av_opt_set_int(encCtx->priv_data, "compression_level", quality, 0);
//             // encCtx->bit_rate = ...
//             break;
//         }
//         default:
//             break;
//     }
// }

/************************************************

 ************************************************/
Encoder::Encoder(QObject *parent) :
    Worker(parent)
{
}

/************************************************

 ************************************************/
Encoder::~Encoder()
{
    // clang-format off
    if (mFilterGraph) avfilter_graph_free(&mFilterGraph);
    if (mDecCtx)      avcodec_free_context(&mDecCtx);
    if (mEncCtx)      avcodec_free_context(&mEncCtx);
    if (mInFmtCtx)    avformat_close_input(&mInFmtCtx);

    if (mOutFmtCtx) {
        if (!(mOutFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&mOutFmtCtx->pb);
        }
        avformat_free_context(mOutFmtCtx);
    };
    // clang-format on
}

/************************************************

 ************************************************/
void Conv::Encoder::run()
{
    mReplayGainEnabled = mProfile.gainType() != GainType::Disable;

    emit trackProgress(track(), TrackState::Encoding, 0);

    try {

        const InputAudioFile &audio = mTrack.audioFile();

        AVCodecID formatId = AV_CODEC_ID_FLAC;

        int  bitsPerSample = calcQuality(audio.bitsPerSample(), mProfile.bitsPerSample(), mProfile.outFormat()->maxBitPerSample());
        int  sampleRate    = calcQuality(audio.sampleRate(), mProfile.sampleRate(), mProfile.outFormat()->maxSampleRate());
        bool deemph        = false;

        if (mTrack.preEmphased()) {
            // sample rate must be 44100 (audio-CD) or 48000 (DAT)
            int rate = mTrack.audioFile().sampleRate();
            if (rate == 44100 || rate == 48000) {
                deemph = true;
            }
            else {
                qCDebug(LOG) << "DeEmphasis disabled, sample rate must be 44100 (audio-CD) or 48000 (DAT)";
            }
        }

        setupInput();
        setupEncoder(formatId, bitsPerSample, sampleRate);
        setupOutput();
        setupFilterGraph(deemph);
        encode();

        deleteFile(mInputFile);

        writeMetadata();

        emit trackProgress(track(), TrackState::Encoding, 100);
        emit trackReady(track(), outFile(), mTrackGain.result());
    }

    catch (const Abort &) {
        deleteFile(mInputFile);
        deleteFile(mOutFile);
        qCDebug(LOG) << "FFEncoder job was aborted on track" << track().trackNumTag();
    }

    catch (const FlaconError &err) {
        deleteFile(mInputFile);
        deleteFile(mOutFile);

        QString msg = tr("Track %1. Encoder error:", "Track error message, %1 is a track number")
                              .arg(track().trackNumTag())
                + "<pre>" + err.what() + "</pre>";
        emit error(track(), msg);
    }
}

void Encoder::writeMetadata() const
{
    MetadataWriter *writer = mProfile.outFormat()->createMetadataWriter(mProfile, outFile());
    if (!writer) {
        return;
    }

    writer->setTags(mTrack);
    if (profile().isEmbedCue()) {
        writer->setEmbeddedCue(embeddedCue());
    }

    if (!coverImage().isEmpty()) {
        writer->setCoverImage(coverImage());
    }

    writer->save();
    delete writer;
}

/**************************************
 *
 **************************************/
void Encoder::setupInput()
{
    int ret = avformat_open_input(&mInFmtCtx, mInputFile.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, tr("Cannot open input audio file: %1").arg(mInputFile)));
    }

    ret = avformat_find_stream_info(mInFmtCtx, nullptr);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, tr("Cannot find stream information in input file.")));
    }

    mAudioStreamIdx = av_find_best_stream(mInFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (mAudioStreamIdx < 0) {
        throw FlaconError(tr("No audio stream found in input file."));
    }

    mInStream              = mInFmtCtx->streams[mAudioStreamIdx];
    const AVCodec *decoder = avcodec_find_decoder(mInStream->codecpar->codec_id);
    if (!decoder) {
        throw FlaconError(tr("Unsupported input audio codec."));
    }

    mDecCtx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(mDecCtx, mInStream->codecpar);
    if (avcodec_open2(mDecCtx, decoder, nullptr) < 0) {
        throw FlaconError(tr("Could not open input audio decoder."));
    }
}

/**************************************
 *
 **************************************/
void Encoder::setupEncoder(AVCodecID formatId, int bitsPerSample, int sampleRate)
{

    // const AVCodec *encoder = avcodec_find_encoder_by_name("flac");
    const AVCodec *encoder = avcodec_find_encoder(formatId);

    if (!encoder) {
        throw FlaconError(QString("Encoder for %1 is not available in system libavcodec.").arg(formatId));
    }

    mEncCtx = avcodec_alloc_context3(encoder);

    mEncCtx->sample_rate = sampleRate;
    av_channel_layout_copy(&mEncCtx->ch_layout, &mDecCtx->ch_layout);

    mEncCtx->sample_fmt = selectBestSampleFormat(encoder, bitsPerSample);

    const AVCodecDescriptor *desc = avcodec_descriptor_get(formatId);
    if (desc && (desc->props & AV_CODEC_PROP_LOSSLESS)) {
        mEncCtx->bits_per_raw_sample = bitsPerSample;
    }

    mEncCtx->time_base.num = 1;
    mEncCtx->time_base.den = mEncCtx->sample_rate;

    // configureEncoderParams .......
    // ..............................

    if (avcodec_open2(mEncCtx, encoder, nullptr) < 0) {
        throw FlaconError(tr("Failed to open audio encoder context."));
    }
}

/**************************************
 *
 **************************************/
void Encoder::setupOutput()
{
    int ret = avformat_alloc_output_context2(&mOutFmtCtx, nullptr, nullptr, mOutFile.toUtf8().constData());
    if (ret < 0 || !mOutFmtCtx) {
        throw FlaconError(QString("Could not create output format context for %1").arg(mOutFile));
    }

    mOutStream = avformat_new_stream(mOutFmtCtx, nullptr);
    avcodec_parameters_from_context(mOutStream->codecpar, mEncCtx);
    mOutStream->time_base = mEncCtx->time_base;

    if (!(mOutFmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&mOutFmtCtx->pb, mOutFile.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            throw FlaconError(ffErrorStr(ret, tr("Cannot open output file %1 for writing").arg(mOutFile)));
        }
    }

    ret = avformat_write_header(mOutFmtCtx, nullptr);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, "Failed to write header to output file."));
    }
}

/**************************************
 *
 **************************************/
void Encoder::setupFilterGraph(bool deemph)
{
    int ret      = 0;
    mFilterGraph = avfilter_graph_alloc();
    if (!mFilterGraph) {
        throw FlaconError(ffErrorStr(AVERROR(ENOMEM), "Failed to configure audio filter graph."));
    }

    const AVFilter *abuffer     = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    AVRational timeBase = mInStream->time_base.num > 0 ? mInStream->time_base : AVRational { 1, mDecCtx->sample_rate };

    char chLayoutStr[64] = { 0 };
    av_channel_layout_describe(&mDecCtx->ch_layout, chLayoutStr, sizeof(chLayoutStr));

    QString srcArgs = QString("sample_rate=%1:sample_fmt=%2:time_base=%3/%4:channel_layout='%5'")
                              .arg(mDecCtx->sample_rate)
                              .arg(av_get_sample_fmt_name(mDecCtx->sample_fmt))
                              .arg(timeBase.num)
                              .arg(timeBase.den)
                              .arg(chLayoutStr);

    ret = avfilter_graph_create_filter(&mFiltSrcCtx, abuffer, "in", srcArgs.toUtf8().constData(), nullptr, mFilterGraph);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, "Failed to configure audio filter graph."));
    }

    ret = avfilter_graph_create_filter(&mFiltSinkCtx, abuffersink, "out", nullptr, nullptr, mFilterGraph);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, "Failed to configure audio filter graph."));
    }

    // We construct a filter chain: [deemph] -> [aformat]
    QStringList filters;

    if (deemph) {
        filters << QString("deemph=sample_rate=%1").arg(mDecCtx->sample_rate);
    }

    // Force all three output parameters (sample_fmts, sample_rates, channel_layouts).
    // The aformat filter will automatically insert aresample if necessary.
    char outChLayoutStr[64] = { 0 };
    av_channel_layout_describe(&mEncCtx->ch_layout, outChLayoutStr, sizeof(outChLayoutStr));

    filters << QString("aformat=sample_fmts=%1:sample_rates=%2:channel_layouts='%3'")
                       .arg(av_get_sample_fmt_name(mEncCtx->sample_fmt))
                       .arg(mEncCtx->sample_rate)
                       .arg(outChLayoutStr);

    QString filterSpec = filters.join(",");

    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVFilterInOut *outputs = avfilter_inout_alloc();

    outputs->name       = av_strdup("in");
    outputs->filter_ctx = mFiltSrcCtx;
    outputs->pad_idx    = 0;
    outputs->next       = nullptr;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = mFiltSinkCtx;
    inputs->pad_idx    = 0;
    inputs->next       = nullptr;

    ret = avfilter_graph_parse_ptr(mFilterGraph, filterSpec.toUtf8().constData(), &inputs, &outputs, nullptr);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, "Failed to configure audio filter graph."));
    }

    ret = avfilter_graph_config(mFilterGraph, nullptr);
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, "Failed to configure audio filter graph."));
    }

    if (mEncCtx->frame_size > 0) {
        av_buffersink_set_frame_size(mFiltSinkCtx, mEncCtx->frame_size);
    }
}

/**************************************
 *
 **************************************/
void Encoder::encode()
{
    AVPacket *inPacket      = av_packet_alloc();
    AVPacket *outPacket     = av_packet_alloc();
    AVFrame  *decodedFrame  = av_frame_alloc();
    AVFrame  *filteredFrame = av_frame_alloc();

    auto cleanup = qScopeGuard([&]() {
        av_packet_free(&inPacket);
        av_packet_free(&outPacket);
        av_frame_free(&decodedFrame);
        av_frame_free(&filteredFrame);
    });

    auto sendFrameToEncoder = [&](AVFrame *frame) {
        int eRet = avcodec_send_frame(mEncCtx, frame);
        if (eRet < 0) {
            throw FlaconError(ffErrorStr(eRet, tr("Error sending frame to encoder.")));
        }

        while (eRet >= 0) {
            eRet = avcodec_receive_packet(mEncCtx, outPacket);
            if (eRet == AVERROR(EAGAIN) || eRet == AVERROR_EOF)
                break;

            if (eRet < 0) {
                throw FlaconError(ffErrorStr(eRet, "Error encoding audio frame."));
            }

            av_packet_rescale_ts(outPacket, mEncCtx->time_base, mOutStream->time_base);
            outPacket->stream_index = mOutStream->index;

            eRet = av_interleaved_write_frame(mOutFmtCtx, outPacket);
            if (eRet < 0) {
                throw FlaconError(ffErrorStr(eRet, "Error writing encoded packet to disk."));
            }
            av_packet_unref(outPacket);
        }
    };

    auto processFilterSink = [&]() {
        while (true) {
            int fRet = av_buffersink_get_frame(mFiltSinkCtx, filteredFrame);
            if (fRet == AVERROR(EAGAIN) || fRet == AVERROR_EOF) {
                break;
            }

            if (fRet < 0) {
                throw FlaconError(ffErrorStr(fRet, tr("Error pulling frame from filter graph.")));
            }

            sendFrameToEncoder(filteredFrame);
            av_frame_unref(filteredFrame);
        }
    };

    int64_t totalSamples     = (mInFmtCtx->duration * mDecCtx->sample_rate) / AV_TIME_BASE;
    int64_t processedSamples = 0;
    int     lastProgress     = 0;

    while (av_read_frame(mInFmtCtx, inPacket) >= 0) {
        Abort::check();

        if (inPacket->stream_index == mAudioStreamIdx) {
            int ret = avcodec_send_packet(mDecCtx, inPacket);
            if (ret < 0)
                throw FlaconError(ffErrorStr(ret, "Error decoding audio packet."));

            while (ret >= 0) {
                ret = avcodec_receive_frame(mDecCtx, decodedFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }

                if (ret < 0) {
                    throw FlaconError(ffErrorStr(ret, tr("Error decoding audio frame.")));
                }

                // ReplayGain ..............
                if (mReplayGainEnabled && decodedFrame->nb_samples > 0) {
                    int bytesPerSample = av_get_bytes_per_sample(mDecCtx->sample_fmt);
                    int dataSize       = decodedFrame->nb_samples * mDecCtx->ch_layout.nb_channels * bytesPerSample;
                    mTrackGain.add(reinterpret_cast<const char *>(decodedFrame->data[0]), dataSize);
                }

                // Progress ................
                processedSamples += decodedFrame->nb_samples;
                if (totalSamples > 0) {
                    int p = static_cast<int>((processedSamples * 100) / totalSamples);
                    if (p != lastProgress) {
                        lastProgress = p;
                        emit trackProgress(track(), TrackState::Encoding, lastProgress);
                    }
                }

                // Resampling / DeEmphasis
                ret = av_buffersrc_add_frame_flags(mFiltSrcCtx, decodedFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
                if (ret < 0) {
                    throw FlaconError(ffErrorStr(ret, "Error feeding filter graph."));
                }

                processFilterSink();
                av_frame_unref(decodedFrame);
            }
        }
        av_packet_unref(inPacket);
    }

    // Flushing the decoder, filter, and encoder
    avcodec_send_packet(mDecCtx, nullptr);
    while (avcodec_receive_frame(mDecCtx, decodedFrame) >= 0) {
        int ret = av_buffersrc_add_frame_flags(mFiltSrcCtx, decodedFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
        if (ret < 0) {
            throw FlaconError(ffErrorStr(ret, "Error feeding audio frame to filter graph."));
        }
        processFilterSink();
        av_frame_unref(decodedFrame);
    }

    int ret = av_buffersrc_add_frame_flags(mFiltSrcCtx, nullptr, 0); // Flush filter
    if (ret < 0) {
        throw FlaconError(ffErrorStr(ret, tr("Error flushing filter graph.")));
    }
    processFilterSink();

    sendFrameToEncoder(nullptr); // Flush encoder

    av_write_trailer(mOutFmtCtx);
}
