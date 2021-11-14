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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QMainWindow>
#include <QCloseEvent>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QMainWindow
{
    Q_OBJECT

public:
    static PreferencesDialog *createAndShow(QWidget *parent = nullptr);
    static PreferencesDialog *createAndShow(const QString &profileId, QWidget *parent = nullptr);

    static void fixLayout(const QWidget *parent);

signals:
    void finished();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::PreferencesDialog *ui;

    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void load();
    void save();

    void initToolBar();
    void showProfile(const QString &profileId);
    void done(bool accept);
};

#endif // PREFERENCESDIALOG_H
