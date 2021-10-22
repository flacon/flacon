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

    const Profiles &profiles() const { return mProfiles; }
    void            setProfiles(const Profiles &value);

    const QString &selectedProfileId() const;
    void           setSelectedProfileId(const QString &value);

private:
    Ui::ProfilesPage *ui;
    Profiles          mProfiles;
    Profile           mCurrentProfile;
    QString           mSelectedProfileId;

    void profileListSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void profileItemChanged(QListWidgetItem *item);
};

#endif // PROFILESPAGE_H
