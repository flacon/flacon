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

#include "updatepage.h"
#include "ui_updatepage.h"

#ifdef MAC_UPDATER

#include "updater/updater.h"

UpdatePage::UpdatePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdatePage)
{
    ui->setupUi(this);

    connect(ui->updateNowBtn, &QPushButton::clicked,
            [this]() {
                Updater::sharedUpdater().checkForUpdatesInBackground();
                updateLastUpdateLbl();
            });

    updateLastUpdateLbl();
}

UpdatePage::~UpdatePage()
{
    delete ui;
}

void UpdatePage::updateLastUpdateLbl()
{
    QDateTime date = Updater::sharedUpdater().lastUpdateCheckDate();
    QString   s;
    if (!date.isNull())
        s = tr("Last check was %1", "Information about last update")
                    .arg(date.toString(Qt::DefaultLocaleLongDate));
    else
        s = tr("Never checked", "Information about last update");

    ui->lastUpdateLbl->setText(s);
}

void UpdatePage::load()
{
    ui->autoUpdateCbk->setChecked(Updater::sharedUpdater().automaticallyChecksForUpdates());
}

void UpdatePage::save()
{
    Updater::sharedUpdater().setAutomaticallyChecksForUpdates(ui->autoUpdateCbk->isChecked());
}
#else

UpdatePage::UpdatePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdatePage)
{
    ui->setupUi(this);
}

UpdatePage::~UpdatePage()
{
    delete ui;
}

void UpdatePage::load()
{
}

void UpdatePage::save()
{
}

#endif
