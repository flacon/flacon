#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "../settings.h"
#include "../icon.h"
#include <QLabel>
#include <QFormLayout>
#include <QToolButton>

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
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    initToolBar();

#ifdef Q_OS_MAC
    ui->programsPageButton->hide();
    ui->buttonBox->hide();

#else
    ui->updatePageButton->hide();
    ui->toolBar->layout()->removeItem(ui->leftSpacer);

#endif

    // Restore saved size ..................
    fixLayout();
    this->setMinimumSize(this->size());
    int width  = Settings::i()->value(Settings::ConfigureDialog_Width).toInt();
    int height = Settings::i()->value(Settings::ConfigureDialog_Height).toInt();
    resize(width, height);
    // Restore saved size ..................

    load();
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::initToolBar()
{
    ui->profilesPageButton->setIcon(Icon("preferences-audio"));
    ui->generalPageButton->setIcon(Icon("preferences-general"));
    ui->updatePageButton->setIcon(Icon("preferences-update"));
    ui->programsPageButton->setIcon(Icon("preferences-programs"));

    QList<QToolButton *> btns = ui->toolBar->findChildren<QToolButton *>();
    for (int i = 0; i < btns.length(); ++i) {
        QToolButton *btn = btns[i];
        connect(btn, &QToolButton::toggled, [i, this]() { ui->pagesWidget->setCurrentIndex(i); });
        connect(ui->pagesWidget, &QStackedWidget::currentChanged, [i, btn](int index) { btn->setChecked(index == i); });
    }

    int w = 0;
    for (QToolButton *b : ui->toolBar->findChildren<QToolButton *>()) {
        b->setIconSize(QSize(24, 24));
        w = qMax(w, b->width());
    }

    for (QToolButton *b : ui->toolBar->findChildren<QToolButton *>()) {
        b->setFixedWidth(w);
    }

    ui->profilesPageButton->setChecked(true);
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
void PreferencesDialog::done(int res)
{
    Q_UNUSED(res)
    Settings::i()->setValue(Settings::ConfigureDialog_Width, size().width());
    Settings::i()->setValue(Settings::ConfigureDialog_Height, size().height());

#ifndef Q_OS_MAC
    if (res)
#endif
    {
        save();
    }

    Settings::i()->sync();
    QDialog::done(res);
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::load()
{
    ui->profilesPage->setProfiles(Settings::i()->profiles());
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::save()
{
    Settings::i()->setProfiles(ui->profilesPage->profiles());
}

/************************************************
 *
 ************************************************/
void PreferencesDialog::fixLayout()
{
    QList<QLabel *> labels;
    int             width = 0;

    for (auto *layout : findChildren<QFormLayout *>()) {
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

    adjustSize();
}
