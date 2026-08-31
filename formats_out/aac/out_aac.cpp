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

#include "out_aac.h"
#include "inputaudiofile.h"
#include "../metadatawriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

/************************************************

 ************************************************/
OutFormat_Aac::OutFormat_Aac()
{
    mId      = "AAC";
    mExt     = "m4a";
    mName    = "AAC";
    mOptions = FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Aac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality", true);
    res.insert("Quality", 100);
    res.insert("Bitrate", 256);
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Aac::configPage(QWidget *parentr) const
{
    return new ConfigPage_Acc(parentr);
}

/************************************************

 ************************************************/
AVCodecID OutFormat_Aac::avCodecId() const
{
    return AV_CODEC_ID_AAC;
}

/************************************************
 * See https://ffmpeg.org/ffmpeg-codecs.html#aac
 ************************************************/
void OutFormat_Aac::setAvCodecParams(const Profile &profile, AVCodecContext *codecContext) const
{
    if (profile.encoderValues()->value("UseQuality").toBool()) {
        int quality = profile.encoderValues()->value("Quality").toInt();
        av_opt_set_int(codecContext, "global_quality", quality, 0);
    }
    else {
        int bitrate = profile.encoderValues()->value("Bitrate").toInt() * 1024;
        av_opt_set_int(codecContext, "b", bitrate, 0);
    }
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Aac::createMetadataWriter(const Profile &profile, const QString &filePath) const
{
    return new Mp4MetaDataWriter(profile, filePath);
}

/************************************************

 ************************************************/
ConfigPage_Acc::ConfigPage_Acc(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    setLossyToolTip(aacQualitySpin);
    aacQualitySlider->setToolTip(aacQualitySpin->toolTip());
    fillBitrateComboBox(aacBitrateCbx, QList<int>() << 64 << 80 << 128 << 160 << 192 << 224 << 256 << 288 << 320);

    connect(aacUseQualityCheck, &QCheckBox::toggled,
            this, &ConfigPage_Acc::useQualityChecked);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::load(const Profile &profile)
{
    loadWidget(profile, "UseQuality", aacUseQualityCheck);
    loadWidget(profile, "Quality", aacQualitySpin);
    loadWidget(profile, "Bitrate", aacBitrateCbx);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::save(Profile *profile)
{
    saveWidget(profile, "UseQuality", aacUseQualityCheck);
    saveWidget(profile, "Quality", aacQualitySpin);
    saveWidget(profile, "Bitrate", aacBitrateCbx);
}

/************************************************
 *
 ************************************************/
void ConfigPage_Acc::useQualityChecked(bool checked)
{
    qualityLabel->setEnabled(checked);
    aacQualitySlider->setEnabled(checked);
    aacQualitySpin->setEnabled(checked);

    bitrateLabel->setEnabled(!checked);
    aacBitrateCbx->setEnabled(!checked);
}
