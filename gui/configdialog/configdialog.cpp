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
#include "outformat.h"
#include "../icon.h"
#include "../patternexpander.h"
#include "formats/encoderconfigpage.h"

#include <QFileDialog>
#include <QListWidget>
#include <QDebug>

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

static const int PROFILE_ID_ROLE = Qt::UserRole;


/************************************************
 *
 ************************************************/
ConfigDialog *ConfigDialog::createAndShow(QWidget *parent)
{
    return createAndShow("", parent);
}


/************************************************

 ************************************************/
ConfigDialog *ConfigDialog::createAndShow(const QString &profileId, QWidget *parent)
{
    ConfigDialog *instance = parent->findChild<ConfigDialog*>();

    if (!instance)
        instance = new ConfigDialog(parent);

    instance->pages->setCurrentWidget(instance->profilesPage);
    instance->setProfile(profileId);

    instance->show();
    instance->raise();
    instance->activateWindow();
    instance->setAttribute(Qt::WA_DeleteOnClose);

    return instance;
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
    int width = Settings::i()->value(Settings::ConfigureDialog_Width).toInt();
    int height = Settings::i()->value(Settings::ConfigureDialog_Height).toInt();
    resize(width, height);

    initPrograms();
    initUpdatePage();

    pages->setCurrentIndex(0);
    load();

    preGapComboBox->setEnabled(perTrackCueCheck->isChecked());


    perTrackCueFormatBtn->addPattern("%a", tr("Insert \"Artist\""));
    perTrackCueFormatBtn->addPattern("%A", tr("Insert \"Album title\""));
    perTrackCueFormatBtn->addPattern("%y", tr("Insert \"Year\""));
    perTrackCueFormatBtn->addPattern("%g", tr("Insert \"Genre\""));

    const QString patterns[] = {
        "%a-%A.cue",
        "%a - %A.cue",
        "%a - %y - %A.cue"};


    for (QString pattern: patterns)
    {
        perTrackCueFormatBtn->addFullPattern(pattern,
                                             tr("Use \"%1\"", "Predefined CUE file name, string like 'Use \"%a/%A/%n - %t.cue\"'")
                                             .arg(pattern)
                                             + "  ( " + PatternExpander::example(pattern) + " )");
    }

    connect(perTrackCueFormatBtn, &OutPatternButton::paternSelected,
            [this](const QString &pattern){ perTrackCueFormatEdit->lineEdit()->insert(pattern);});

    connect(perTrackCueFormatBtn, &OutPatternButton::fullPaternSelected,
            [this](const QString &pattern){ perTrackCueFormatEdit->lineEdit()->setText(pattern);});

    connect(profilesList, &QListWidget::currentItemChanged,
            this, &ConfigDialog::profileListSelected);

    connect(profilesList, &QListWidget::itemChanged,
            this, &ConfigDialog::profileItemChanged);

    perTrackCueFormatBtn->setIcon(Icon("pattern-button"));

    fillProfilesList();

#ifdef Q_OS_MAC
    buttonBox->hide();
    line->hide();
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
    connect(tmpDirButton, &QToolButton::clicked, this, &ConfigDialog::tmpDirShowDialog);


    connect(coverDisableButton,  &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::Disable);  });
    connect(coverKeepSizeButton, &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::OrigSize); });
    connect(coverScaleButton,    &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::Scale);    });

    preGapComboBox->addItem(tr("Extract to separate file"), preGapTypeToString(PreGapType::ExtractToFile));
    preGapComboBox->addItem(tr("Add to first track"),       preGapTypeToString(PreGapType::AddToFirstTrack));


    bitDepthComboBox->addItem(tr("Same as source", "Item in combobox"), int(BitsPerSample::AsSourcee));
    bitDepthComboBox->addItem(tr("16-bit",         "Item in combobox"), int(BitsPerSample::Bit_16));
    bitDepthComboBox->addItem(tr("24-bit",         "Item in combobox"), int(BitsPerSample::Bit_24));
    bitDepthComboBox->addItem(tr("32-bit",         "Item in combobox"), int(BitsPerSample::Bit_32));

    sampleRateComboBox->addItem(tr("Same as source", "Item in combobox"), int(SampleRate::AsSource));
    sampleRateComboBox->addItem(tr("44100 Hz",       "Item in combobox"), int(SampleRate::Hz_44100));
    sampleRateComboBox->addItem(tr("48000 Hz",       "Item in combobox"), int(SampleRate::Hz_48000));
    sampleRateComboBox->addItem(tr("96000 Hz",       "Item in combobox"), int(SampleRate::Hz_96000));
    sampleRateComboBox->addItem(tr("192000 Hz",      "Item in combobox"), int(SampleRate::Hz_192000));

}


/************************************************

 ************************************************/
#if defined(MAC_BUNDLE) || defined(FLATPAK_BUNDLE)
void ConfigDialog::initPrograms()
{
    pages->removeTab(pages->indexOf(programsPage));
}
#else
void ConfigDialog::initPrograms()
{
    QStringList progs = QStringList::fromSet(Settings::i()->programs());
    progs.sort();

    int row = 0;
    foreach (QString prog, progs)
    {
        ProgramEdit *edit = new ProgramEdit(prog, programsPage);
        mProgramEdits << edit;

        QLabel *label = new QLabel(prog + ": ");
        label->setBuddy(edit);
        progsLayout->addWidget(label, row, 0);
        progsLayout->addWidget(edit,  row, 1);
        connect(progScanButton, &QPushButton::clicked, edit, &ProgramEdit::find);
        row++;
    }
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
void ConfigDialog::fillProfilesList()
{
    for (const Profile &profile: mProfiles) {
        if (profile.hasConfigPage()) {
            QListWidgetItem *item = new QListWidgetItem(profilesList);
            item->setText(profile.name());
            item->setData(PROFILE_ID_ROLE, profile.id());
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }

    if (profilesList->count() > 0) {
        profilesList->sortItems();
        profilesList->setCurrentRow(0);
    }
}


/************************************************
 *
 ************************************************/
void ConfigDialog::profileListSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (mEncoderPage && previous) {
        mEncoderPage->save();
    }

    if (!current)
        return;

    if (mEncoderPage) {
        profileParent->layout()->removeWidget(mEncoderPage);
        delete mEncoderPage;
    }

    int n = mProfiles.indexOf(current->data(PROFILE_ID_ROLE).toString());    
    if (n<0) {
        profileParent->hide();
        return;
    }

    Profile &profile = mProfiles[n];

    mEncoderPage = profile.configPage(profileParent);
    qobject_cast<QVBoxLayout*>(profileParent->layout())->insertWidget(0, mEncoderPage);

    mEncoderPage->load();
    Controls::loadFromSettings(bitDepthComboBox, Settings::Resample_BitsPerSample);
    Controls::loadFromSettings(sampleRateComboBox, Settings::Resample_SampleRate);

    mEncoderPage->show();
    profileParent->show();
}


/************************************************
 *
 ************************************************/
void ConfigDialog::profileItemChanged(QListWidgetItem *item)
{
    QString id = item->data(PROFILE_ID_ROLE).toString();
    int n = mProfiles.indexOf(id);

    if (n>-1) {
        mProfiles[n].setName(item->text());
    }
}


/************************************************

 ************************************************/
void ConfigDialog::done(int res)
{
    Settings::i()->setValue(Settings::ConfigureDialog_Width,  size().width());
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
void ConfigDialog::setCoverMode(CoverMode mode)
{
    switch (mode)
    {
    case CoverMode::Disable:
        coverDisableButton->setChecked(true);
        coverResizeSpinBox->setEnabled(false);
        break;

    case CoverMode::OrigSize:
        coverKeepSizeButton->setChecked(true);
        coverResizeSpinBox->setEnabled(false);
        break;

    case CoverMode::Scale:
        coverScaleButton->setChecked(true);
        coverResizeSpinBox->setEnabled(true);
        break;
    }
}


/************************************************
 *
 ************************************************/
void ConfigDialog::setProfile(const QString &profileId)
{
    for (int i=0; i<profilesList->count(); ++i) {
        QListWidgetItem *item = profilesList->item(i);
        if (item->data(PROFILE_ID_ROLE).toString() == profileId) {
            profilesList->setCurrentItem(item);
            return;
        }
    }
}


/************************************************
 *
 ************************************************/
void ConfigDialog::updateLastUpdateLbl()
{
#ifdef MAC_UPDATER
    QDateTime date = Updater::sharedUpdater().lastUpdateCheckDate();
    QString s;
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
    Controls::loadFromSettings(perTrackCueCheck, Settings::PerTrackCue_Create);
    Controls::loadFromSettings(preGapComboBox, Settings::PerTrackCue_Pregap);
    perTrackCueFormatEdit->setEditText(Settings::i()->value(Settings::PerTrackCue_FileName).toString());

    setCoverMode(Settings::i()->coverMode());
    Controls::loadFromSettings(coverResizeSpinBox, Settings::Cover_Size);

    foreach(ProgramEdit *edit, mProgramEdits)
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
    Controls::saveToSettings(perTrackCueCheck, Settings::PerTrackCue_Create);
    Controls::saveToSettings(preGapComboBox,   Settings::PerTrackCue_Pregap);

    Settings::i()->setValue(Settings::PerTrackCue_FileName, perTrackCueFormatEdit->currentText());

    Controls::saveToSettings(bitDepthComboBox, Settings::Resample_BitsPerSample);
    Controls::saveToSettings(sampleRateComboBox, Settings::Resample_SampleRate);

    Settings::i()->setValue(Settings::Cover_Mode, coverModeToString(coverMode()));
    Controls::saveToSettings(coverResizeSpinBox, Settings::Cover_Size);

    foreach(ProgramEdit *edit, mProgramEdits)
        Settings::i()->setValue("Programs/" + edit->programName(), edit->text());

    if (mEncoderPage) {
        mEncoderPage->save();
    }
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

 ************************************************/
CoverMode ConfigDialog::coverMode() const
{
    if (coverDisableButton->isChecked())  return CoverMode::Disable;
    if (coverKeepSizeButton->isChecked()) return CoverMode::OrigSize;
    if (coverScaleButton->isChecked())    return CoverMode::Scale;

    return CoverMode::Disable;
}
