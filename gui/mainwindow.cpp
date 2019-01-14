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


#include "mainwindow.h"
#include "project.h"
#include "disk.h"
#include "settings.h"
#include "converter/converter.h"
#include "outformat.h"
#include "inputaudiofile.h"
#include "formats/format.h"
#include "configdialog/configdialog.h"
#include "aboutdialog/aboutdialog.h"
#include "cuediskselectdialog/cuediskselectdialog.h"
#include "scanner.h"
#include "gui/coverdialog/coverdialog.h"
#include "internet/dataprovider.h"
#include "gui/trackviewmodel.h"
#include "gui/tageditor/tageditor.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QQueue>
#include <QKeyEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QToolBar>
#include <QToolButton>
#include <QStandardPaths>

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

/************************************************

 ************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mScanner(nullptr)
{
    Messages::setHandler(this);

    setupUi(this);

    qApp->setWindowIcon(loadMainIcon());
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(24,24));
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);

#ifdef Q_OS_MAC
    this->setUnifiedTitleAndToolBarOnMac(true);
    setWindowIcon(QIcon());

    trackView->setFrameShape(QFrame::NoFrame);
    splitter->setStyleSheet("::handle{ border-right: 1px solid #b6b6b6;}");
#endif

    setAcceptDrops(true);
    setAcceptDrops(true);
    this->setContextMenuPolicy(Qt::NoContextMenu);

    outPatternButton->setToolTip(outPatternEdit->toolTip());
    outDirEdit->setToolTip(actionSelectResultDir->toolTip());

    // TrackView ...............................................
    trackView->setRootIsDecorated(false);
    trackView->setItemsExpandable(false);
    trackView->hideColumn((int)TrackView::ColumnComment);
    trackView->setAlternatingRowColors(false);

    // Tag edits ...............................................
    tagGenreEdit->setTagId(TagId::Genre);
    connect(tagGenreEdit, SIGNAL(textEdited(QString)), this, SLOT(setTrackTag()));

    tagYearEdit->setTagId(TagId::Date);
    connect(tagYearEdit, SIGNAL(textEdited(QString)), this, SLOT(setTrackTag()));

    tagArtistEdit->setTagId(TagId::Artist);
    connect(tagArtistEdit, SIGNAL(textEdited(QString)), this, SLOT(setTrackTag()));
    connect(tagArtistEdit, SIGNAL(textEdited(QString)), this, SLOT(refreshEdits()));

    tagDiskPerformerEdit->setTagId(TagId::AlbumArtist);
    connect(tagDiskPerformerEdit, SIGNAL(textEdited(QString)), this, SLOT(setDiskTag()));
    connect(tagDiskPerformerEdit, SIGNAL(textEdited(QString)), this, SLOT(refreshEdits()));

    tagAlbumEdit->setTagId(TagId::Album);
    connect(tagAlbumEdit, SIGNAL(textEdited(QString)), this, SLOT(setTrackTag()));


    tagDiscIdEdit->setTagId(TagId::DiscId);
    connect(tagStartNumEdit, SIGNAL(editingFinished()), this, SLOT(setStartTrackNum()));
    connect(tagStartNumEdit, SIGNAL(valueChanged(int)), this, SLOT(setStartTrackNum()));

    connect(trackView->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(refreshEdits()));
    connect(trackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(trackViewMenu(QPoint)));

    connect(editTagsButton, &QPushButton::clicked, this, &MainWindow::openEditTagsDialog);

    initActions();

    // Buttons .................................................
    outDirButton->setAutoRaise(true);
    outDirButton->setStyleSheet("border: none;");
    initOutDirButton();

    configureEncoderBtn->setDefaultAction(actionConfigureEncoder);
    configureEncoderBtn->setAutoRaise(true);
    configureEncoderBtn->setStyleSheet("border: none;");

    outPatternButton->addPattern("%n", tr("Insert \"Track number\""));
    outPatternButton->addPattern("%N", tr("Insert \"Total number of tracks\""));
    outPatternButton->addPattern("%a", tr("Insert \"Artist\""));
    outPatternButton->addPattern("%A", tr("Insert \"Album title\""));
    outPatternButton->addPattern("%t", tr("Insert \"Track title\""));
    outPatternButton->addPattern("%y", tr("Insert \"Year\""));
    outPatternButton->addPattern("%g", tr("Insert \"Genre\""));

    const QString patterns[] = {
        "%a/{%y - }%A/%n - %t",
        "%a -{ %y }%A/%n - %t",
        "{%y }%A - %a/%n - %t",
        "%a/%A/%n - %t",
        "%a - %A/%n - %t",
        "%A - %a/%n - %t" };

    for (QString pattern: patterns)
    {
        outPatternButton->addFullPattern(pattern,
                                         tr("Use \"%1\"", "Predefined out file pattern, string like 'Use \"%a/%A/%n - %t\"'")
                                         .arg(pattern)
                                         + "  ( " + patternExample(pattern)  + ".flac )");
    }

    outPatternButton->menu()->addSeparator();

    outPatternEdit->deleteItemAction()->setText(tr("Delete current pattern from history"));
    outPatternButton->menu()->addAction(outPatternEdit->deleteItemAction());

    connect(outPatternButton, SIGNAL(paternSelected(QString)),
            this, SLOT(insertOutPattern(QString)));

    connect(outPatternButton, SIGNAL(fullPaternSelected(QString)),
            this, SLOT(replaceOutPattern(QString)));

    outPatternButton->setIcon(loadIcon("pattern-button"));

    // Format combo ............................................
    initOutFormatCombo();

    loadSettings();

    outDirEdit->setHistory(settings->value(Settings::OutFiles_DirectoryHistory).toStringList());
    outDirEdit->setCurrentText(settings->value(Settings::OutFiles_Directory).toString());

    outPatternEdit->setHistory(settings->value(Settings::OutFiles_PatternHistory).toStringList());

    // Signals .................................................
    connect(settings, SIGNAL(changed()), trackView->model(), SIGNAL(layoutChanged()));

    connect(outPatternEdit->lineEdit(), SIGNAL(editingFinished()), this, SLOT(setPattern()));
    connect(outPatternEdit, SIGNAL(currentIndexChanged(int)), this, SLOT(setPattern()));

    connect(outDirEdit->lineEdit(),     SIGNAL(editingFinished()), this, SLOT(setOutDir()));
    connect(outDirEdit,     SIGNAL(currentIndexChanged(int)), this, SLOT(setOutDir()));

    connect(outFormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutFormat()));
    connect(codepageCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCodePage()));

    connect(trackView, SIGNAL(selectCueFile(Disk*)),     this, SLOT(setCueForDisc(Disk*)));
    connect(trackView, SIGNAL(selectAudioFile(Disk*)),   this, SLOT(setAudioForDisk(Disk*)));
    connect(trackView, SIGNAL(selectCoverImage(Disk*)),  this, SLOT(setCoverImage(Disk*)));
    connect(trackView, SIGNAL(downloadInfo(Disk*)),      this, SLOT(downloadDiskInfo(Disk*)));

    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(refreshEdits()));
    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(layoutChanged()), trackView, SLOT(layoutChanged()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(diskChanged(Disk*)), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(diskChanged(Disk*)), this, SLOT(setControlsEnable()));


    refreshEdits();
    setControlsEnable();
}


/************************************************
 *
 ************************************************/
void MainWindow::showEvent(QShowEvent *)
{
    if (project->count())
        trackView->selectDisk(project->disk(0));
}


/************************************************

 ************************************************/
MainWindow::~MainWindow()
{

}

/************************************************

 ************************************************/
void MainWindow::closeEvent(QCloseEvent *)
{
    if (mConverter)
        mConverter->stop();
    saveSettings();
}


/************************************************

 ************************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    foreach(QUrl url, event->mimeData()->urls())
    {
        if (url.isLocalFile())
        {
            event->acceptProposedAction();
            return;
        }
    }
}


/************************************************

 ************************************************/
void MainWindow::dropEvent(QDropEvent * event)
{
    foreach(QUrl url, event->mimeData()->urls())
    {
        addFileOrDir(url.toLocalFile());
    }
}


/************************************************

 ************************************************/
void MainWindow::insertOutPattern(const QString &pattern)
{
    outPatternEdit->lineEdit()->insert(pattern);
    setPattern();
}


/************************************************

 ************************************************/
void MainWindow::replaceOutPattern(const QString &pattern)
{
    outPatternEdit->lineEdit()->setText(pattern);
    setPattern();
}


/************************************************

 ************************************************/
void MainWindow::setPattern()
{
    settings->setValue(Settings::OutFiles_Pattern, outPatternEdit->currentText());
    settings->setValue(Settings::OutFiles_PatternHistory, outPatternEdit->history());
}


/************************************************

 ************************************************/
void MainWindow::setOutDir()
{
    settings->setValue(Settings::OutFiles_Directory, outDirEdit->currentText());
    settings->setValue(Settings::OutFiles_DirectoryHistory, outDirEdit->history());
}


/************************************************

 ************************************************/
void MainWindow::openOutDirDialog()
{
    QString outDir = QFileDialog::getExistingDirectory(this, tr("Select result directory"), outDirEdit->currentText());
    if (!outDir.isEmpty())
    {
        outDir.replace(QDir::homePath(), "~");
        outDirEdit->setCurrentText(outDir);
        setOutDir();
    }
}


/************************************************

 ************************************************/
void MainWindow::setCueForDisc(Disk *disk)
{
    QString flt = getOpenFileFilter(false, true);

    QString dir;
    if (!disk->audioFileName().isEmpty())
        dir = QFileInfo(disk->audioFileName()).dir().absolutePath();
    else if (! disk->cueFile().isEmpty())
        dir = QFileInfo(disk->cueFile()).dir().absolutePath();
    else
        dir = settings->value(Settings::Misc_LastDir).toString();


    QString fileName = QFileDialog::getOpenFileName(this, tr("Select CUE file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    try
    {
        CueReader reader;
        QVector<CueDisk> cue = reader.load(fileName);

        int diskNum = 0;
        if (cue.count() > 1)
        {
            int proposal = 0;
            for (int i=0; i<cue.count(); ++i)
            {
                if (!project->diskExists(cue.at(i).uri()))
                {
                    proposal = i;
                    break;
                }
            }

            diskNum = CueDiskSelectDialog::getDiskNumber(cue, proposal);
            if (diskNum < 0)
                return;
        }

        disk->loadFromCue(cue.at(diskNum));
    }
    catch (FlaconError &err)
    {
        Messages::error(err.message());
    }
}


/************************************************

 ************************************************/
void MainWindow::setOutFormat()
{
    int n = outFormatCombo->currentIndex();
    if (n > -1)
    {
        settings->setValue(Settings::OutFiles_Format, outFormatCombo->itemData(n));
    }
}


/************************************************

 ************************************************/
void MainWindow::setControlsEnable()
{
    bool running = mConverter && mConverter->isRunning();

    if (running)
    {
        outFilesBox->setEnabled(false);
        tagsBox->setEnabled(false);

        actionAddDisk->setEnabled(false);
        actionRemoveDisc->setEnabled(false);
        actionStartConvert->setVisible(false);
        actionAbortConvert->setVisible(true);
        actionDownloadTrackInfo->setEnabled(false);
        actionScan->setEnabled(false);
        actionConfigure->setEnabled(false);
        actionConfigureEncoder->setEnabled(false);
    }
    else
    {
        QList<Disk*> selectedDisks = trackView->selectedDisks();

        bool tracksSelected = trackView->selectedTracks().count() > 0;
        bool discsSelected  = trackView->selectedDisks().count() > 0;
        bool canConvert = Converter::canConvert();
        bool canDownload = false;
        foreach (const Disk *disk, selectedDisks)
            canDownload = canDownload || !disk->discId().isEmpty();

        outFilesBox->setEnabled(true);
        tagsBox->setEnabled(tracksSelected);

        actionAddDisk->setEnabled(true);
        actionRemoveDisc->setEnabled(discsSelected);
        actionStartConvert->setVisible(true);
        actionAbortConvert->setVisible(false);
        actionStartConvert->setEnabled(canConvert);
        actionDownloadTrackInfo->setEnabled(canDownload);
        actionScan->setEnabled(!mScanner);
        actionConfigure->setEnabled(true);
        actionConfigureEncoder->setEnabled(OutFormat::currentFormat()->hasConfigPage());
    }
}


/************************************************

 ************************************************/
void MainWindow::refreshEdits()
{

    // Disks ...............................
    QSet<int> startNums;
    QSet<QString> diskId;
    QSet<QString> codePage;

    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        startNums << disk->startTrackNum();
        diskId << disk->discId();
        codePage << disk->codecName();
    }

    // Tracks ..............................
    QSet<QString> genre;
    QSet<QString> artist;
    QSet<QString> album;
    QSet<QString> date;
    QSet<QString> diskPerformer;

    QList<Track*> tracks = trackView->selectedTracks();
    foreach(Track *track, tracks)
    {
        genre << track->genre();
        artist << track->artist();
        album << track->album();
        date << track->date();
        diskPerformer << track->tag(TagId::AlbumArtist);
    }

    tagGenreEdit->setMultiValue(genre);
    tagYearEdit->setMultiValue(date);
    tagArtistEdit->setMultiValue(artist);
    tagAlbumEdit->setMultiValue(album);
    tagStartNumEdit->setMultiValue(startNums);
    tagDiscIdEdit->setMultiValue(diskId);
    codepageCombo->setMultiValue(codePage);
    tagDiskPerformerEdit->setMultiValue(diskPerformer);

    if (outDirEdit->currentText() != settings->value(Settings::OutFiles_Directory).toString())
        outDirEdit->lineEdit()->setText(settings->value(Settings::OutFiles_Directory).toString());

    if (outPatternEdit->currentText() != settings->value(Settings::OutFiles_Pattern).toString())
        outPatternEdit->lineEdit()->setText(settings->value(Settings::OutFiles_Pattern).toString());

    int n = outFormatCombo->findData(settings->value(Settings::OutFiles_Format).toString());
    if (n > -1)
        outFormatCombo->setCurrentIndex(n);
    else
        outFormatCombo->setCurrentIndex(0);
}


/************************************************

 ************************************************/
void MainWindow::setCodePage()
{
    int n = codepageCombo->currentIndex();
    if (n > -1)
    {
        QString codepage = codepageCombo->itemData(n).toString();

        QList<Disk*> disks = trackView->selectedDisks();
        foreach(Disk *disk, disks)
            disk->setCodecName(codepage);

        settings->setValue(Settings::Tags_DefaultCodepage, codepage);
    }
}


/************************************************

 ************************************************/
void MainWindow::setTrackTag()
{
    TagLineEdit *edit = qobject_cast<TagLineEdit*>(sender());
    if (!edit)
        return;

    QList<Track*> tracks = trackView->selectedTracks();
    foreach(Track *track, tracks)
    {
        track->setTag(edit->tagId(), edit->text());
        trackView->update(*track);
    }
}


/************************************************
 *
 ************************************************/
void MainWindow::setDiskTag()
{
    TagLineEdit *edit = qobject_cast<TagLineEdit*>(sender());
    if (!edit)
        return;

    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        disk->setDiskTag(edit->tagId(), edit->text());
        trackView->update(*disk);
    }
}


/************************************************
 *
 ************************************************/
void MainWindow::setDiskTagInt()
{
    TagSpinBox *spinBox = qobject_cast<TagSpinBox*>(sender());
    if (!spinBox)
        return;

    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        disk->setDiskTag(spinBox->tagId(), QString::number(spinBox->value()));
        trackView->update(*disk);
    }
}


/************************************************

 ************************************************/
void MainWindow::startConvert()
{
    trackView->setFocus();

    bool ok = true;
    for (int i=0; i< project->count(); ++i)
    {
        ok = ok && project->disk(i)->canConvert();
    }

    if (!ok)
    {
        int res = QMessageBox::warning(this,
                                       windowTitle(),
                                       tr("Some albums will not be converted, they contain errors.\nDo you want to continue?"),
                                       QMessageBox::Ok | QMessageBox::Cancel);
        if (res !=  QMessageBox::Ok)
            return;
    }

    for(int d=0; d<project->count(); ++d)
    {
        Disk *disk = project->disk(d);
        for (int t=0; t<disk->count(); ++t)
            trackView->model()->trackProgressChanged(*disk->track(t), TrackState::NotRunning, 0);
    }

    trackView->setColumnWidth(TrackView::ColumnPercent, 200);
    mConverter = new Converter();
    connect(mConverter, SIGNAL(finished()),
            this, SLOT(setControlsEnable()));

    connect(mConverter, SIGNAL(finished()),
            mConverter, SLOT(deleteLater()));

    connect(mConverter,         SIGNAL(trackProgress(Track,TrackState,Percent)),
            trackView->model(), SLOT(trackProgressChanged(Track,TrackState,Percent)));


    mConverter->start();
    setControlsEnable();
}


/************************************************

 ************************************************/
void MainWindow::stopConvert()
{
    if (mConverter)
        mConverter->stop();
    setControlsEnable();
}


/************************************************

 ************************************************/
void MainWindow::configure()
{
    ConfigDialog::createAndShow(0, this);
}


/************************************************

 ************************************************/
void MainWindow::configureEncoder()
{
    ConfigDialog::createAndShow(OutFormat::currentFormat(), this);
}


/************************************************

 ************************************************/
void MainWindow::downloadInfo()
{
    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        this->downloadDiskInfo(disk);
    }
}


/************************************************

 ************************************************/
QString MainWindow::getOpenFileFilter(bool includeAudio, bool includeCue)
{
    QStringList flt;
    QStringList allFlt;
    QString fltPattern = tr("%1 files", "OpenFile dialog filter line, like \"WAV files\"") + " (*.%2)";

    if (includeAudio)
    {
        foreach(const AudioFormat *format, AudioFormat::inputFormats())
        {
            allFlt << QString(" *.%1").arg(format->ext());
            flt << fltPattern.arg(format->name(), format->ext());
        }
    }

    flt.sort();

    if (includeCue)
    {
        allFlt << QString("*.cue");
        flt.insert(0, fltPattern.arg("CUE", "cue"));
    }

    if (allFlt.count()  > 1)
        flt.insert(0, tr("All supported formats", "OpenFile dialog filter line") + " (" + allFlt.join(" ")  + ")");

    flt << tr("All files", "OpenFile dialog filter line like \"All files\"") + " (*)";

    return flt.join(";;");
}


/************************************************
 *
 ************************************************/
void MainWindow::initOutDirButton()
{
    outDirButton->setIcon(loadIcon("pattern-button"));
    QMenu *menu = outDirButton->menu();

    menu->addAction(actionSelectResultDir);
    menu->addSeparator();

    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (!dir.isEmpty() && dir != QDir::homePath())
        {
            QAction *act = new QAction(menu);
            act->setText(tr("Standard music location", "Menu item for output direcory button"));
            connect(act, &QAction::triggered, [this, dir](){ outDirEdit->setEditText(dir); setOutDir();});
            menu->addAction(act);
        }
    }

    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        if (!dir.isEmpty())
        {
            QAction *act = new QAction(menu);
            act->setText(tr("Desktop", "Menu item for output direcory button"));
            connect(act, &QAction::triggered, [this, dir](){ outDirEdit->setCurrentText(dir); setOutDir();});
            menu->addAction(act);
        }
    }

    {
        QString s = tr("Same directory as CUE file", "Menu item for output direcory button");
        QAction *act = new QAction(s, menu);
        outDirEdit->lineEdit()->setPlaceholderText(s);
        connect(act, &QAction::triggered, [this](){ outDirEdit->setCurrentText(""); setOutDir();});
        outDirEdit->lineEdit()->editingFinished();
        menu->addAction(act);
    }

    menu->addSeparator();
    QAction * act = outDirEdit->deleteItemAction();
    act->setText(tr("Remove current directory from history"));
    menu->addAction(act);
}


/************************************************

 ************************************************/
void MainWindow::openAddFileDialog()
{
    QString flt = getOpenFileFilter(true, true);
    QString lastDir = settings->value(Settings::Misc_LastDir).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add CUE or audio file", "OpenFile dialog title"), lastDir, flt);

    foreach(const QString &fileName, fileNames)
    {
        settings->setValue(Settings::Misc_LastDir, QFileInfo(fileName).dir().path());
        addFileOrDir(fileName);
    }
}


/************************************************

 ************************************************/
void MainWindow::setAudioForDisk(Disk *disk)
{
    QString flt = getOpenFileFilter(true, false);

    QString dir;

    if (!disk->cueFile().isEmpty())
        dir = QFileInfo(disk->cueFile()).dir().absolutePath();
    else if (! disk->audioFileName().isEmpty())
        dir = QFileInfo(disk->audioFileName()).dir().absolutePath();
    else
        dir = settings->value(Settings::Misc_LastDir).toString();


    QString fileName = QFileDialog::getOpenFileName(this, tr("Select audio file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    InputAudioFile audio(fileName);
    if (audio.isValid())
        disk->setAudioFile(audio);
    else
        Messages::error(audio.errorString());
}


/************************************************
 *
 ************************************************/
void MainWindow::setCoverImage(Disk *disk)
{
    CoverDialog::createAndShow(disk, this);
}


/************************************************
 *
 ************************************************/
void MainWindow::downloadDiskInfo(Disk *disk)
{
    if (!disk->canDownloadInfo())
        return;

    DataProvider *provider = new FreeDbProvider(*disk);
    connect(provider, SIGNAL(finished()), provider, SLOT(deleteLater()));
    connect(provider, &DataProvider::finished,
            [disk, this]() { trackView->downloadFinished(*disk); });

    connect(provider, &DataProvider::ready,
            [disk](const QVector<Tracks> data) { disk->addTagSets(data); });

    provider->start();
    trackView->downloadStarted(*disk);
}


/************************************************

 ************************************************/
void MainWindow::addFileOrDir(const QString &fileName)
{
    DiskList addedDisks;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFileInfo fi = QFileInfo(fileName);
    if (fi.isDir())
    {
        mScanner = new Scanner;
        setControlsEnable();
        mScanner->start(fi.absoluteFilePath());
        delete mScanner;
        mScanner = 0;
        setControlsEnable();
    }
    else if (fi.size() > 102400)
    {
        addedDisks << project->addAudioFile(fileName, true);
    }
    else
    {
        addedDisks << project->addCueFile(fileName, true);
    }
    QApplication::restoreOverrideCursor();

    if (!addedDisks.isEmpty())
        this->trackView->selectDisk(addedDisks.first());
}


/************************************************

 ************************************************/
void MainWindow::removeDisks()
{
    QList<Disk*> disks = trackView->selectedDisks();
    project->removeDisk(&disks);
    setControlsEnable();
}


/************************************************

 ************************************************/
void MainWindow::openScanDialog()
{
    QString lastDir = settings->value(Settings::Misc_LastDir).toString();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), lastDir);

    if (!dir.isEmpty())
    {
        settings->setValue(Settings::Misc_LastDir, dir);
        addFileOrDir(dir);
    }
}


/************************************************

 ************************************************/
void MainWindow::openAboutDialog()
{
    AboutDialog dlg;
    dlg.exec();
}


/************************************************

 ************************************************/
void MainWindow::checkUpdates()
{
#ifdef MAC_UPDATER
    Updater::sharedUpdater().checkForUpdatesInBackground();
#endif
}


/************************************************
 *
 ************************************************/
void MainWindow::trackViewMenu(const QPoint &pos)
{
    QModelIndex index = trackView->indexAt(pos);
    if (!index.isValid())
        return;


    Disk *disk = trackView->model()->diskByIndex(index);
    if (!disk)
        return;


    QMenu menu;
    QAction *act = new QAction(tr("Edit tags…", "context menu"), &menu);
    connect(act, &QAction::triggered, this, &MainWindow::openEditTagsDialog);
    menu.addAction(act);

    menu.addSeparator();

    act = new QAction(tr("Select another audio file…", "context menu"), &menu);
    connect(act, &QAction::triggered, [this, disk](){ this->setAudioForDisk(disk); });
    menu.addAction(act);


    act = new QAction(tr("Select another CUE file…", "context menu"), &menu);
    connect(act, &QAction::triggered, [this, disk](){ this->setCueForDisc(disk); });
    menu.addAction(act);

    act = new QAction(tr("Get data from CDDB", "context menu"), &menu);
    act->setEnabled(disk->canDownloadInfo());
    connect(act, &QAction::triggered, [this, disk](){ this->downloadDiskInfo(disk);});
    menu.addAction(act);

    menu.exec(trackView->viewport()->mapToGlobal(pos));
}


/************************************************
 *
 ************************************************/
void MainWindow::openEditTagsDialog()
{
    TagEditor editor(trackView->selectedTracks(), trackView->selectedDisks());
    editor.exec();
    refreshEdits();
}


/************************************************

 ************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (mScanner)
            mScanner->stop();
    }
}


/************************************************

 ************************************************/
bool MainWindow::event(QEvent *event)
{
    switch (event->type())
    {
#ifdef Q_OS_MAC
    case QEvent::WindowActivate:
        toolBar->setEnabled(true);
        break;

    case QEvent::WindowDeactivate:
        toolBar->setEnabled(false);
        break;
#endif
    default:
        break;
    }
    return QMainWindow::event(event);
}


/************************************************

 ************************************************/
void MainWindow::setStartTrackNum()
{
    if (!tagStartNumEdit->isModified())
        return;

    int value = tagStartNumEdit->value();
    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        disk->setStartTrackNum(value);
    }
}


/************************************************

 ************************************************/
void MainWindow::initActions()
{
    actionAddDisk->setIcon(loadIcon("add-disk"));
    connect(actionAddDisk, SIGNAL(triggered()), this, SLOT(openAddFileDialog()));

    actionRemoveDisc->setIcon(loadIcon("remove-disk"));
    connect(actionRemoveDisc, SIGNAL(triggered()), this, SLOT(removeDisks()));

    actionScan->setIcon(loadIcon("scan"));
    connect(actionScan, SIGNAL(triggered()), this, SLOT(openScanDialog()));

    actionDownloadTrackInfo->setIcon(loadIcon("download-info"));
    connect(actionDownloadTrackInfo, SIGNAL(triggered()), this, SLOT(downloadInfo()));


    actionStartConvert->setIcon(loadIcon("start-convert"));
    connect(actionStartConvert, SIGNAL(triggered()), this, SLOT(startConvert()));

    actionAbortConvert->setIcon(loadIcon("abort-convert"));
    connect(actionAbortConvert, SIGNAL(triggered()), this, SLOT(stopConvert()));

    actionSelectResultDir->setIcon(loadIcon("folder"));
    connect(actionSelectResultDir, SIGNAL(triggered()), this, SLOT(openOutDirDialog()));

    actionConfigure->setIcon(loadIcon("configure"));
    connect(actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));
    actionConfigure->setMenuRole(QAction::PreferencesRole);

    actionConfigureEncoder->setIcon(actionConfigure->icon());
    connect(actionConfigureEncoder, SIGNAL(triggered()), this, SLOT(configureEncoder()));

    connect(actionAbout, SIGNAL(triggered()), this,  SLOT(openAboutDialog()));
    actionAbout->setMenuRole(QAction::AboutRole);

    int w = 0;
    foreach (QAction *act, toolBar->actions())
    {
        QToolButton *btn = qobject_cast<QToolButton*>(toolBar->widgetForAction(act));
        if (btn)
            w = qMax(w, btn->sizeHint().width());
    }

    foreach (QAction *act, toolBar->actions())
    {
        QToolButton *btn = qobject_cast<QToolButton*>(toolBar->widgetForAction(act));
        if (btn)
            btn->setMinimumWidth(w);
    }

#ifdef MAC_UPDATER
    actionUpdates->setVisible(true);
    actionUpdates->setMenuRole(QAction::ApplicationSpecificRole);

    connect(actionUpdates, &QAction::triggered,
            &Updater::sharedUpdater(), &Updater::checkForUpdatesInBackground);
#else
    actionUpdates->setVisible(false);
#endif
}


/************************************************

 ************************************************/
void MainWindow::initOutFormatCombo()
{
    QList<OutFormat*> formats = OutFormat::allFormats();
    foreach (OutFormat *format, formats)
    {
        outFormatCombo->addItem(format->name(), format->id());
    }
}


/************************************************
  Load settings
 ************************************************/
void MainWindow::loadSettings()
{
     // MainWindow geometry
    int width = settings->value(Settings::MainWindow_Width,  QVariant(987)).toInt();
    int height = settings->value(Settings::MainWindow_Height, QVariant(450)).toInt();
    this->resize(width, height);

    splitter->restoreState(settings->value("MainWindow/Splitter").toByteArray());
    trackView->header()->restoreState(settings->value("MainWindow/TrackView").toByteArray());
}


/************************************************
  Write settings
 ************************************************/
void MainWindow::saveSettings()
{
     settings->setValue("MainWindow/Width",     QVariant(size().width()));
     settings->setValue("MainWindow/Height",    QVariant(size().height()));
     settings->setValue("MainWindow/Splitter",  QVariant(splitter->saveState()));
     settings->setValue("MainWindow/TrackView", QVariant(trackView->header()->saveState()));
}


/************************************************
 *
 ************************************************/
QIcon MainWindow::loadMainIcon()
{
    if (QIcon::themeName() == "hicolor")
    {
        QStringList failback;
        failback << "oxygen";
        failback << "Tango";
        failback << "Prudence-icon";
        failback << "Humanity";
        failback << "elementary";
        failback << "gnome";


        QDir usrDir("/usr/share/icons/");
        QDir usrLocalDir("/usr/local/share/icons/");
        foreach (QString s, failback)
        {
            if (usrDir.exists(s) || usrLocalDir.exists(s))
            {
                QIcon::setThemeName(s);
                break;
            }
        }
    }

    return QIcon::fromTheme("flacon", loadIcon("mainicon", false));
}


/************************************************
 *
 ************************************************/
void MainWindow::showErrorMessage(const QString &message)
{
    QString msg = message;
    msg.replace("\n", "<br>");

    QMessageBox *box = new QMessageBox(this);
    box->setIcon(QMessageBox::Critical);
    box->setWindowTitle(QObject::tr("Flacon", "Error"));
    box->setText("<html>" + msg + "</html>");
    box->setStandardButtons(QMessageBox::Ok);
    box->setAttribute(Qt::WA_DeleteOnClose, true);

    box->show();
}
