/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2026
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

#include "faacconfigpage.h"
#include "ui_faacconfigpage.h"

/************************************************

 ************************************************/
FaacConfigPage::FaacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::FaacConfigPage)
{
    ui->setupUi(this);

    setLossyToolTip(ui->aacQualitySpin);
    ui->aacQualitySlider->setToolTip(ui->aacQualitySpin->toolTip());
    fillBitrateComboBox(ui->aacBitrateCbx, QList<int>() << 64 << 80 << 128 << 160 << 192 << 224 << 256 << 288 << 320);

    connect(ui->aacUseQualityCheck, &QCheckBox::toggled, this, &FaacConfigPage::useQualityChecked);
}

/************************************************

 ************************************************/
FaacConfigPage::~FaacConfigPage()
{
    delete ui;
}

/************************************************

 ************************************************/
void FaacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "UseQuality", ui->aacUseQualityCheck);
    loadWidget(profile, "Quality", ui->aacQualitySpin);
    loadWidget(profile, "Bitrate", ui->aacBitrateCbx);
}

/************************************************

 ************************************************/
void FaacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "UseQuality", ui->aacUseQualityCheck);
    saveWidget(profile, "Quality", ui->aacQualitySpin);
    saveWidget(profile, "Bitrate", ui->aacBitrateCbx);
}

/************************************************

 ************************************************/
void FaacConfigPage::useQualityChecked(bool checked)
{
    ui->qualityLabel->setEnabled(checked);
    ui->aacQualitySlider->setEnabled(checked);
    ui->aacQualitySpin->setEnabled(checked);

    ui->bitrateLabel->setEnabled(!checked);
    ui->aacBitrateCbx->setEnabled(!checked);
}
