/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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


#include "addprofiledialog.h"
#include "ui_addprofiledialog.h"
#include "outformat.h"
#include <QPushButton>

/************************************************
 *
 ************************************************/
AddProfileDialog::AddProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddProfileDialog)
{
    ui->setupUi(this);

    for (const auto format: OutFormat::allFormats()) {
        ui->formatCbx->addItem(format->name(), format->id());
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)
            ->setText(tr("Create a profile", "Button caption"));

    connect(ui->profileNameEd, &QLineEdit::textChanged,
            this, &AddProfileDialog::setButtonsEnabled);

    setButtonsEnabled();
}


/************************************************
 *
 ************************************************/
AddProfileDialog::~AddProfileDialog()
{
    delete ui;
}


/************************************************
 *
 ************************************************/
const QString AddProfileDialog::profileName() const
{
    return ui->profileNameEd->text();
}


/************************************************
 *
 ************************************************/
void AddProfileDialog::setProfileName(const QString &value)
{
    ui->profileNameEd->setText(value);
}


/************************************************
 *
 ************************************************/
const QString AddProfileDialog::formaiId() const
{
    return ui->formatCbx->currentData().toString();
}


/************************************************
 *
 ************************************************/
void AddProfileDialog::setFormatId(const QString &value)
{
    int n = ui->formatCbx->findData(value);
    ui->formatCbx->setCurrentIndex(qMax(0, n));
}


/************************************************
 *
 ************************************************/
void AddProfileDialog::setButtonsEnabled()
{
    auto btn = ui->buttonBox->button(QDialogButtonBox::Ok);
    btn->setEnabled(!ui->profileNameEd->text().isEmpty());
}
