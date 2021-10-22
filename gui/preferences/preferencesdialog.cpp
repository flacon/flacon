#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "../settings.h"
#include "../icon.h"
#include <QLabel>
#include <QFormLayout>
#include <QToolButton>

PreferencesDialog *PreferencesDialog::createAndShow(QWidget *parent)
{
    return createAndShow("", parent);
}

PreferencesDialog *PreferencesDialog::createAndShow(const QString &profileId, QWidget *parent)
{
    PreferencesDialog *instance = parent->findChild<PreferencesDialog *>();

    if (!instance) {
        instance = new PreferencesDialog(parent);
    }

    instance->ui->profilesPage->show();
    instance->ui->profilesPage->setSelectedProfileId(profileId);
    instance->show();
    instance->raise();
    instance->activateWindow();
    instance->setAttribute(Qt::WA_DeleteOnClose);

    return instance;
}

void PreferencesDialog::closeEvent(QCloseEvent *event)
{
    done(false);
    QMainWindow::closeEvent(event);
}

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
#define TOOLBAR 1
#if TOOLBAR
    ui->actionProfiles->setIcon(Icon("preferences-audio"));
    ui->actionGeneral->setIcon(Icon("preferences-general"));
    ui->actionUpdate->setIcon(Icon("preferences-update"));

    int w = 0;
    for (const QToolButton *b : ui->toolBar->findChildren<QToolButton *>()) {
        w = qMax(w, b->width());
    }

    for (QToolButton *b : ui->toolBar->findChildren<QToolButton *>()) {
        b->setFixedWidth(w);
    }

    ui->toolButtons->hide();
#else

    ui->profilesPageButton->setIcon(Icon("preferences-audio"));
    ui->generalPageButton->setIcon(Icon("preferences-general"));
    ui->updatePageButton->setIcon(Icon("preferences-update"));

    int w = 0;
    for (const QToolButton *b : ui->toolButtons->findChildren<QToolButton *>()) {
        w = qMax(w, b->width());
    }

    for (QToolButton *b : ui->toolButtons->findChildren<QToolButton *>()) {
        b->setFixedWidth(w);
    }

    ui->toolBar->hide();
#endif

    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->toolBar->setIconSize(QSize(24, 24));
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#ifdef Q_OS_MAC
    this->setUnifiedTitleAndToolBarOnMac(true);
    ui->actionPrograms->setVisible(false);
    ui->buttonBox->hide();

    //    QCoreApplication::setAttribute( Qt::AA_DontCreateNativeWidgetSiblings );
    //    NSView *nsview = ( __bridge NSView * )reinterpret_cast<void *>( this->window()->winId() );
    //    NSWindow *nswindow = [nsview window];
    //    nswindow.titlebarAppearsTransparent = YES;

#endif

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]() { done(true); });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]() { done(true); });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [this]() { done(false); });
    fixLayout();

    load();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

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
        Settings::i()->sync();
    }

    close();
}

void PreferencesDialog::load()
{
    ui->profilesPage->setProfiles(Settings::i()->profiles());
}

void PreferencesDialog::save()
{
}

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
