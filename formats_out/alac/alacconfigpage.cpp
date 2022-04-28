/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include "alacconfigpage.h"
#include "ui_alacconfigpage.h"

AlacConfigPage::AlacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::AlacConfigPage)
{
    ui->setupUi(this);

    setLosslessToolTip(ui->compressionSlider);
    ui->compressionSpin->setToolTip(ui->compressionSlider->toolTip());
}

AlacConfigPage::~AlacConfigPage()
{
    delete ui;
}

void AlacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "Compression", ui->compressionSlider);
}

void AlacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "Compression", ui->compressionSlider);
}
