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

#ifndef COVERGROUPBOX_H
#define COVERGROUPBOX_H

#include <QGroupBox>
#include "../types.h"

namespace Ui {
class CoverGroupBox;
}

class CoverGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CoverGroupBox(QWidget *parent = nullptr);
    ~CoverGroupBox();

    CoverOptions coverOptions() const;
    void         setCoverOptions(const CoverOptions &value);

private:
    Ui::CoverGroupBox *ui;

    bool mKeepSize = true;

    void refresh();
};

#endif // COVERGROUPBOX_H
