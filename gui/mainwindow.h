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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "types.h"
#include <QPointer>
#include "converter.h"

namespace Ui {
class MainWindow;
}

class Project;
class Converter;
class Scanner;

class MainWindow : public QMainWindow, private Ui::MainWindow, private Messages::Handler
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void addFileOrDir(const QString &fileName);
    void startConvertAll();
    void startConvertSelected();
    void stopConvert();

signals:
    void downloadStarted(Disc *disc);
    void downloadFinished(Disc *disc);

private slots:
    void setPattern();
    void setOutDir();
    void setOutProfile();
    void setCodePage();

    void setControlsEnable();
    void refreshEdits();

    void openAddFileDialog();

    void setCueForDisc(Disc *disc);
    void setAudioForDisc(Disc *disc, int audioFileNum);
    void setCoverImage(Disc *disc);
    void downloadDiscInfo(Disc *disc);

    void removeDiscs();

    void setStartTrackNum();
    void setTrackTag();
    void setDiscTag();
    void setDiscTagInt();

    void configure();
    void configureEncoder();

    void downloadInfo();
    void openScanDialog();

    void openAboutDialog();
    void checkUpdates();

    void trackViewMenu(const QPoint &pos);
    void showDiskAudioFileMenu(Disc *disc, const QPoint &pos);
    void openEditTagsDialog();

protected:
    void closeEvent(QCloseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

private:
    QPointer<Conv::Converter> mConverter;
    Scanner                  *mScanner;
    QString                   getOpenFileFilter(bool includeAudio, bool includeCue);

    void polishView();
    void initActions();
    void refreshOutProfileCombo();

    void loadSettings();
    void saveSettings();

    void startConvert(const Conv::Converter::Jobs &jobs);

    QIcon loadMainIcon();

    void showErrorMessage(const QString &message) override;
    void fillAudioMenu(Disc *disc, QMenu &menu);
};

#endif // MAINWINDOW_H
