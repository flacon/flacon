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

#include "profilespage.h"
#include "ui_profilespage.h"
#include <QListWidgetItem>
#include "addprofiledialog.h"
#include <QDateTime>

static const int PROFILE_ID_ROLE = Qt::UserRole;

/************************************************
 *
 ************************************************/
ProfileListWidget::ProfileListWidget(QWidget *parent) :
    QListWidget(parent)
{
    connect(this, &QListWidget::currentRowChanged, this, [this](int row) {
        emit currentProfileChanged(this->rowId(row));
    });
}

/************************************************
 *
 ************************************************/
void ProfileListWidget::refresh(const Profiles &profiles)
{
    QString id = currentId();

    blockSignals(true);
    clear();
    for (const Profile &p : profiles) {
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setText(p.name());
        item->setData(PROFILE_ID_ROLE, p.id());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if (p.id() == id) {
            setCurrentItem(item);
        }
    }

    sortItems();
    blockSignals(false);

    if (!currentItem() && count()) {
        setCurrentRow(0);
    }
}

/************************************************
 *
 ************************************************/
QString ProfileListWidget::currentId() const
{
    QListWidgetItem *item = currentItem();
    return item ? item->data(PROFILE_ID_ROLE).toString() : "";
}

/************************************************
 *
 ************************************************/
void ProfileListWidget::setCurrentId(const QString &id)
{
    for (int i = 0; i < count(); ++i) {
        if (item(i)->data(PROFILE_ID_ROLE).toString() == id) {
            setCurrentRow(i);
            return;
        }
    }

    setCurrentRow(-1);
}

/************************************************
 *
 ************************************************/
QString ProfileListWidget::rowId(int row) const
{
    if (row > -1 && row < count()) {
        return item(row)->data(PROFILE_ID_ROLE).toString();
    }

    return "";
}

/************************************************
 *
 ************************************************/
ProfilesPage::ProfilesPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfilesPage)
{
    ui->setupUi(this);

    int h = ui->addProfileButton->sizeHint().height();
    ui->addProfileButton->setFixedSize(h, h);
    ui->delProfileButton->setFixedSize(h, h);

    connect(ui->profilesList, &ProfileListWidget::currentProfileChanged,
            this, &ProfilesPage::selectProfile);

    connect(ui->profilesList, &QListWidget::itemChanged,
            this, &ProfilesPage::renameProfile);

    connect(ui->addProfileButton, &QToolButton::clicked,
            this, &ProfilesPage::addProfile);

    connect(ui->delProfileButton, &QToolButton::clicked,
            this, &ProfilesPage::deleteProfile);
}

/************************************************
 *
 ************************************************/
ProfilesPage::~ProfilesPage()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
Profiles ProfilesPage::profiles() const
{
    ui->tabWidget->toProfile(mProfile);
    return mProfiles;
}

/************************************************
 *
 ************************************************/
void ProfilesPage::setProfiles(const Profiles &profiles)
{
    mProfiles = profiles;
    ui->profilesList->refresh(profiles);
    selectProfile(ui->profilesList->currentId());
}

/************************************************
 *
 ************************************************/
void ProfilesPage::selectProfile(const QString &profileId)
{
    Profile *profile = mProfiles.find(profileId);

    if (mProfile) {
        ui->tabWidget->toProfile(mProfile);
    }

    mProfile = (!profile && !mProfiles.isEmpty()) ? &mProfiles.first() : profile;

    ui->profilesList->blockSignals(true);
    ui->profilesList->setCurrentId(profileId);
    ui->profilesList->blockSignals(false);

    ui->tabWidget->fromProfile(mProfile);
    ui->delProfileButton->setEnabled(mProfile != nullptr);
}

/************************************************
 *
 ************************************************/
void ProfilesPage::renameProfile(QListWidgetItem *item)
{
    QString id = item->data(PROFILE_ID_ROLE).toString();

    for (Profile &p : mProfiles) {
        if (p.id() == id) {
            p.setName(item->text());
        }
    }
}

/************************************************
 *
 ************************************************/
void ProfilesPage::addProfile()
{
    QString formatId       = mProfile ? mProfile->formatId() : "FLAC";
    QString outFileDir     = mProfile ? mProfile->outFileDir() : Profile().outFileDir();
    QString outFilePattern = mProfile ? mProfile->outFilePattern() : Profile().outFilePattern();

    AddProfileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);

    dialog.setFormatId(formatId);

    if (!dialog.exec()) {
        return;
    }

    QString id = QString("%1_%2")
                         .arg(dialog.formaiId())
                         .arg(QDateTime::currentMSecsSinceEpoch());

    Profile profile(dialog.formaiId(), id);
    profile.setName(dialog.profileName());
    profile.setOutFileDir(outFileDir);
    profile.setOutFilePattern(outFilePattern);

    if (!profile.isValid()) {
        return;
    }

    mProfiles.append(profile);
    ui->profilesList->refresh(mProfiles);
    selectProfile(profile.id());
}

/************************************************
 *
 ************************************************/
void ProfilesPage::deleteProfile()
{
    if (!mProfile) {
        return;
    }

    int n = mProfiles.indexOf(mProfile->id());
    if (n < 0) {
        return;
    }

    QMessageBox dialog(this);
    dialog.setText(tr("Are you sure you want to delete the profile \"%1\"?", "Message box text").arg(mProfile->name()));
    dialog.setTextFormat(Qt::RichText);
    dialog.setIconPixmap(QPixmap(":/64/mainicon"));

    dialog.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    dialog.setButtonText(QMessageBox::Yes, tr("Delete the profile", "Button caption"));
    dialog.setDefaultButton(QMessageBox::Yes);

    dialog.setWindowModality(Qt::WindowModal);

    int ret = dialog.exec();

    if (ret != QMessageBox::Yes) {
        return;
    }

    Profile profile = mProfiles.takeAt(n);
    if (mProfile && mProfile->id() == profile.id()) {
        mProfile = nullptr;
    }

    n = ui->profilesList->currentRow();
    ui->profilesList->refresh(mProfiles);

    n = std::min(n, ui->profilesList->count() - 1);
    selectProfile(ui->profilesList->rowId(n));
}
