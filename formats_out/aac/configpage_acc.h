/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2026
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

#ifndef CONFIGPAGE_ACC_H
#define CONFIGPAGE_ACC_H

#include "../encoderconfigpage.h"

class QLabel;
class QComboBox;
class FaacConfigPage;
class FdkaacConfigPage;
class QStackedWidget;

class ConfigPage_Acc : public EncoderConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Acc(QWidget *parent = nullptr);

    void load(const Profile &profile) override;
    void save(Profile *profile) override;

private:
    QLabel    *mProgLabel = nullptr;
    QComboBox *mProgCombo = nullptr;

    QStackedWidget   *mPages      = nullptr;
    FaacConfigPage   *mFaacPage   = nullptr;
    FdkaacConfigPage *mFdkaacPage = nullptr;

    void initSingle();
    void initDouble();

    void progComboChanged();
};
#endif // CONFIGPAGE_ACC_H
