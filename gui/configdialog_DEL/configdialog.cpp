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
#define DONE

#include "configdialog.h"
#include "../icon.h"
#include "addprofiledialog.h"

#include <QtGlobal>
#include <QFileDialog>
#include <QDebug>
#include <QDateTime>

DONE #ifdef MAC_UPDATER
DONE #include "updater/updater.h"
DONE #endif

DONE static const int PROFILE_ID_ROLE = Qt::UserRole;



DONE /************************************************
DONE  *
DONE  ************************************************/
DONE ConfigDialog *ConfigDialog::createAndShow(QWidget *parent)
DONE {
DONE     return createAndShow("", parent);
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
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
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::show(const QString &profileId)
DONE {
DONE     for (int i = 0; i < profilesList->count(); ++i) {
DONE         QListWidgetItem *item = profilesList->item(i);
DONE         if (item->data(PROFILE_ID_ROLE).toString() == profileId) {
DONE             profilesList->setCurrentItem(item);
DONE             break;
DONE         }
DONE     }
DONE
DONE     QDialog::show();
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
DONE ConfigDialog::ConfigDialog(QWidget *parent) :
DONE     QDialog(parent)
DONE {
DONE     setupUi(this);
DONE
DONE    this->setMinimumSize(this->size());
DONE
DONE     generalPage->setWindowTitle(tr("General configuration"));
DONE     programsPage->setWindowTitle(tr("Full path of the external applications"));
DONE
DONE     initGeneralPage();
DONE    int width  = Settings::i()->value(Settings::ConfigureDialog_Width).toInt();
DONE    int height = Settings::i()->value(Settings::ConfigureDialog_Height).toInt();
DONE    resize(width, height);
DONE
DONE    int h = addProfileButton->sizeHint().height();
DONE    addProfileButton->setFixedSize(h, h);
DONE    delProfileButton->setFixedSize(h, h);

    initProgramsPage();
DONE     initUpdatePage();
DONE
DONE     pages->setCurrentIndex(0);
DONE     cddbComboBox->addItem("http://www.gnudb.org");
DONE
DONE     load();
DONE
DONE    connect(profilesList, &QListWidget::currentItemChanged,
DONE            this, &ConfigDialog::profileListSelected);
DONE
DONE    connect(profilesList, &QListWidget::itemChanged,
DONE            this, &ConfigDialog::profileItemChanged);
DONE
DONE     connect(addProfileButton, &QToolButton::clicked,
DONE             this, &ConfigDialog::addProfile);
DONE
DONE     connect(delProfileButton, &QToolButton::clicked,
DONE             this, &ConfigDialog::deleteProfile);
DONE
DONE     profileParent->hide();
DONE     refreshProfilesList("");
DONE
DONE #ifdef Q_OS_MAC
DONE    buttonBox->hide();
DONE #endif
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
DONE ConfigDialog::~ConfigDialog()
DONE {
DONE }
DONE
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

DONE     tmpDirButton->setIcon(Icon("folder"));
DONE     tmpDirButton->setBuddy(tmpDirEdit);
DONE     connect(tmpDirButton, &QToolButton::clicked, this, &ConfigDialog::tmpDirShowDialog);
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
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE #ifdef MAC_UPDATER
DONE void ConfigDialog::initUpdatePage()
DONE {
DONE     connect(updateNowBtn, &QPushButton::clicked,
DONE             [this]() {
DONE                 Updater::sharedUpdater().checkForUpdatesInBackground();
DONE                 updateLastUpdateLbl();
DONE             });
DONE
DONE     updateLastUpdateLbl();
DONE }
DONE #else
DONE void ConfigDialog::initUpdatePage()
DONE {
DONE     pages->removeTab(pages->indexOf(updatePage));
DONE }
DONE #endif
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::refreshProfilesList(const QString &selectedProfileId)
DONE {
DONE     QListWidgetItem *sel = nullptr;
DONE
DONE     profilesList->blockSignals(true);
DONE     profilesList->clear();
DONE
DONE     for (const Profile &profile : mProfiles) {
DONE         QListWidgetItem *item = new QListWidgetItem(profilesList);
DONE         item->setText(profile.name());
DONE         item->setData(PROFILE_ID_ROLE, profile.id());
DONE         item->setFlags(item->flags() | Qt::ItemIsEditable);
DONE         if (profile.id() == selectedProfileId) {
DONE             sel = item;
DONE         }
DONE     }
DONE     profilesList->sortItems();
DONE     profilesList->blockSignals(false);
DONE
DONE     if (profilesList->count() > 0) {
DONE         if (sel)
DONE             profilesList->setCurrentItem(sel);
DONE         else
DONE             profilesList->setCurrentRow(0);
DONE     }
DONE }
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::profileListSelected(QListWidgetItem *current, QListWidgetItem *)
DONE {
DONE     pages->blockSignals(true);
DONE     if (mProfileWidget && mProfileWidget->profile().isValid()) {
DONE         mProfiles.update(mProfileWidget->profile());
DONE     }
DONE
DONE     if (mProfileWidget) {
DONE         profilePlace->removeWidget(mProfileWidget);
DONE         delete mProfileWidget;
DONE         mProfileWidget = nullptr;
DONE     }
DONE
DONE     if (current) {
DONE         int     n       = mProfiles.indexOf(current->data(PROFILE_ID_ROLE).toString());
DONE         Profile profile = (n > -1) ? mProfiles[n] : Profile();
DONE
DONE         mProfileWidget = new ProfileWidget(profile, profileParent);
DONE         profilePlace->addWidget(mProfileWidget);
DONE         mProfileWidget->show();
DONE         profileParent->show();
DONE     }
DONE     pages->blockSignals(false);
DONE }
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::profileItemChanged(QListWidgetItem *item)
DONE {
DONE     QString id = item->data(PROFILE_ID_ROLE).toString();
DONE     if (id == currentProfile().id()) {
DONE         currentProfile().setName(item->text());
DONE     }
DONE
DONE     refreshProfilesList(id);
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
DONE void ConfigDialog::done(int res)
DONE {
DONE     Settings::i()->setValue(Settings::ConfigureDialog_Width, size().width());
DONE     Settings::i()->setValue(Settings::ConfigureDialog_Height, size().height());
DONE
DONE #ifndef Q_OS_MAC
DONE     if (res)
DONE #endif
DONE     {
DONE         save();
DONE         Settings::i()->sync();
DONE     }
DONE
DONE     QDialog::done(res);
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
DONE void ConfigDialog::tmpDirShowDialog()
DONE {
DONE     QString tmpDir = QFileDialog::getExistingDirectory(this, tr("Select temporary directory"), tmpDirEdit->text());
DONE     if (!tmpDir.isEmpty())
DONE         tmpDirEdit->setText(tmpDir);
DONE }
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE Profile &ConfigDialog::currentProfile()
DONE {
DONE     return mProfileWidget ? mProfileWidget->profile() : NullProfile();
DONE }
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::updateLastUpdateLbl()
DONE {
DONE #ifdef MAC_UPDATER
DONE     QDateTime date = Updater::sharedUpdater().lastUpdateCheckDate();
DONE     QString   s;
DONE     if (!date.isNull())
DONE         s = tr("Last check was %1", "Information about last update")
DONE                     .arg(date.toString(Qt::DefaultLocaleLongDate));
DONE     else
DONE         s = tr("Never checked", "Information about last update");
DONE
DONE     lastUpdateLbl->setText(s);
DONE #endif
DONE }
DONE
DONE /************************************************
DONE
DONE  ************************************************/
DONE void ConfigDialog::load()
DONE {
DONE     Controls::loadFromSettings(codePageComboBox, Settings::Tags_DefaultCodepage);
DONE     Controls::loadFromSettings(threadsCountSpin, Settings::Encoder_ThreadCount);
DONE
DONE     copyCoverGroupBox->setCoverMode(Settings::i()->coverMode());
DONE     copyCoverGroupBox->setImageSize(Settings::i()->coverImageSize());
DONE
DONE     embededCoverGroupBox->setCoverMode(Settings::i()->embededCoverMode());
DONE     embededCoverGroupBox->setImageSize(Settings::i()->embededCoverImageSize());
DONE
DONE     cddbComboBox->setCurrentText(Settings::i()->value(Settings::Inet_CDDBHost).toString());

    foreach (ProgramEdit *edit, mProgramEdits)
        edit->setText(Settings::i()->value("Programs/" + edit->programName()).toString());

DONE     mProfiles = Settings::i()->profiles();

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
DONE     Controls::saveToSettings(codePageComboBox, Settings::Tags_DefaultCodepage);
DONE     Controls::saveToSettings(threadsCountSpin, Settings::Encoder_ThreadCount);
DONE
DONE     Settings::i()->setCoverMode(copyCoverGroupBox->coverMode());
DONE     Settings::i()->setCoverImageSize(copyCoverGroupBox->imageSize());
DONE
DONE     Settings::i()->setEmbededCoverMode(embededCoverGroupBox->coverMode());
DONE     Settings::i()->setEmbededCoverImageSize(embededCoverGroupBox->imageSize());
DONE
DOEN     Settings::i()->setValue(Settings::Inet_CDDBHost, cddbComboBox->currentText());
DOEN
    foreach (ProgramEdit *edit, mProgramEdits)
        Settings::i()->setValue("Programs/" + edit->programName(), edit->text());

DONE     if (currentProfile().isValid())
DONE         mProfiles.update(currentProfile());
DONE
DONE     Settings::i()->setProfiles(mProfiles);

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
DONE void ConfigDialog::addProfile()
DONE {
DONE     const Profile &  cur = currentProfile();
DONE     AddProfileDialog dialog(this);
DONE     dialog.setWindowModality(Qt::WindowModal);
DONE
DONE     dialog.setFormatId(cur.formatId());
DONE
DONE     if (!dialog.exec())
DONE         return;
DONE
DONE     OutFormat *format = OutFormat::formatForId(dialog.formaiId());
DONE     if (!format)
DONE         return;
DONE
DONE     QString id = QString("%1_%2")
DONE                          .arg(format->id())
DONE                          .arg(QDateTime::currentMSecsSinceEpoch());
DONE
DONE     Profile profile(*format, id);
DONE     profile.setName(dialog.profileName());
DONE     profile.setOutFileDir(cur.outFileDir());
DONE     profile.setOutFilePattern(cur.outFilePattern());
DONE
DONE     mProfiles.append(profile);
DONE
DONE     refreshProfilesList(profile.id());
DONE }
DONE
DONE /************************************************
DONE  *
DONE  ************************************************/
DONE void ConfigDialog::deleteProfile()
DONE {
DONE     Profile prof = currentProfile();
DONE     if (!prof.isValid())
DONE         return;
DONE
DONE     int n = (mProfiles.indexOf(prof.id()));
DONE     if (n < 0)
DONE         return;
DONE
DONE     QMessageBox dialog(this);
DONE     dialog.setText(tr("Are you sure you want to delete the profile \"%1\"?", "Message box text").arg(prof.name()));
DONE     dialog.setTextFormat(Qt::RichText);
DONE     dialog.setIconPixmap(QPixmap(":/64/mainicon"));
DONE
DONE     dialog.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
DONE     dialog.setButtonText(QMessageBox::Yes, tr("Delete the profile", "Button caption"));
DONE     dialog.setDefaultButton(QMessageBox::Yes);
DONE
DONE     dialog.setWindowModality(Qt::WindowModal);
DONE
DONE     int ret = dialog.exec();
DONE
DONE     if (ret != QMessageBox::Yes)
DONE         return;
DONE
DONE     delete profilesList->takeItem(profilesList->currentRow());
DONE     mProfiles.removeAt(n);
DONE }
