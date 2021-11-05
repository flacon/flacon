#ifndef PROFILESPAGE_H
#define PROFILESPAGE_H

#include <QWidget>
#include "profiles.h"

class QListWidgetItem;

namespace Ui {
class ProfilesPage;
}

class ProfilesPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProfilesPage(QWidget *parent = nullptr);
    ~ProfilesPage();

    const Profiles &profiles() const;
    void            setProfiles(const Profiles &value);

    QString selectedProfileId() const { return mProfile.id(); };

    void selectProfile(const QString &id);

private:
    Ui::ProfilesPage *ui;
    mutable Profiles  mProfiles;
    mutable Profile   mProfile;

    void profileListSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void profileItemChanged(QListWidgetItem *item);
    void syncProfile() const;

    void addProfile();
    void deleteProfile();
};

#endif // PROFILESPAGE_H
