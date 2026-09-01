/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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

#include "alacoutformat.h"
#include "alacconfigpage.h"
#include "../metadatawriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

/************************************************
 *
 ************************************************/
OutFormat_Alac::OutFormat_Alac()
{
    mId      = "ALAC";
    mExt     = "m4a";
    mName    = "ALAC";
    mOptions = FormatOption::Lossless | FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************
 *
 ************************************************/
QHash<QString, QVariant> OutFormat_Alac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Compression", 0);
    return res;
}

/************************************************
 *
 ************************************************/
EncoderConfigPage *OutFormat_Alac::configPage(QWidget *parent) const
{
    return new AlacConfigPage(parent);
}

/************************************************
 *
 ************************************************/
AVCodecID OutFormat_Alac::avCodecId() const
{
    return AV_CODEC_ID_ALAC;
}

/************************************************
 *
 ************************************************/
void OutFormat_Alac::setAvCodecParams(const Profile &profile, AVCodecContext *codecContext) const
{
    int compression = profile.encoderValues()->value("Compression").toInt();
    av_opt_set_int(codecContext, "compression_level", compression, 0);
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Alac::createMetadataWriter(const Profile &profile, const QString &filePath) const
{
    return new Mp4MetaDataWriter(profile, filePath);
}
