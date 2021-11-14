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
