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

#include "disk.h"
#include "asynclistwidgetitem.h"


#define FileNameRole (Qt::UserRole + 1)

/************************************************
 *
 ************************************************/
CoverDialog *CoverDialog::createAndShow(Disk *disk, QWidget *parent)
{
    CoverDialog *instance = parent->findChild<CoverDialog*>();

    if (!instance)
        instance = new CoverDialog(parent);

    instance->setAttribute(Qt::WA_DeleteOnClose);
    instance->setDisk(disk);
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
    mDisk(nullptr)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Without cover image"));

    mEmptyIcon = QIcon(QPixmap::fromImage(QImage(":noCover").scaled(
                                              ui->coverView->iconSize(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation)));

    connect(ui->coverView, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this,          SLOT(coverDoubleClicked(QListWidgetItem*)));

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this,          SLOT(buttonClicked(QAbstractButton*)));
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
void CoverDialog::setDisk(Disk *disk)
{
    mDisk = disk;
    connect(mDisk.data(), SIGNAL(destroyed(QObject*)), SLOT(close()));
    ui->coverView->clear();
    scan(QFileInfo(disk->cueFile()).absoluteDir().absolutePath());
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
    if (!mDisk)
    {
        this->reject();
        return;
    }


    if (button == ui->buttonBox->button(QDialogButtonBox::Ok))
    {
        QListWidgetItem *item = ui->coverView->currentItem();
        if (!item)
            return;
        mDisk->setCoverImageFile(item->data(FileNameRole).toString());
        this->accept();
        return;
    }


    if (button == ui->buttonBox->button(QDialogButtonBox::Reset))
    {
        mDisk->setCoverImageFile("");
        this->accept();
        return;
    }
}


/************************************************
 *
 ************************************************/
void CoverDialog::scan(const QString &startDir)
{
    QString curFile = mDisk->coverImageFile();
    foreach (QString f, mDisk->searchCoverImages(startDir))
    {
        QFileInfo file(f);
        AsyncListWidgetItem *item = new AsyncListWidgetItem(ui->coverView);
        item->setText(file.baseName());
        item->setData(FileNameRole, file.absoluteFilePath());
        item->setIcon(mEmptyIcon);
        item->setIconAsync(file.absoluteFilePath());

        if (file.absoluteFilePath() == curFile)
            ui->coverView->setCurrentItem(item);
    }
}

