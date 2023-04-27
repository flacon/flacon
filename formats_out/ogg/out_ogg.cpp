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

#include "out_ogg.h"
#include <QDebug>
#include <math.h>
#include "oggmetadatawriter.h"
#include <QByteArray>

/************************************************

 ************************************************/
OutFormat_Ogg::OutFormat_Ogg()
{
    mId      = "OGG";
    mExt     = "ogg";
    mName    = "OGG";
    mOptions = FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Ogg::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality", true);
    res.insert("Quality", 7);
    res.insert("MinBitrate", "");
    res.insert("NormBitrate", "");
    res.insert("MaxBitrate", "");
    res.insert("ReplayGain", gainTypeToString(GainType::Disable));
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Ogg::configPage(QWidget *parent) const
{
    return new ConfigPage_Ogg(parent);
}

/************************************************

 ************************************************/
ExtProgram *OutFormat_Ogg::encoderProgram(const Profile &) const
{
    return ExtProgram::oggenc();
}

/************************************************

 ************************************************/
QStringList OutFormat_Ogg::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;

    args << "--quiet";

    // Quality settings .........................................
    if (profile.encoderValue("UseQuality").toBool()) {
        args << "-q" << profile.encoderValue("Quality").toString();
    }
    else {
        QString val = profile.encoderValue("NormBitrate").toString();
        if (!val.isEmpty())
            args << "-b" << val;

        val = profile.encoderValue("MinBitrate").toString();
        if (!val.isEmpty())
            args << "-m" << val;

        val = profile.encoderValue("MaxBitrate").toString();
        if (!val.isEmpty())
            args << "-M" << val;
    }

    // Files ....................................................
    args << "-o" << outFile;
    args << "-";
    return args;
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Ogg::createMetadataWriter(const QString &filePath) const
{
    return new OggMetaDataWriter(filePath);
}

/************************************************

 ************************************************/
ConfigPage_Ogg::ConfigPage_Ogg(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    connect(oggUseQualityCheck, &QCheckBox::clicked, this, &ConfigPage_Ogg::setUseQualityMode);

    setLossyToolTip(oggQualitySpin);
    oggQualitySlider->setToolTip(oggQualitySpin->toolTip());
    oggQualityLabel->setToolTip(oggQualitySpin->toolTip());

    connect(oggQualitySlider, &QSlider::valueChanged,
            this, &ConfigPage_Ogg::oggQualitySliderChanged);

    connect(oggQualitySpin, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &ConfigPage_Ogg::oggQualitySpinChanged);

    QList<int> bitrates;
    bitrates << 0 << 64 << 80 << 96 << 128 << 160 << 196 << 256 << 350;
    fillBitrateComboBox(oggMinBitrateCbx, bitrates);
    fillBitrateComboBox(oggNormBitrateCbx, bitrates);
    fillBitrateComboBox(oggMaxBitrateCbx, bitrates);
}

/************************************************

 ************************************************/
void ConfigPage_Ogg::load(const Profile &profile)
{
    loadWidget(profile, "UseQuality", oggUseQualityCheck);
    loadWidget(profile, "Quality", oggQualitySpin);
    loadWidget(profile, "MinBitrate", oggMinBitrateCbx);
    loadWidget(profile, "NormBitrate", oggNormBitrateCbx);
    loadWidget(profile, "MaxBitrate", oggMaxBitrateCbx);

    setUseQualityMode(oggUseQualityCheck->isChecked());
}

/************************************************

 ************************************************/
void ConfigPage_Ogg::save(Profile *profile)
{
    saveWidget(profile, "UseQuality", oggUseQualityCheck);
    saveWidget(profile, "Quality", oggQualitySpin);
    saveWidget(profile, "MinBitrate", oggMinBitrateCbx);
    saveWidget(profile, "NormBitrate", oggNormBitrateCbx);
    saveWidget(profile, "MaxBitrate", oggMaxBitrateCbx);
}

/************************************************

 ************************************************/
void ConfigPage_Ogg::oggQualitySliderChanged(int value)
{
    oggQualitySpin->setValue(value / 10.0);
}

/************************************************

 ************************************************/
void ConfigPage_Ogg::oggQualitySpinChanged(double value)
{
    oggQualitySlider->setValue(value * 10);

    //            -1  0   1   2   3    4    5    6    7    8    9    10   fake
    int    kbps[]  = { 45, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500, 500 };
    int    x1      = int(value);
    double y1      = kbps[x1 + 1];
    double y2      = kbps[x1 + 2];
    int    bitrate = round((y2 - y1) * (value - x1) + y1);
    oggQualityLabel->setText(QString("(~%1 kbps)").arg(bitrate));
}

/************************************************
 *
 ************************************************/
void ConfigPage_Ogg::setUseQualityMode(bool checked)
{
    qualityLabel->setEnabled(checked);
    oggQualitySlider->setEnabled(checked);
    oggQualitySpin->setEnabled(checked);
    oggQualityLabel->setEnabled(checked);

    oggMinBitrateCbx->setEnabled(!checked);
    oggNormBitrateCbx->setEnabled(!checked);
    oggMaxBitrateCbx->setEnabled(!checked);

    oggMinBitrateLabel->setEnabled(!checked);
    oggNormBitrateLabel->setEnabled(!checked);
    oggMaxBitrateLabel->setEnabled(!checked);
}
