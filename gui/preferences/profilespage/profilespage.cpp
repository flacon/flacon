#include "profilespage.h"
#include "ui_profilespage.h"
#include <QListWidgetItem>
#include "addprofiledialog.h"
#include <QDateTime>

static const int PROFILE_ID_ROLE = Qt::UserRole;

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

    connect(ui->profilesList, &QListWidget::currentItemChanged,
            this, &ProfilesPage::profileListSelected);

    connect(ui->profilesList, &QListWidget::itemChanged,
            this, &ProfilesPage::profileItemChanged);

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
const Profiles &ProfilesPage::profiles() const
{
    syncProfile();
    return mProfiles;
}

/************************************************
 *
 ************************************************/
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

    selectProfile("");
}

/************************************************
 *
 ************************************************/
void ProfilesPage::syncProfile() const
{
    if (mProfile.isValid()) {
        ui->tabWidget->toProfile(&mProfile);
        mProfiles.update(mProfile);
    }
}

/************************************************
 *
 ************************************************/
void ProfilesPage::selectProfile(const QString &id)
{
    bool isSelected = false;

    for (int i = 0; i < ui->profilesList->count(); ++i) {
        const auto item = ui->profilesList->item(i);
        isSelected      = isSelected && item->isSelected();

        if (ui->profilesList->item(i)->data(PROFILE_ID_ROLE) == id) {
            ui->profilesList->setCurrentRow(i);
            return;
        }
    }

    if (!isSelected && ui->profilesList->count()) {
        ui->profilesList->setCurrentRow(0);
    }
}

/************************************************
 *
 ************************************************/
void ProfilesPage::profileListSelected(QListWidgetItem *current, QListWidgetItem *)
{
    syncProfile();

    if (current) {
        int n = mProfiles.indexOf(current->data(PROFILE_ID_ROLE).toString());

        mProfile = (n > -1) ? mProfiles[n] : Profile();
        ui->tabWidget->fromProfile(mProfile);
    }
}

/************************************************
 *
 ************************************************/
void ProfilesPage::profileItemChanged(QListWidgetItem *item)
{
    QString id = item->data(PROFILE_ID_ROLE).toString();
    if (mProfile.id() == id) {
        mProfile.setName(item->text());
    }

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
    const Profile   &cur = mProfile;
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

    setProfiles(mProfiles);
    selectProfile(profile.id());
}

/************************************************
 *
 ************************************************/
void ProfilesPage::deleteProfile()
{
    if (!mProfile.isValid()) {
        return;
    }

    int n = (mProfiles.indexOf(mProfile.id()));
    if (n < 0) {
        return;
    }

    QMessageBox dialog(this);
    dialog.setText(tr("Are you sure you want to delete the profile \"%1\"?", "Message box text").arg(mProfile.name()));
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

    delete ui->profilesList->takeItem(ui->profilesList->currentRow());
    mProfiles.removeAt(n);
}
