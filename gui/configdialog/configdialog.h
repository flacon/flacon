/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QPointer>
#include "ui_configdialog.h"
#include "profilewidget.h"

class OutFormat;
class ProgramEdit;
class QListWidgetItem;

class ConfigDialog : public QDialog, private Ui::ConfigDialog
{
    Q_OBJECT
public:
    static ConfigDialog *createAndShow(QWidget *parent = nullptr);
    static ConfigDialog *createAndShow(const QString &profileId, QWidget *parent = nullptr);

    using QDialog::show;
    void show(const QString &profileId);

signals:

public slots:
    void done(int res) override;
    void tmpDirShowDialog();

private slots:
    void profileListSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void profileItemChanged(QListWidgetItem *item);
    void deleteProfile();
    void addProfile();

private:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void initGeneralPage();
    void initProgramsPage();
    void initUpdatePage();

    void load();
    void save();

    Profile &currentProfile();
    void     refreshProfilesList(const QString &selectedProfileId);
    void     updateLastUpdateLbl();

    QList<ProgramEdit *>    mProgramEdits;
    Profiles                mProfiles;
    QPointer<ProfileWidget> mProfileWidget;
};

#endif // CONFIGDIALOG_H
