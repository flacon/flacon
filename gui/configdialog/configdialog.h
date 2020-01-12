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
#include "ui_configdialog.h"

class OutFormat;
class EncoderConfigPage;
class ProgramEdit;
class QListWidgetItem;

class ConfigDialog : public QDialog, private Ui::ConfigDialog
{
    Q_OBJECT
public:
    static ConfigDialog *createAndShow(const OutFormat *format, QWidget *parent = nullptr);

signals:
    
public slots:
    void setPage(int pageIndex);
    void setPage(const OutFormat *format);

    void done(int res);
    void tmpDirShowDialog();

private slots:
    void profileListSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void profileItemChanged(QListWidgetItem *item);

private:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void initGeneralPage();
    void initPrograms();
    void initUpdatePage();

    void load();
    void save();

    CoverMode coverMode() const;
    void setCoverMode(CoverMode mode);

    void fillProfilesList();
    void updateLastUpdateLbl();

    QList<ProgramEdit*> mProgramEdits;
    Profiles mProfiles;
    EncoderConfigPage *mEncoderPage = nullptr;
};

#endif // CONFIGDIALOG_H
