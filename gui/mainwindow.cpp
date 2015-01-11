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
#include "configdialog/configdialog.h"
#include "aboutdialog/aboutdialog.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QQueue>
#include <QKeyEvent>
class DirScanner
{
public:
    explicit DirScanner();
    void start(const QString &startDir);
    void stop() { mAbort = true; }

private:
    bool mActive;
    bool mAbort;
};

/************************************************

 ************************************************/
DirScanner::DirScanner():
    mActive(false),
    mAbort(false)
{
}


/************************************************

 ************************************************/
void DirScanner::start(const QString &startDir)
{
    mActive = true;
    mAbort = false;

    QStringList exts;
    foreach(InputAudioFormat format, InputAudioFormat::allFormats())
        exts << QString("*.%1").arg(format.ext());


    QQueue<QString> query;
    query << startDir;

    QSet<QString> processed;
    while (!query.isEmpty())
    {
        QDir dir(query.dequeue());

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        foreach(QFileInfo d, dirs)
        {
            qApp->processEvents();
            if (mAbort)
                return;

            if (d.isSymLink())
                d = QFileInfo(d.symLinkTarget());

            if (!processed.contains(d.absoluteFilePath()))
            {
                processed << d.absoluteFilePath();
                query << d.absoluteFilePath();
            }
        }

        QFileInfoList files = dir.entryInfoList(exts, QDir::Files | QDir::Readable);
        foreach(QFileInfo f, files)
        {
            qApp->processEvents();
            if (mAbort)
                return;

            project->addAudioFile(f.absoluteFilePath());

        }
    }
}


/************************************************

 ************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mConverter(new Converter(this)),
    mScanner(0)
{
    setupUi(this);

    setWindowIcon(QIcon::fromTheme("flacon"));
    setAcceptDrops(true);
    setAcceptDrops(true);

    outPatternButton->setToolTip(outPatternEdit->toolTip());
    outDirEdit->setToolTip(actionSelectResultDir->toolTip());

    // TrackView ...............................................
    trackView->setRootIsDecorated(false);
    trackView->setItemsExpandable(false);
    trackView->hideColumn((int)TrackView::ColumnComment);


    // Tag edits ...............................................
    tagGenreEdit->setTagName(TAG_GENRE);
    connect(tagGenreEdit, SIGNAL(editingFinished()), this, SLOT(setTrackTag()));

    tagYearEdit->setTagName(TAG_DATE);
    connect(tagYearEdit, SIGNAL(editingFinished()), this, SLOT(setTrackTag()));

    tagArtistEdit->setTagName(TAG_PERFORMER);
    connect(tagArtistEdit, SIGNAL(editingFinished()), this, SLOT(setTrackTag()));

    tagAlbumEdit->setTagName(TAG_ALBUM);
    connect(tagAlbumEdit, SIGNAL(editingFinished()), this, SLOT(setTrackTag()));

    tagDiscIdEdit->setTagName(TAG_DISCID);

    connect(tagStartNumEdit, SIGNAL(editingFinished()), this, SLOT(setStartTrackNum()));
    connect(tagStartNumEdit, SIGNAL(valueChanged(int)), this, SLOT(setStartTrackNum()));

    initActions();

    // Buttons .................................................
    outDirButton->setDefaultAction(actionSelectResultDir);
    configureEncoderBtn->setDefaultAction(actionConfigureEncoder);

    outPatternButton->addPattern("%n", tr("Insert \"Track number\""));
    outPatternButton->addPattern("%N", tr("Insert \"Total number of tracks\""));
    outPatternButton->addPattern("%a", tr("Insert \"Artist\""));
    outPatternButton->addPattern("%A", tr("Insert \"Album title\""));
    outPatternButton->addPattern("%t", tr("Insert \"Track title\""));
    outPatternButton->addPattern("%y", tr("Insert \"Year\""));
    outPatternButton->addPattern("%g", tr("Insert \"Genre\""));

    QString pattern;

    pattern = "%a/{%y - }%A/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));

    pattern = "%a -{ %y }%A/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));

    pattern = "{%y }%A - %a/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));

    pattern = "%a/%A/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));

    pattern = "%a - %A/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));

    pattern = "%A - %a/%n - %t";
    outPatternButton->addFullPattern(pattern, tr("Use \"%1\"", "Predefined out file pattern").arg(pattern));


    outPatternButton->setFixedWidth(outDirButton->sizeHint().width());
    connect(outPatternButton, SIGNAL(paternSelected(QString)),
            this, SLOT(insertOutPattern(QString)));

    connect(outPatternButton, SIGNAL(fullPaternSelected(QString)),
            this, SLOT(replaceOutPattern(QString)));
    outPatternEdit->setAutoCompletionCaseSensitivity(Qt::CaseSensitive);

    // Format combo ............................................
    initOutFormatCombo();

    loadSettings();

    // Signals .................................................
    connect(settings, SIGNAL(changed()), trackView->model(), SIGNAL(layoutChanged()));
    connect(outPatternEdit->lineEdit(), SIGNAL(editingFinished()), this, SLOT(setPattern()));
    connect(outPatternEdit, SIGNAL(currentIndexChanged(int)), this, SLOT(setPattern()));
    connect(outDirEdit,     SIGNAL(editingFinished()), this, SLOT(setOutDir()));

    connect(outFormatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutFormat()));
    connect(codepageCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCodePage()));

    connect(mConverter, SIGNAL(finished()), this, SLOT(setControlsEnable()));
    connect(mConverter, SIGNAL(error(QString)), this, SLOT(showErrorMessage(QString)));

    connect(trackView, SIGNAL(selectCueFile(Disk*)), this, SLOT(setCueForDisc(Disk*)));
    connect(trackView, SIGNAL(selectAudioFile(Disk*)), this, SLOT(setAudioForDisk(Disk*)));

    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(refreshEdits()));
    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(layoutChanged()), trackView, SLOT(layoutChanged()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(diskChanged(Disk*)), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(diskChanged(Disk*)), this, SLOT(setControlsEnable()));

    outPatternEdit->setHistory(settings->value(Settings::OutFiles_PatternHistory).toStringList());

    refreshEdits();
    setControlsEnable();
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
    mConverter->stop();
    saveSettings();
}


/************************************************

 ************************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    foreach(QUrl url, event->mimeData()->urls())
    {
#if (QT_VERSION < QT_VERSION_CHECK(4, 8, 0))
        QString scheme = url.scheme();
        if (scheme.isEmpty() || scheme == "file")
        {
            event->acceptProposedAction();
            return;
        }
#else
        if (url.isLocalFile())
        {
            event->acceptProposedAction();
            return;
        }
#endif
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
    settings->setValue(Settings::OutFiles_Directory, outDirEdit->text());
}


/************************************************

 ************************************************/
void MainWindow::openOutDirDialog()
{
    QString outDir = QFileDialog::getExistingDirectory(this, tr("Select result directory"), outDirEdit->text());
    if (!outDir.isEmpty())
    {
        outDir.replace(QDir::homePath(), "~");
        outDirEdit->setText(outDir);
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
    {
        dir = QFileInfo(disk->audioFileName()).dir().absolutePath();
    }
    else if (! disk->cueFile().isEmpty())
    {
        dir = QFileInfo(disk->cueFile()).dir().absolutePath();
    }
    else
    {
        dir = settings->value(Settings::Misc_LastDir).toString();
    }


    QString fileName = QFileDialog::getOpenFileName(this, tr("Select CUE file", "OpenFile dialog title"), dir, flt);

    if (!fileName.isEmpty())
    {
        disk->loadFromCue(fileName, true);
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
    bool running = mConverter->isRunning();

    if (running)
    {
        outFilesBox->setEnabled(false);
        tagsBox->setEnabled(false);

        actionAddFile->setEnabled(false);
        actionRemoveDisc->setEnabled(false);
        actionStartConvert->setEnabled(false);
        actionAbortConvert->setEnabled(true);
        actionDownloadTrackInfo->setEnabled(false);
        actionScan->setEnabled(false);
        actionConfigure->setEnabled(false);
        actionConfigureEncoder->setEnabled(false);
    }
    else
    {
        bool tracksSelected = trackView->selectedTracks().count() > 0;
        bool discsSelected  = trackView->selectedDisks().count() > 0;
        bool canConvert = mConverter->canConvert();

        outFilesBox->setEnabled(true);
        tagsBox->setEnabled(tracksSelected);

        actionAddFile->setEnabled(true);
        actionRemoveDisc->setEnabled(discsSelected);
        actionStartConvert->setEnabled(canConvert);
        actionAbortConvert->setEnabled(running);
        actionDownloadTrackInfo->setEnabled(tracksSelected);
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
        codePage << disk->textCodecName();
    }

    // Tracks ..............................
    QSet<QString> genre;
    QSet<QString> artist;
    QSet<QString> album;
    QSet<QString> date;

    QList<Track*> tracks = trackView->selectedTracks();
    foreach(Track *track, tracks)
    {
        genre << track->genre();
        artist << track->artist();
        album << track->album();
        date << track->date();
    }

    tagGenreEdit->setMultiValue(genre);
    tagYearEdit->setMultiValue(date);
    tagArtistEdit->setMultiValue(artist);
    tagAlbumEdit->setMultiValue(album);
    tagStartNumEdit->setMultiValue(startNums);
    tagDiscIdEdit->setMultiValue(diskId);
    codepageCombo->setMultiValue(codePage);

    outDirEdit->setText(settings->value(Settings::OutFiles_Directory).toString());

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
            disk->setTextCodecName(codepage);

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
        track->setTag(edit->tagName(), edit->text());
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

    for(int i=0; i<project->count(); ++i)
    {
        Disk *disk = project->disk(i);
        for (int j=0; j<disk->count(); ++j)
            disk->track(j)->setProgress(Track::NotRunning, -1);
    }

    trackView->setColumnWidth(TrackView::ColumnPercent, 200);
    mConverter->start();
    setControlsEnable();
}


/************************************************

 ************************************************/
void MainWindow::stopConvert()
{
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
        disk->downloadInfo();
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
        foreach(InputAudioFormat format, InputAudioFormat::allFormats())
        {
            allFlt << QString(" *.%1").arg(format.ext());
            flt << fltPattern.arg(format.name(), format.ext());
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

 ************************************************/
void MainWindow::openAddFileDialog()
{
    QString flt = getOpenFileFilter(true, true);
    QString lastDir = settings->value(Settings::Misc_LastDir).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add CUE or audio file", "OpenFile dialog title"), lastDir, flt);

    foreach(const QString &fileName, fileNames)
    {
        //fileName = self._fixFileNameBug(fileName)
        settings->setValue(Settings::Misc_LastDir, QFileInfo(fileName).dir().path());

        QApplication::setOverrideCursor(Qt::WaitCursor);
        addFileOrDir(fileName);
        QApplication::restoreOverrideCursor();
    }
}


/************************************************

 ************************************************/
void MainWindow::setAudioForDisk(Disk *disk)
{
    QString flt = getOpenFileFilter(true, false);

    QString dir;

    if (!disk->cueFile().isEmpty())
    {
        dir = QFileInfo(disk->cueFile()).dir().absolutePath();
    }
    else if (! disk->audioFileName().isEmpty())
    {
        dir = QFileInfo(disk->audioFileName()).dir().absolutePath();
    }
    else
    {
        dir = settings->value(Settings::Misc_LastDir).toString();
    }


    QString fileName = QFileDialog::getOpenFileName(this, tr("Select audio file", "OpenFile dialog title"), dir, flt);

    if (!fileName.isEmpty())
    {
        disk->setAudioFile(fileName);
    }
}


/************************************************

 ************************************************/
void MainWindow::addFileOrDir(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFileInfo fi = QFileInfo(fileName);
    if (fi.isDir())
    {
        mScanner = new DirScanner;
        setControlsEnable();
        mScanner->start(fi.absoluteFilePath());
        delete mScanner;
        mScanner = 0;
        setControlsEnable();
    }
    else if (fi.size() > 102400)
    {
        project->addAudioFile(fi.canonicalFilePath());
    }
    else
    {
        project->addCueFile(fi.canonicalFilePath());
    }
    QApplication::restoreOverrideCursor();
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
void MainWindow::showErrorMessage(const QString &message)
{
    QMessageBox::critical(this, tr("Flacon", "Error"), message);
}


/************************************************

 ************************************************/
void MainWindow::setStartTrackNum()
{
    QList<Disk*> disks = trackView->selectedDisks();
    foreach(Disk *disk, disks)
    {
        disk->setStartTrackNum(tagStartNumEdit->value());
    }
}


/************************************************

 ************************************************/
void MainWindow::initActions()
{
    actionAddFile->setIcon(Project::getIcon("document-open", "fileopen", ":/icons/22/add-file"));
    connect(actionAddFile, SIGNAL(triggered()), this, SLOT(openAddFileDialog()));

    actionRemoveDisc->setIcon(Project::getIcon("edit-delete", "remove", ":/icons/22/remove-disk"));
    connect(actionRemoveDisc, SIGNAL(triggered()), this, SLOT(removeDisks()));

    actionScan->setIcon(Project::getIcon("document-open-folder", "document-open", "folder_open", ":/icons/22/scan"));
    connect(actionScan, SIGNAL(triggered()), this, SLOT(openScanDialog()));

    actionDownloadTrackInfo->setIcon(Project::getIcon("download", "web-browser", "network", ":/icons/22/download-track-info"));
    connect(actionDownloadTrackInfo, SIGNAL(triggered()), this, SLOT(downloadInfo()));


    actionStartConvert->setIcon(Project::getIcon("dialog-ok", "button_ok", ":/icons/22/start-convert"));
    connect(actionStartConvert, SIGNAL(triggered()), this, SLOT(startConvert()));

    actionAbortConvert->setIcon(Project::getIcon("dialog-cancel", "button_cancel", ":/icons/22/abort-convert"));
    connect(actionAbortConvert, SIGNAL(triggered()), this, SLOT(stopConvert()));

    actionSelectResultDir->setIcon(Project::getIcon("document-open-folder", "document-open", "folder_open", ":/icons/22/select-result-dir"));
    connect(actionSelectResultDir, SIGNAL(triggered()), this, SLOT(openOutDirDialog()));

    actionConfigure->setIcon(Project::getIcon("configure", "preferences-system", ":/icons/22/configure"));
    connect(actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));

    actionConfigureEncoder->setIcon(actionConfigure->icon());
    connect(actionConfigureEncoder, SIGNAL(triggered()), this, SLOT(configureEncoder()));

    actionAbout->setIcon(Project::getIcon("help-about", "info", ":/icons/22/about"));
    connect(actionAbout, SIGNAL(triggered()), this,  SLOT(openAboutDialog()));
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



