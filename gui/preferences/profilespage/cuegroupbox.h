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

#ifndef CUEGROUPBOX_H
#define CUEGROUPBOX_H

#include <QGroupBox>
#include "types.h"
#include "profiles.h"
#include "controls.h"

namespace Ui {
class CueGroupBox;
}

class CueGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CueGroupBox(QWidget *parent = nullptr);
    ~CueGroupBox();

    void fromProfile(const Profile *profile);
    void toProfile(Profile *profile) const;

private:
    Ui::CueGroupBox *ui;

    bool mSupportEmbeddedCue = false;

    void refresh();
};

using PreGapTypeComboBox = EnumCombobox<PreGapType>;

#endif // CUEGROUPBOX_H
