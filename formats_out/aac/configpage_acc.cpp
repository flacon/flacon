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

#include "configpage_acc.h"
#include <QBoxLayout>
#include <QFormLayout>
#include "faac/faacconfigpage.h"
#include "fdkaac/fdkaacconfigpage.h"
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include "../extprogram.h"

/************************************************

 ************************************************/
ConfigPage_Acc::ConfigPage_Acc(QWidget *parent) :
    EncoderConfigPage(parent)
{
    if (ExtProgram::fdkaac()->check()) {
        initDouble();
    }
    else {
        initSingle();
    }
}

/************************************************

 ************************************************/
void ConfigPage_Acc::initSingle()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mFaacPage           = new FaacConfigPage(this);
    layout->addWidget(mFaacPage);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::initDouble()
{

    mProgLabel = new QLabel(this);
    mProgLabel->setText(tr("Program:"));

    mProgCombo = new QComboBox(this);
    mProgCombo->addItem("faac", "faac");
    mProgCombo->addItem("fdkaac", "fdkaac");
    connect(mProgCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &ConfigPage_Acc::progComboChanged);

    mFaacPage   = new FaacConfigPage(this);
    mFdkaacPage = new FdkaacConfigPage(this);

    mPages = new QStackedWidget(this);
    mPages->addWidget(mFaacPage);

    mPages->addWidget(mFdkaacPage);

    QFormLayout *hLayout = new QFormLayout();
    hLayout->setWidget(0, QFormLayout::LabelRole, mProgLabel);
    hLayout->setWidget(0, QFormLayout::FieldRole, mProgCombo);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(mPages);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::progComboChanged()
{
    mPages->setCurrentIndex(mProgCombo->currentIndex());
}

/************************************************

 ************************************************/
void ConfigPage_Acc::load(const Profile &profile)
{
    if (mFaacPage) {
        mFaacPage->load(profile);
    }

    if (mFdkaacPage) {
        mFdkaacPage->load(profile);
    }

    if (mProgCombo) {
        loadWidget(profile, "Program", mProgCombo);
    }
}

/************************************************

 ************************************************/
void ConfigPage_Acc::save(Profile *profile)
{
    if (mFaacPage) {
        mFaacPage->save(profile);
    }

    if (mFdkaacPage) {
        mFdkaacPage->save(profile);
    }

    if (mProgCombo) {
        saveWidget(profile, "Program", mProgCombo);
    }
}
