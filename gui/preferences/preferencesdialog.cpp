#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "../settings.h"
#include "../icon.h"
#include <QLabel>
#include <QFormLayout>
#include <QToolButton>
#include "../controls.h"

/************************************************
 *
 ************************************************/
PreferencesDialog *PreferencesDialog::createAndShow(QWidget *parent)
{
    return createAndShow("", parent);
}

/************************************************
 *
 ************************************************/
PreferencesDialog *PreferencesDialog::createAndShow(const QString &profileId, QWidget *parent)
{
    PreferencesDialog *instance = parent->findChild<PreferencesDialog *>();

    if (!instance) {
        instance = new PreferencesDialog(parent);
    }

    if (!profileId.isEmpty()) {
        instance->showProfile(profileId);
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
void PreferencesDialog::showProfile(const QString &profileId)
{
    ui->pagesWidget->setCurrentIndex(0);
    ui->profilesPage->selectProfile(profileId);
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

    setStyleSheet("QFrame[frameShape='4'] { border-bottom: 1px solid #7F7F7F7F; background: transparent; }");

    initToolBar();

#ifdef Q_OS_MAC
    ui->buttonBox->hide();
#endif

    // Restore saved size ..................
    fixLayout(this);
    int width  = Settings::i()->value(Settings::ConfigureDialog_Width).toInt();
    int height = Settings::i()->value(Settings::ConfigureDialog_Height).toInt();
    resize(width, height);

    // Restore saved size ..................
    load();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]() { done(true); });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [this]() { done(false); });
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

#if defined(MAC_BUNDLE) || defined(FLATPAK_BUNDLE)
    ui->actShowProgramsPage->setVisible(false);
    ui->programsPage->hide();
#endif

    QList<QAction *> acts = ui->toolBar->actions();
    for (int i = 0; i < acts.length(); ++i) {
        QAction *act = acts[i];
        connect(act, &QAction::triggered, [i, this] { ui->pagesWidget->setCurrentIndex(i); });
        connect(ui->pagesWidget, &QStackedWidget::currentChanged, [i, act](int index) { act->setChecked(index == i); });
    }

    ui->actShowProfilesPage->setChecked(true);
}

/************************************************
 *
 ************************************************/
PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::done(bool accept)
{
    Q_UNUSED(accept)
    Settings::i()->setValue(Settings::ConfigureDialog_Width, size().width());
    Settings::i()->setValue(Settings::ConfigureDialog_Height, size().height());

#ifndef Q_OS_MAC
    if (accept)
#endif
    {
        save();
    }

    Settings::i()->sync();
    close();
    emit finished();
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::load()
{
    const Settings *settings = Settings::i();

    // Profiles page ......................
    ui->profilesPage->setProfiles(settings->profiles());

    // General  page .......................
    ui->generalPage->setEncoderThreadsCount(settings->encoderThreadsCount());
#ifndef FLATPAK_BUNDLE
    ui->generalPage->setTmpDir(settings->tmpDir());
#endif
    ui->generalPage->setDefaultCodepage(settings->defaultCodepage());
    ui->generalPage->setCddbHost(settings->cddbHost());

#if !defined(MAC_BUNDLE) && !defined(FLATPAK_BUNDLE)
    // Programs page .......................
    ui->programsPage->load();
#endif
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::save()
{
    Settings *settings = Settings::i();

    // Profiles page ......................
    Settings::i()->setProfiles(ui->profilesPage->profiles());

    // General  page .......................
    settings->setEncoderThreadsCount(ui->generalPage->encoderThreadsCount());
#ifndef FLATPAK_BUNDLE
    settings->setTmpDir(ui->generalPage->tmpDir());
#endif
    settings->setDefaultCodepage(ui->generalPage->defaultCodepage());
    settings->setCddbHost(ui->generalPage->cddbHost());

#if !defined(MAC_BUNDLE) && !defined(FLATPAK_BUNDLE)
    // Programs page .......................
    ui->programsPage->save();
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

/************************************************
 *
 ************************************************/
void PreferencesDialog::closeEvent(QCloseEvent *event)
{
    done(false);
    QMainWindow::closeEvent(event);
}
