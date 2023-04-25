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

#ifndef PROFILESPAGE_H
#define PROFILESPAGE_H

#include <QWidget>
#include <QListWidget>
#include "profiles.h"

class QListWidgetItem;

namespace Ui {
class ProfilesPage;
}

class ProfileListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ProfileListWidget(QWidget *parent = nullptr);
    using QListWidget::QListWidget;

    void refresh(const Profiles &profiles);

    QString currentId() const;
    void    setCurrentId(const QString &id);

    QString rowId(int row) const;

signals:
    void currentProfileChanged(const QString &id);
};

class ProfilesPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProfilesPage(QWidget *parent = nullptr);
    ~ProfilesPage();

    Profiles profiles() const;
    void     setProfiles(const Profiles &profiles);

    void     selectProfile(const QString &profileId);
    Profile *currentProfile() { return mProfile; }

    void addProfile();

private:
    Ui::ProfilesPage *ui;

    Profiles mProfiles;
    Profile *mProfile = nullptr;

    void renameProfile(QListWidgetItem *item);

    void deleteProfile();
};

#endif // PROFILESPAGE_H
