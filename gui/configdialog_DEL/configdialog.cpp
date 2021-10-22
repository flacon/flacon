/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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

#include "configdialog.h"
#include "../icon.h"
#include "addprofiledialog.h"

#include <QtGlobal>
#include <QFileDialog>
#include <QDebug>
#include <QDateTime>

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

static const int PROFILE_ID_ROLE = Qt::UserRole;

#define DONE

/************************************************
 *
 ************************************************/
DONE ConfigDialog *ConfigDialog::createAndShow(QWidget *parent)
DONE {
DONE     return createAndShow("", parent);
DONE }

/************************************************

 ************************************************/
DONE ConfigDialog *ConfigDialog::createAndShow(const QString &profileId, QWidget *parent)
DONE {
DONE     ConfigDialog *instance = parent->findChild<ConfigDialog *>();
DONE
DONE     if (!instance)
DONE         instance = new ConfigDialog(parent);
DONE
DONE     instance->pages->setCurrentWidget(instance->profilesPage);
DONE
DONE     instance->show(profileId);
DONE     instance->raise();
DONE     instance->activateWindow();
DONE     instance->setAttribute(Qt::WA_DeleteOnClose);
DONE
DONE     return instance;
DONE }

/************************************************
 *
 ************************************************/
void ConfigDialog::show(const QString &profileId)
{
    for (int i = 0; i < profilesList->count(); ++i) {
        QListWidgetItem *item = profilesList->item(i);
        if (item->data(PROFILE_ID_ROLE).toString() == profileId) {
            profilesList->setCurrentItem(item);
            break;
        }
    }

    QDialog::show();
}

/************************************************

 ************************************************/
ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    this->setMinimumSize(this->size());

    generalPage->setWindowTitle(tr("General configuration"));
    programsPage->setWindowTitle(tr("Full path of the external applications"));

    initGeneralPage();
    int width  = Settings::i()->value(Settings::ConfigureDialog_Width).toInt();
    int height = Settings::i()->value(Settings::ConfigureDialog_Height).toInt();
    resize(width, height);

    int h = addProfileButton->sizeHint().height();
    addProfileButton->setFixedSize(h, h);
    delProfileButton->setFixedSize(h, h);

    initProgramsPage();
    initUpdatePage();

    pages->setCurrentIndex(0);
    cddbComboBox->addItem("http://www.gnudb.org");

    load();

    connect(profilesList, &QListWidget::currentItemChanged,
            this, &ConfigDialog::profileListSelected);

    connect(profilesList, &QListWidget::itemChanged,
            this, &ConfigDialog::profileItemChanged);

    connect(addProfileButton, &QToolButton::clicked,
            this, &ConfigDialog::addProfile);

    connect(delProfileButton, &QToolButton::clicked,
            this, &ConfigDialog::deleteProfile);

    profileParent->hide();
    refreshProfilesList("");

#ifdef Q_OS_MAC
    buttonBox->hide();
#endif
}

/************************************************

 ************************************************/
ConfigDialog::~ConfigDialog()
{
}

/************************************************
 *
 ************************************************/
void ConfigDialog::initGeneralPage()
{
#ifdef FLATPAK_BUNDLE
    tmpDirLabel->hide();
    tmpDirEdit->hide();
    tmpDirButton->hide();
#endif

    tmpDirButton->setIcon(Icon("folder"));
    tmpDirButton->setBuddy(tmpDirEdit);
    connect(tmpDirButton, &QToolButton::clicked, this, &ConfigDialog::tmpDirShowDialog);
}

/************************************************

 ************************************************/
#if defined(MAC_BUNDLE) || defined(FLATPAK_BUNDLE)
void ConfigDialog::initProgramsPage()
{
    pages->removeTab(pages->indexOf(programsPage));
}
#else
void ConfigDialog::initProgramsPage()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList progs = QStringList::fromSet(Settings::i()->programs());
#else
    // After 5.14.0, QT has stated range constructors are available and preferred.
    // See: https://doc.qt.io/qt-5/qlist.html#fromSet
    QSet<QString> program_set = Settings::i()->programs();
    QStringList   progs       = QStringList(program_set.begin(), program_set.end());
#endif
    progs.sort();

    int row = 0;
    foreach (QString prog, progs) {
        ProgramEdit *edit = new ProgramEdit(prog, programsPage);
        mProgramEdits << edit;

        QLabel *label = new QLabel(tr("%1:", "Template for the program name label on the preferences form. %1 is a program name.").arg(prog));
        label->setBuddy(edit);
#ifdef Q_OS_MAC
        label->setAlignment(Qt::AlignRight);
#endif
        progsLayout->addWidget(label, row, 0);
        progsLayout->addWidget(edit, row, 1);
        connect(progScanButton, &QPushButton::clicked, edit, &ProgramEdit::find);
        row++;
    }

    progsArea->setStyleSheet("QScrollArea, #scrollAreaWidgetContents { background-color: transparent;}");
}
#endif

/************************************************
 *
 ************************************************/
#ifdef MAC_UPDATER
void ConfigDialog::initUpdatePage()
{
    connect(updateNowBtn, &QPushButton::clicked,
            [this]() {
                Updater::sharedUpdater().checkForUpdatesInBackground();
                updateLastUpdateLbl();
            });

    updateLastUpdateLbl();
}
#else
void ConfigDialog::initUpdatePage()
{
    pages->removeTab(pages->indexOf(updatePage));
}
#endif

/************************************************
 *
 ************************************************/
void ConfigDialog::refreshProfilesList(const QString &selectedProfileId)
{
    QListWidgetItem *sel = nullptr;

    profilesList->blockSignals(true);
    profilesList->clear();

    for (const Profile &profile : mProfiles) {
        QListWidgetItem *item = new QListWidgetItem(profilesList);
        item->setText(profile.name());
        item->setData(PROFILE_ID_ROLE, profile.id());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (profile.id() == selectedProfileId) {
            sel = item;
        }
    }
    profilesList->sortItems();
    profilesList->blockSignals(false);

    if (profilesList->count() > 0) {
        if (sel)
            profilesList->setCurrentItem(sel);
        else
            profilesList->setCurrentRow(0);
    }
}

/************************************************
 *
 ************************************************/
void ConfigDialog::profileListSelected(QListWidgetItem *current, QListWidgetItem *)
{
    pages->blockSignals(true);
    if (mProfileWidget && mProfileWidget->profile().isValid()) {
        mProfiles.update(mProfileWidget->profile());
    }

    if (mProfileWidget) {
        profilePlace->removeWidget(mProfileWidget);
        delete mProfileWidget;
        mProfileWidget = nullptr;
    }

    if (current) {
        int     n       = mProfiles.indexOf(current->data(PROFILE_ID_ROLE).toString());
        Profile profile = (n > -1) ? mProfiles[n] : Profile();

        mProfileWidget = new ProfileWidget(profile, profileParent);
        profilePlace->addWidget(mProfileWidget);
        mProfileWidget->show();
        profileParent->show();
    }
    pages->blockSignals(false);
}

/************************************************
 *
 ************************************************/
void ConfigDialog::profileItemChanged(QListWidgetItem *item)
{
    QString id = item->data(PROFILE_ID_ROLE).toString();
    if (id == currentProfile().id()) {
        currentProfile().setName(item->text());
    }

    refreshProfilesList(id);
}

/************************************************

 ************************************************/
void ConfigDialog::done(int res)
{
    Settings::i()->setValue(Settings::ConfigureDialog_Width, size().width());
    Settings::i()->setValue(Settings::ConfigureDialog_Height, size().height());

#ifndef Q_OS_MAC
    if (res)
#endif
    {
        save();
        Settings::i()->sync();
    }

    QDialog::done(res);
}

/************************************************

 ************************************************/
void ConfigDialog::tmpDirShowDialog()
{
    QString tmpDir = QFileDialog::getExistingDirectory(this, tr("Select temporary directory"), tmpDirEdit->text());
    if (!tmpDir.isEmpty())
        tmpDirEdit->setText(tmpDir);
}

/************************************************
 *
 ************************************************/
Profile &ConfigDialog::currentProfile()
{
    return mProfileWidget ? mProfileWidget->profile() : NullProfile();
}

/************************************************
 *
 ************************************************/
void ConfigDialog::updateLastUpdateLbl()
{
#ifdef MAC_UPDATER
    QDateTime date = Updater::sharedUpdater().lastUpdateCheckDate();
    QString   s;
    if (!date.isNull())
        s = tr("Last check was %1", "Information about last update")
                    .arg(date.toString(Qt::DefaultLocaleLongDate));
    else
        s = tr("Never checked", "Information about last update");

    lastUpdateLbl->setText(s);
#endif
}

/************************************************

 ************************************************/
void ConfigDialog::load()
{
    Controls::loadFromSettings(codePageComboBox, Settings::Tags_DefaultCodepage);
    Controls::loadFromSettings(threadsCountSpin, Settings::Encoder_ThreadCount);

    copyCoverGroupBox->setCoverMode(Settings::i()->coverMode());
    copyCoverGroupBox->setImageSize(Settings::i()->coverImageSize());

    embededCoverGroupBox->setCoverMode(Settings::i()->embededCoverMode());
    embededCoverGroupBox->setImageSize(Settings::i()->embededCoverImageSize());

    cddbComboBox->setCurrentText(Settings::i()->value(Settings::Inet_CDDBHost).toString());

    foreach (ProgramEdit *edit, mProgramEdits)
        edit->setText(Settings::i()->value("Programs/" + edit->programName()).toString());

    mProfiles = Settings::i()->profiles();

#ifdef MAC_UPDATER
    autoUpdateCbk->setChecked(Updater::sharedUpdater().automaticallyChecksForUpdates());
#endif

#ifndef FLATPAK_BUNDLE
    Controls::loadFromSettings(tmpDirEdit, Settings::Encoder_TmpDir);
#endif
}

/************************************************

 ************************************************/
void ConfigDialog::save()
{
    Controls::saveToSettings(codePageComboBox, Settings::Tags_DefaultCodepage);
    Controls::saveToSettings(threadsCountSpin, Settings::Encoder_ThreadCount);

    Settings::i()->setCoverMode(copyCoverGroupBox->coverMode());
    Settings::i()->setCoverImageSize(copyCoverGroupBox->imageSize());

    Settings::i()->setEmbededCoverMode(embededCoverGroupBox->coverMode());
    Settings::i()->setEmbededCoverImageSize(embededCoverGroupBox->imageSize());

    Settings::i()->setValue(Settings::Inet_CDDBHost, cddbComboBox->currentText());

    foreach (ProgramEdit *edit, mProgramEdits)
        Settings::i()->setValue("Programs/" + edit->programName(), edit->text());

    if (currentProfile().isValid())
        mProfiles.update(currentProfile());

    Settings::i()->setProfiles(mProfiles);

#ifdef MAC_UPDATER
    Updater::sharedUpdater().setAutomaticallyChecksForUpdates(
            autoUpdateCbk->isChecked());
#endif

#ifndef FLATPAK_BUNDLE
    Controls::saveToSettings(tmpDirEdit, Settings::Encoder_TmpDir);
#endif
}

/************************************************
 *
 ************************************************/
void ConfigDialog::addProfile()
{
    const Profile &  cur = currentProfile();
    AddProfileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);

    dialog.setFormatId(cur.formatId());

    if (!dialog.exec())
        return;

    OutFormat *format = OutFormat::formatForId(dialog.formaiId());
    if (!format)
        return;

    QString id = QString("%1_%2")
                         .arg(format->id())
                         .arg(QDateTime::currentMSecsSinceEpoch());

    Profile profile(*format, id);
    profile.setName(dialog.profileName());
    profile.setOutFileDir(cur.outFileDir());
    profile.setOutFilePattern(cur.outFilePattern());

    mProfiles.append(profile);

    refreshProfilesList(profile.id());
}

/************************************************
 *
 ************************************************/
void ConfigDialog::deleteProfile()
{
    Profile prof = currentProfile();
    if (!prof.isValid())
        return;

    int n = (mProfiles.indexOf(prof.id()));
    if (n < 0)
        return;

    QMessageBox dialog(this);
    dialog.setText(tr("Are you sure you want to delete the profile \"%1\"?", "Message box text").arg(prof.name()));
    dialog.setTextFormat(Qt::RichText);
    dialog.setIconPixmap(QPixmap(":/64/mainicon"));

    dialog.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    dialog.setButtonText(QMessageBox::Yes, tr("Delete the profile", "Button caption"));
    dialog.setDefaultButton(QMessageBox::Yes);

    dialog.setWindowModality(Qt::WindowModal);

    int ret = dialog.exec();

    if (ret != QMessageBox::Yes)
        return;

    delete profilesList->takeItem(profilesList->currentRow());
    mProfiles.removeAt(n);
}
