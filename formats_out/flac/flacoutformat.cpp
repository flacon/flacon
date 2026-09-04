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

#include "flacoutformat.h"
#include "project.h"
#include "inputaudiofile.h"
#include "flacmetadatawriter.h"
#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

/************************************************
 *
 ************************************************/
OutFormat_Flac::OutFormat_Flac()
{
    mId      = "FLAC";
    mExt     = "flac";
    mName    = "FLAC";
    mOptions = FormatOption::Lossless | FormatOption::SupportGain | FormatOption::SupportEmbeddedCue | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Flac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Compression", 5);
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Flac::configPage(QWidget *parent) const
{
    return new ConfigPage_Flac(parent);
}

/************************************************

 ************************************************/
AVCodecID OutFormat_Flac::avCodecId() const
{
    return AV_CODEC_ID_FLAC;
}

/**************************************
 * See https://ffmpeg.org/ffmpeg-codecs.html#flac-2
 **************************************/
void OutFormat_Flac::setAvCodecParams(const Profile &profile, AVCodecContext *codecContext) const
{
    int compression = profile.encoderValues()->value("Compression").toInt();
    av_opt_set_int(codecContext, "compression_level", compression, 0);
}

/************************************************

************************************************/
MetadataWriter *OutFormat_Flac::createMetadataWriter(const Profile &profile, const QString &filePath) const
{
    return new FlacMetadataWriter(profile, filePath);
}

/************************************************

 ************************************************/
ConfigPage_Flac::ConfigPage_Flac(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    // See https://ffmpeg.org/ffmpeg-codecs.html#flac-2
    flacCompressionSlider->setMinimum(0);
    flacCompressionSlider->setMaximum(12);
    setLosslessToolTip(flacCompressionSlider);

    initSpinBox(flacCompressionSlider, flacCompressionSpin);
}

/************************************************

 ************************************************/
void ConfigPage_Flac::load(const Profile &profile)
{
    loadWidget(profile, "Compression", flacCompressionSlider);
}

/************************************************

 ************************************************/
void ConfigPage_Flac::save(Profile *profile)
{
    saveWidget(profile, "Compression", flacCompressionSlider);
}
