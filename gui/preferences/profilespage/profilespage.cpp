#include "profilespage.h"
#include "ui_profilespage.h"
#include <QListWidgetItem>

static const int PROFILE_ID_ROLE = Qt::UserRole;

ProfilesPage::ProfilesPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfilesPage)
{
    ui->setupUi(this);

    qDebug() << Q_FUNC_INFO;
    connect(ui->profilesList, &QListWidget::currentItemChanged,
            this, &ProfilesPage::profileListSelected);

    connect(ui->profilesList, &QListWidget::itemChanged,
            this, &ProfilesPage::profileItemChanged);
}

ProfilesPage::~ProfilesPage()
{
    delete ui;
}

void ProfilesPage::setProfiles(const Profiles &value)
{
    mProfiles = value;
    ui->profilesList->blockSignals(true);
    ui->profilesList->clear();

    for (const Profile &profile : qAsConst(mProfiles)) {
        QListWidgetItem *item = new QListWidgetItem(ui->profilesList);
        item->setText(profile.name());
        item->setData(PROFILE_ID_ROLE, profile.id());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    ui->profilesList->sortItems();
    ui->profilesList->blockSignals(false);

    setSelectedProfileId(mSelectedProfileId);
}

const QString &ProfilesPage::selectedProfileId() const
{
    return mSelectedProfileId;
}

void ProfilesPage::setSelectedProfileId(const QString &value)
{
    mSelectedProfileId = value;
    bool isSelected    = false;

    for (int i = 0; i < ui->profilesList->count(); ++i) {
        const auto item = ui->profilesList->item(i);
        isSelected      = isSelected && item->isSelected();

        if (ui->profilesList->item(i)->data(PROFILE_ID_ROLE) == mSelectedProfileId) {
            ui->profilesList->setCurrentRow(i);
            return;
        }
    }

    if (!isSelected && ui->profilesList->count()) {
        ui->profilesList->setCurrentRow(0);
    }
}

void ProfilesPage::profileListSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    qDebug() << Q_FUNC_INFO;
    // pages->blockSignals(true);
    // if (mProfileWidget && mProfileWidget->profile().isValid()) {
    //        mProfiles.update(mProfileWidget->profile());
    //    }

    //    if (mProfileWidget) {
    //        profilePlace->removeWidget(mProfileWidget);
    //        delete mProfileWidget;
    //        mProfileWidget = nullptr;
    //    }

    if (current) {
        int     n       = mProfiles.indexOf(current->data(PROFILE_ID_ROLE).toString());
        Profile profile = (n > -1) ? mProfiles[n] : Profile();

        ui->tabWidget->setProfile(profile);
        // mProfileWidget = new ProfileWidget(profile, profileParent);
        // profilePlace->addWidget(mProfileWidget);
        // mProfileWidget->show();
        // profileParent->show();
    }
    // pages->blockSignals(false);
}

void ProfilesPage::profileItemChanged(QListWidgetItem *item)
{
    QString id = item->data(PROFILE_ID_ROLE).toString();
    if (ui->tabWidget->profile().id() == id) {
        ui->tabWidget->setProfileName(item->text());
    }

    for (Profile &p : mProfiles) {
        if (p.id() == id) {
            p.setName(item->text());
        }
    }
}
