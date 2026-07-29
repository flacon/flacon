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

#include "fdkaacconfigpage.h"
#include "ui_fdkaacconfigpage.h"

/************************************************

 ************************************************/
FdkaacConfigPage::FdkaacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::FdkaacConfigPage)
{
    ui->setupUi(this);

    initQualitySlider();
    initCbrBitrateComboBox();

    connect(ui->useVbrCheckBox, &QCheckBox::clicked, this, &FdkaacConfigPage::refresh);

    refresh();
}

/************************************************

 ************************************************/
FdkaacConfigPage::~FdkaacConfigPage()
{
    delete ui;
}

/************************************************

 ************************************************/
bool FdkaacConfigPage::useVbr() const
{
    return ui->useVbrCheckBox->isChecked();
}

/************************************************

 ************************************************/
int FdkaacConfigPage::vbrQuality() const
{
    return ui->qualitySlider->value();
}

/************************************************

 ************************************************/
int FdkaacConfigPage::cbrBitrate() const
{
    return ui->cbrBitrateComboBox->currentData().toInt();
}

/************************************************

 ************************************************/
void FdkaacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "Fdkaac/UseVBR", ui->useVbrCheckBox);
    loadWidget(profile, "Fdkaac/Quality", ui->qualitySlider);
    loadWidget(profile, "Fdkaac/Bitrate", ui->cbrBitrateComboBox);
    refresh();
}

/************************************************

 ************************************************/
void FdkaacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "Fdkaac/UseVBR", ui->useVbrCheckBox);
    saveWidget(profile, "Fdkaac/Quality", ui->qualitySlider);
    saveWidget(profile, "Fdkaac/Bitrate", ui->cbrBitrateComboBox);
}

/************************************************

 ************************************************/
void FdkaacConfigPage::initQualitySlider()
{
    QLabel   *label   = ui->qualityLabel;
    QSlider  *slider  = ui->qualitySlider;
    QSpinBox *spinBox = ui->qualitySpin;

    label->setBuddy(slider);

    slider->setMinimum(0);
    slider->setMaximum(5);
    slider->setSingleStep(1);
    slider->setPageStep(1);
    slider->setTracking(true);
    slider->setTickPosition(QSlider::TicksAbove);
    slider->setTickInterval(1);

    spinBox->setMinimum(slider->minimum());
    spinBox->setMaximum(slider->maximum());

    connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), slider, &QSlider::setValue);

    slider->setToolTip(tr("VBR (higher value -> higher bitrate)"));
    spinBox->setToolTip(slider->toolTip());
}

void FdkaacConfigPage::initCbrBitrateComboBox()
{
    QLabel    *label    = ui->cbrBitrateLabel;
    QComboBox *comboBox = ui->cbrBitrateComboBox;

    fillBitrateComboBox(comboBox, { 64, 80, 128, 160, 192, 224, 256, 288, 320 });

    label->setBuddy(comboBox);

    comboBox->setToolTip(tr("Target bitrate (for CBR)"));
}

/************************************************

 ************************************************/
void FdkaacConfigPage::refresh()
{
    ui->qualityLabel->setEnabled(useVbr());
    ui->qualitySlider->setEnabled(useVbr());
    ui->qualitySpin->setEnabled(useVbr());

    ui->cbrBitrateComboBox->setEnabled(!useVbr());
    ui->cbrBitrateLabel->setEnabled(ui->cbrBitrateComboBox->isEnabled());
}
