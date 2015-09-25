/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2015
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


#include "cuediskselectdialog.h"
#include "ui_cuediskselectdialog.h"

#include "../cue.h"


/************************************************
 *
 ************************************************/
CueDiskSelectDialog::CueDiskSelectDialog(const CueReader &cue, int selectedDisk, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CueDiskSelectDialog),
    mCue(cue)
{
    if (selectedDisk < 0 || selectedDisk >= cue.diskCount())
        selectedDisk = 0;

    ui->setupUi(this);
    ui->text->setText(tr("The CUE contains information about multiple discs. What disk you want to use?"));

    for (int d=0; d<cue.diskCount(); d++)
    {
        CueTagSet tags = cue.disk(d);
        QTreeWidgetItem *diskItem = new QTreeWidgetItem(ui->diskTree);
        diskItem->setText(0, tr("%1 [ disk %2 ]").arg(tags.diskTag("ALBUM")).arg(d+1));
        diskItem->setData(0,Qt::UserRole, d);
        if (d == selectedDisk)
        {
            diskItem->setSelected(true);
            ui->diskTree->setCurrentItem(diskItem, 0);
        }

        QFont font = diskItem->font(0);
        font.setBold(true);
        diskItem->setFont(0, font);

        for (int t=0; t<tags.tracksCount(); ++t)
        {
            QTreeWidgetItem *trackItem = new QTreeWidgetItem(diskItem);
            trackItem->setText(0, tags.trackTag(t, "TITLE"));
            trackItem->setFlags(Qt::NoItemFlags );
        }
    }
    ui->diskTree->expandAll();


    connect(ui->diskTree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(treeDoubleClicked(QModelIndex)));
}


/************************************************
 *
 ************************************************/
CueDiskSelectDialog::~CueDiskSelectDialog()
{
    delete ui;
}


/************************************************
 *
 ************************************************/
int CueDiskSelectDialog::diskNumber()
{
    return ui->diskTree->currentIndex().row();
}


/************************************************
 *
 ************************************************/
void CueDiskSelectDialog::treeDoubleClicked(const QModelIndex &index)
{
    if (!index.parent().isValid())
        accept();
}


/************************************************
 *
 ************************************************/
int CueDiskSelectDialog::getDiskNumber(const CueReader &cue, int selectedDisk)
{
    CueDiskSelectDialog dialog(cue, selectedDisk);

    if (dialog.exec() == QDialog::Accepted)
        return dialog.diskNumber();
    else
        return -1;
}
