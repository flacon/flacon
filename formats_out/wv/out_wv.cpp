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

#include "out_wv.h"
#include <QDebug>
#include "wvmetadatawriter.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

static const constexpr char *COMPRESSION_KEY = "Compression";

/************************************************

 ************************************************/
OutFormat_Wv::OutFormat_Wv()
{
    mId      = "WV";
    mExt     = "wv";
    mName    = "WavPack";
    mOptions = FormatOption::Lossless | FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Wv::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert(COMPRESSION_KEY, 1);
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Wv::configPage(QWidget *parent) const
{
    return new ConfigPage_Wv(parent);
}

/**************************************
 *
 **************************************/
AVCodecID OutFormat_Wv::avCodecId() const
{
    return AV_CODEC_ID_WAVPACK;
}

/**************************************
 * https://ffmpeg.org/ffmpeg-codecs.html#wavpack
 **************************************/
void OutFormat_Wv::setAvCodecParams(const Profile &profile, AVCodecContext *codecContext) const
{
    int compression = profile.encoderValues()->value(COMPRESSION_KEY).toInt();
    av_opt_set_int(codecContext, "compression_level", compression, 0);
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Wv::createMetadataWriter(const Profile &profile, const QString &filePath) const
{
    return new WvMetadataWriter(profile, filePath);
}

/************************************************

 ************************************************/
ConfigPage_Wv::ConfigPage_Wv(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    wvCompressionSlider->setMinimum(0);
    wvCompressionSlider->setMaximum(3);
    setLosslessToolTip(wvCompressionSlider);

    initSpinBox(wvCompressionSlider, wvCompressionSpin);
}

/************************************************

 ************************************************/
void ConfigPage_Wv::load(const Profile &profile)
{
    loadWidget(profile, COMPRESSION_KEY, wvCompressionSlider);
}

/************************************************

 ************************************************/
void ConfigPage_Wv::save(Profile *profile)
{
    saveWidget(profile, COMPRESSION_KEY, wvCompressionSlider);
}
