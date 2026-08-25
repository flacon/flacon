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

#ifndef AVCOMPAT_H
#define AVCOMPAT_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>
}

namespace AvCompat {

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 24, 100)
#define HAS_AV_CHANNEL_LAYOUT 1
#else
#define HAS_AV_CHANNEL_LAYOUT 0
#endif

/**************************************
 *
 **************************************/
inline int getChannelsNum(const AVCodecContext *ctx)
{
#if HAS_AV_CHANNEL_LAYOUT
    return ctx->ch_layout.nb_channels;
#else
    return ctx->channels;
#endif
}

/**************************************
 *
 **************************************/
inline int getChannelsNum(const AVFrame *frame)
{
#if HAS_AV_CHANNEL_LAYOUT
    return frame->ch_layout.nb_channels;
#else
    return frame->channels;
#endif
}

/**************************************
 *
 **************************************/
inline int copyChannelLayout(AVCodecContext *dst, const AVCodecContext *src)
{
#if HAS_AV_CHANNEL_LAYOUT
    return av_channel_layout_copy(&dst->ch_layout, &src->ch_layout);
#else
    uint64_t layout = src->channel_layout;
    if (!layout) {
        layout = av_get_default_channel_layout(src->channels);
    }
    dst->channel_layout = layout;
    dst->channels       = src->channels;
    return 0;
#endif
}

/**************************************
 * https://ffmpeg.org/ffmpeg-utils.html#channel-layout-syntax
 **************************************/
inline void describeChannelLayout(const AVCodecContext *ctx, char *buf, size_t buf_size)
{
#if HAS_AV_CHANNEL_LAYOUT
    uint64_t mask = 0;

    if (ctx->ch_layout.order == AV_CHANNEL_ORDER_NATIVE && ctx->ch_layout.u.mask != 0) {
        mask = ctx->ch_layout.u.mask;
    }
    else {
        AVChannelLayout defLayout;
        av_channel_layout_default(&defLayout, ctx->ch_layout.nb_channels);

        if (defLayout.order == AV_CHANNEL_ORDER_NATIVE) {
            mask = defLayout.u.mask;
        }
        av_channel_layout_uninit(&defLayout);
    }

    if (mask != 0) {
        snprintf(buf, buf_size, "0x%" PRIx64, mask);
    }
    else {
        av_channel_layout_describe(&ctx->ch_layout, buf, buf_size);
    }
#else
    uint64_t layout = ctx->channel_layout;
    if (!layout) {
        layout = av_get_default_channel_layout(ctx->channels);
    }

    snprintf(buf, buf_size, "0x%" PRIx64, layout);
#endif
}
} // namespace
#endif // AVCOMPAT_H
