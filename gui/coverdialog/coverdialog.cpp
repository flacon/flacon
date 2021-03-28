/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#include "coverdialog.h"
#include "ui_coverdialog.h"

#include <QListWidget>
#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>

#include "disc.h"
#include "asynclistwidgetitem.h"

#define FileNameRole (Qt::UserRole + 1)

/************************************************
 *
 ************************************************/
CoverDialog *CoverDialog::createAndShow(Disc *disc, QWidget *parent)
{
    CoverDialog *instance = parent->findChild<CoverDialog *>();

    if (!instance)
        instance = new CoverDialog(parent);

    instance->setAttribute(Qt::WA_DeleteOnClose);
    instance->setDisc(disc);
    instance->show();
    instance->raise();
    instance->activateWindow();

    return instance;
}

/************************************************
 *
 ************************************************/
CoverDialog::CoverDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoverDialog),
    mDisc(nullptr)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Without cover image"));

    mEmptyIcon = QIcon(QPixmap::fromImage(QImage(":noCover").scaled(ui->coverView->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    connect(ui->coverView, &QListWidget::itemDoubleClicked,
            this, &CoverDialog::coverDoubleClicked);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &CoverDialog::buttonClicked);
}

/************************************************
 *
 ************************************************/
CoverDialog::~CoverDialog()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
void CoverDialog::setDisc(Disc *disc)
{
    mDisc = disc;
    connect(mDisc.data(), &QObject::destroyed,
            this, &CoverDialog::close);

    ui->coverView->clear();
    scan(QFileInfo(disc->cueFilePath()).absoluteDir().absolutePath());
    ui->coverView->setGridSize(QSize(140, 160));
}

/************************************************
 *
 ************************************************/
void CoverDialog::coverDoubleClicked(QListWidgetItem *)
{
    buttonClicked(ui->buttonBox->button(QDialogButtonBox::Ok));
}

/************************************************
 *
 ************************************************/
void CoverDialog::buttonClicked(QAbstractButton *button)
{
    if (!mDisc) {
        this->reject();
        return;
    }

    if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        QListWidgetItem *item = ui->coverView->currentItem();
        if (!item)
            return;
        mDisc->setCoverImageFile(item->data(FileNameRole).toString());
        this->accept();
        return;
    }

    if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
        mDisc->setCoverImageFile("");
        this->accept();
        return;
    }
}

/************************************************
 *
 ************************************************/
void CoverDialog::scan(const QString &startDir)
{
    QString curFile = mDisc->coverImageFile();
    foreach (QString f, mDisc->searchCoverImages(startDir)) {
        QFileInfo            file(f);
        AsyncListWidgetItem *item = new AsyncListWidgetItem(ui->coverView);
        item->setText(file.baseName());
        item->setData(FileNameRole, file.absoluteFilePath());
        item->setIcon(mEmptyIcon);
        item->setIconAsync(file.absoluteFilePath());

        if (file.absoluteFilePath() == curFile)
            ui->coverView->setCurrentItem(item);
    }
}
