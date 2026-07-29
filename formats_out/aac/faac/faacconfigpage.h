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

#ifndef FAACCONFIGPAGE_H
#define FAACCONFIGPAGE_H

#include "../../encoderconfigpage.h"

namespace Ui {
class FaacConfigPage;
}

class FaacConfigPage : public EncoderConfigPage
{
    Q_OBJECT

public:
    explicit FaacConfigPage(QWidget *parent = nullptr);
    ~FaacConfigPage();

    static QHash<QString, QVariant> defaultParameters();

    virtual void load(const Profile &profile) override;
    virtual void save(Profile *profile) override;

private:
    void useQualityChecked(bool checked);

    Ui::FaacConfigPage *ui;
};

#endif // FAACCONFIGPAGE_H
