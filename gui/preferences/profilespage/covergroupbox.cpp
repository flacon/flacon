/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#include "covergroupbox.h"
#include "ui_covergroupbox.h"
#include <QDebug>

CoverGroupBox::CoverGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::CoverGroupBox)
{
    ui->setupUi(this);
    this->setChecked(false);
    ui->keepSizeButton->setChecked(mKeepSize);
    ui->scaleButton->setChecked(!mKeepSize);

    int h = 0;
    for (int r = 0; r < ui->coverImageLayout->rowCount(); ++r) {
        h = qMax(h, ui->coverImageLayout->cellRect(r, 0).height());
    }

    for (int r = 0; r < ui->coverImageLayout->rowCount(); ++r) {
        ui->coverImageLayout->setRowMinimumHeight(r, h);
    }

    connect(ui->scaleButton, &QRadioButton::toggled, this, &CoverGroupBox::refresh);
    connect(this, &QGroupBox::toggled, this, &CoverGroupBox::refresh);
}

CoverGroupBox::~CoverGroupBox()
{
    delete ui;
}

CoverOptions CoverGroupBox::coverOptions() const
{
    CoverOptions res;

    if (this->isChecked()) {
        res.mode = (ui->scaleButton->isChecked()) ? CoverMode::Scale : CoverMode::OrigSize;
    }

    res.size = ui->resizeSpinBox->value();
    return res;
}

void CoverGroupBox::setCoverOptions(const CoverOptions &value)
{
    if (value.mode != CoverMode::Disable) {
        mKeepSize = (value.mode == CoverMode::OrigSize);
    }

    this->setChecked(value.mode != CoverMode::Disable);
    ui->keepSizeButton->setChecked(mKeepSize);
    ui->scaleButton->setChecked(!mKeepSize);

    ui->resizeSpinBox->setValue(value.size);
}

void CoverGroupBox::refresh()
{
    ui->resizeSpinBox->setEnabled(ui->scaleButton->isChecked() && this->isChecked());
}
