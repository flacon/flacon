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
