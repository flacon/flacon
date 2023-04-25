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

#ifndef PROFILETABWIDGET_H
#define PROFILETABWIDGET_H

#include <QTabWidget>
#include "profiles.h"
#include <memory>
#include "formats_out/encoderconfigpage.h"
#include "../controls.h"

namespace Ui {
class ProfileTabWidget;
}

class ProfileTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit ProfileTabWidget(QWidget *parent = nullptr);
    ~ProfileTabWidget();

    void fromProfile(const Profile *profile);
    void toProfile(Profile *profile) const;

private:
    Ui::ProfileTabWidget *ui;

    std::unique_ptr<EncoderConfigPage> mEncoderWidget = nullptr;

    void recreateEncoderWidget(const Profile *profile);
};

using BitsPerSampleCombobox = EnumCombobox<int>;
using SampleRateCombobox    = EnumCombobox<SampleRate>;
using GainTypeCombobox      = EnumCombobox<GainType>;

#endif // PROFILETABWIDGET_H
