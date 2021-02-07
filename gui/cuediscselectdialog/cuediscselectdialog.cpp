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

#include "cuediscselectdialog.h"
#include "ui_cuediscselectdialog.h"

#include <assert.h>
#include "../cue.h"

/************************************************
 *
 ************************************************/
CueDiscSelectDialog::CueDiscSelectDialog(const QVector<CueDisc> &cue, int selectedDisc, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CueDiscSelectDialog),
    mCue(cue)
{
    if (selectedDisc < 0 || selectedDisc >= cue.count())
        selectedDisc = 0;

    ui->setupUi(this);

    for (int d = 0; d < cue.count(); d++) {
        CueDisc          tags     = cue.at(d);
        QTreeWidgetItem *discItem = new QTreeWidgetItem(ui->discTree);
        assert(!tags.isEmpty());
        discItem->setText(0, tr("%1 [ disc %2 ]", "Cue disc select dialog, string like 'The Wall [disc 1]'").arg(tags.first().album()).arg(d + 1));
        discItem->setData(0, Qt::UserRole, d);
        if (d == selectedDisc) {
            discItem->setSelected(true);
            ui->discTree->setCurrentItem(discItem, 0);
        }

        QFont font = discItem->font(0);
        font.setBold(true);
        discItem->setFont(0, font);

        for (int t = 0; t < tags.count(); ++t) {
            QTreeWidgetItem *trackItem = new QTreeWidgetItem(discItem);
            trackItem->setText(0, tags.at(t).title());
            trackItem->setFlags(Qt::NoItemFlags);
        }
    }
    ui->discTree->expandAll();

    connect(ui->discTree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(treeDoubleClicked(QModelIndex)));
}

/************************************************
 *
 ************************************************/
CueDiscSelectDialog::~CueDiscSelectDialog()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
int CueDiscSelectDialog::discNumber()
{
    return ui->discTree->currentIndex().row();
}

/************************************************
 *
 ************************************************/
void CueDiscSelectDialog::treeDoubleClicked(const QModelIndex &index)
{
    if (!index.parent().isValid())
        accept();
}

/************************************************
 *
 ************************************************/
int CueDiscSelectDialog::getDiscNumber(const QVector<CueDisc> &cue, int selectedDisc)
{
    CueDiscSelectDialog dialog(cue, selectedDisc);

    if (dialog.exec() == QDialog::Accepted)
        return dialog.discNumber();
    else
        return -1;
}
