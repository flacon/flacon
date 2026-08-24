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
    dst->channel_layout = src->channel_layout;
    dst->channels       = src->channels;
    return 0;
#endif
}

/**************************************
 *
 **************************************/
inline void describeChannelLayout(const AVCodecContext *ctx, char *buf, size_t buf_size)
{
#if HAS_AV_CHANNEL_LAYOUT
    av_channel_layout_describe(&ctx->ch_layout, buf, buf_size);
#else
    av_get_channel_layout_string(buf, static_cast<int>(buf_size), ctx->channels, ctx->channel_layout);
#endif
}
} // namespace
#endif // AVCOMPAT_H
