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


#ifndef CUEDISCSELECTDIALOG_H
#define CUEDISCSELECTDIALOG_H

#include <QDialog>
#include "cue.h"

class QModelIndex;

namespace Ui {
class CueDiscSelectDialog;
}

class CueDiscSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CueDiscSelectDialog(const QVector<CueDisc> &cue, int selectedDisc = 0, QWidget *parent = nullptr);
    ~CueDiscSelectDialog();

    static int getDiscNumber(const QVector<CueDisc> &cue, int selectedDisc = 0);

    int discNumber();

private slots:
    void treeDoubleClicked(const QModelIndex &index);

private:
    Ui::CueDiscSelectDialog *ui;
    const QVector<CueDisc> &mCue;
};

#endif // CUEDISCSELECTDIALOG_H
