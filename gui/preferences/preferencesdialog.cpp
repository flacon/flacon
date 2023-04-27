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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "../icon.h"
#include <QLabel>
#include <QFormLayout>
#include <QToolButton>
#include <QMessageBox>
#include "../controls.h"
#include "settings.h"

#ifdef Q_OS_MAC
static constexpr bool DIALOG_HAS_BUTTONS = false;
#else
static constexpr bool DIALOG_HAS_BUTTONS = true;
#endif

static constexpr auto SETTINGS_DIALOG_WIDTH_KEY  = "ConfigureDialog/Width";
static constexpr auto SETTINGS_DIALOG_HEIGHT_KEY = "ConfigureDialog/Height";

/************************************************
 *
 ************************************************/
PreferencesDialog *PreferencesDialog::createAndShow(const Profiles &profiles, QWidget *parent)
{
    return createAndShow(profiles, "", parent);
}

/************************************************
 *
 ************************************************/
PreferencesDialog *PreferencesDialog::createAndShow(const Profiles &profiles, const QString &currentProfileId, QWidget *parent)
{
    PreferencesDialog *instance = parent->findChild<PreferencesDialog *>();

    if (!instance) {
        instance = new PreferencesDialog(parent);
    }

    instance->setProfiles(profiles);
    if (!currentProfileId.isEmpty()) {
        instance->showProfile(currentProfileId);
    }

    instance->show();
    instance->raise();
    instance->activateWindow();
    instance->setAttribute(Qt::WA_DeleteOnClose);
    return instance;
}

/************************************************
 *
 ************************************************/
PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    this->setMinimumSize(this->size());

    if (parent) {
        parent->installEventFilter(this);
    }

    setStyleSheet("QFrame[frameShape='4'] { border-bottom: 1px solid #7F7F7F7F; background: transparent; }");

    initToolBar();

    ui->buttonBox->setVisible(DIALOG_HAS_BUTTONS);
    mSaveOnClose = !DIALOG_HAS_BUTTONS;

    // Restore saved size ..................
    fixLayout(this);
    int width  = Settings::i()->value(SETTINGS_DIALOG_WIDTH_KEY).toInt();
    int height = Settings::i()->value(SETTINGS_DIALOG_HEIGHT_KEY).toInt();
    resize(width, height);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        mSaveOnClose = true;
        close();
    });

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        mSaveOnClose = false;
        close();
    });
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::initToolBar()
{
    this->setUnifiedTitleAndToolBarOnMac(true);

    ui->actShowProfilesPage->setIcon(Icon("preferences-audio"));
    ui->actShowGeneralPage->setIcon(Icon("preferences-general"));
    ui->actShowUpdatePage->setIcon(Icon("preferences-update"));
    ui->actShowProgramsPage->setIcon(Icon("preferences-programs"));

    Controls::arangeTollBarButtonsWidth(ui->toolBar);

#ifndef MAC_UPDATER
    ui->actShowUpdatePage->setVisible(false);
    ui->updatePage->hide();
#endif

#ifdef BUNDLED_PROGRAMS
    ui->actShowProgramsPage->setVisible(false);
    ui->programsPage->hide();
#endif

    QList<QAction *> acts = ui->toolBar->actions();
    for (int i = 0; i < acts.length(); ++i) {
        QAction *act = acts[i];
        connect(act, &QAction::triggered, this, [i, this] { ui->pagesWidget->setCurrentIndex(i); });
        connect(ui->pagesWidget, &QStackedWidget::currentChanged, this, [i, act](int index) { act->setChecked(index == i); });
    }

    ui->actShowProfilesPage->setChecked(true);
}

/************************************************
 *
 ************************************************/
PreferencesDialog::~PreferencesDialog()
{
    Settings::i()->setValue(SETTINGS_DIALOG_WIDTH_KEY, size().width());
    Settings::i()->setValue(SETTINGS_DIALOG_HEIGHT_KEY, size().height());
    Settings::i()->sync();

    delete ui;
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::showProfile(const QString &profileId)
{
    ui->pagesWidget->setCurrentIndex(0);
    ui->profilesPage->selectProfile(profileId);
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::setProfiles(const Profiles &profiles)
{
    // Profiles page ......................
    ui->profilesPage->setProfiles(profiles);

    // General  page .......................
    Profile p = !profiles.isEmpty() ? profiles.first() : Profile();
    ui->generalPage->setEncoderThreadsCount(p.encoderThreadsCount());
    ui->generalPage->setTmpDir(p.tmpDir());
    ui->generalPage->setDefaultCodepage(p.defaultCodepage());

#ifndef BUNDLED_PROGRAMS
    // Programs page .......................
    ui->programsPage->load();
#endif
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::fixLayout(const QWidget *parent)
{
    QList<QLabel *> labels;
    int             width = 0;

    for (auto *layout : parent->findChildren<QFormLayout *>()) {
        layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        for (int r = 0; r < layout->count(); ++r) {
            QLayoutItem *item = layout->itemAt(r, QFormLayout::LabelRole);
            if (!item)
                continue;

            QLabel *label = qobject_cast<QLabel *>(item->widget());
            if (label) {
                labels << label;
                width = qMax(width, label->sizeHint().width());
            }
        }
    }

    for (QLabel *label : labels) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setMinimumWidth(width);
    }
}

Profiles PreferencesDialog::profiles() const
{
    return ui->profilesPage->profiles();
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::closeEvent(QCloseEvent *event)
{
    if (mSaveOnClose && !save()) {
        event->ignore();
    }
}

/************************************************
 *
 ************************************************/
bool PreferencesDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parent() && event->type() == QEvent::Close) {
        mSaveOnClose = false;
        close();
    }

    return QMainWindow::eventFilter(obj, event);
}

/************************************************
 *
 ************************************************/
bool PreferencesDialog::save()
{
    // Profiles page ......................
    if (ui->profilesPage->profiles().isEmpty()) {
        QMessageBox dialog(this);
        dialog.setIconPixmap(QPixmap(":/64/mainicon"));

        dialog.setText(tr("I can't apply your preferences.", "Message box text"));
        dialog.setInformativeText(tr("You should create at least one profile.", "Message box text"));

        dialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        dialog.setButtonText(QMessageBox::Ok, tr("Create profile", "Button caption"));
        dialog.setDefaultButton(QMessageBox::Ok);
        dialog.setWindowModality(Qt::ApplicationModal);

        if (dialog.exec() == QMessageBox::Ok) {
            ui->profilesPage->addProfile();
        }
        return false;
    }

    // General  page .......................
    Profile &p = ui->profilesPage->profiles().first();
    p.setEncoderThreadsCount(ui->generalPage->encoderThreadsCount());
    p.setTmpDir(ui->generalPage->tmpDir());
    p.setDefaultCodepage(ui->generalPage->defaultCodepage());

#ifndef BUNDLED_PROGRAMS
    // Programs page .......................
    ui->programsPage->save();
#endif

    emit accepted();
    return true;
}
