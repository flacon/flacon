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
#include "disc.h"
#include "settings.h"
#include "converter/converter.h"
#include "outformat.h"
#include "inputaudiofile.h"
#include "formats/informat.h"
#include "configdialog/configdialog.h"
#include "aboutdialog/aboutdialog.h"
#include "scanner.h"
#include "gui/coverdialog/coverdialog.h"
#include "internet/dataprovider.h"
#include "gui/trackviewmodel.h"
#include "gui/tageditor/tageditor.h"
#include "controls.h"
#include "gui/icon.h"
#include "application.h"
#include "patternexpander.h"

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
    toolBar->setIconSize(QSize(24, 24));
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#ifdef Q_OS_MAC
    this->setUnifiedTitleAndToolBarOnMac(true);
    setWindowIcon(QIcon());

    trackView->setFrameShape(QFrame::NoFrame);
    splitter->setStyleSheet("::handle{ border-right: 1px solid #7F7F7F7F;}");
#endif

    setAcceptDrops(true);
    setAcceptDrops(true);
    this->setContextMenuPolicy(Qt::NoContextMenu);

    outPatternButton->setToolTip(outPatternEdit->toolTip());
    outDirButton->setToolTip(outDirEdit->toolTip());

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

    tagDiscPerformerEdit->setTagId(TagId::AlbumArtist);
    connect(tagDiscPerformerEdit, SIGNAL(textEdited(QString)), this, SLOT(setDiscTag()));
    connect(tagDiscPerformerEdit, SIGNAL(textEdited(QString)), this, SLOT(refreshEdits()));

    tagAlbumEdit->setTagId(TagId::Album);
    connect(tagAlbumEdit, SIGNAL(textEdited(QString)), this, SLOT(setTrackTag()));

    tagDiscIdEdit->setTagId(TagId::DiscId);
    connect(tagStartNumEdit, SIGNAL(editingFinished()), this, SLOT(setStartTrackNum()));
    connect(tagStartNumEdit, SIGNAL(valueChanged(int)), this, SLOT(setStartTrackNum()));

    connect(trackView->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(refreshEdits()));
    connect(trackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(trackViewMenu(QPoint)));

    connect(editTagsButton, &QPushButton::clicked, this, &MainWindow::openEditTagsDialog);

    initActions();

    // Buttons .................................................
    outDirButton->setBuddy(outDirEdit);
    outDirButton->menu()->addSeparator();
    QAction *act = outDirEdit->deleteItemAction();
    act->setText(tr("Remove current directory from history"));
    outDirButton->menu()->addAction(act);

    configureProfileBtn->setBuddy(outProfileCombo);
    configureProfileBtn->setDefaultAction(actionConfigureEncoder);

    outPatternButton->setBuddy(outPatternEdit);
    outPatternButton->addStandardPatterns();
    outPatternButton->menu()->addSeparator();

    outPatternEdit->deleteItemAction()->setText(tr("Delete current pattern from history"));
    outPatternButton->menu()->addAction(outPatternEdit->deleteItemAction());

    loadSettings();

    // Signals .................................................
    connect(Settings::i(), SIGNAL(changed()), trackView->model(), SIGNAL(layoutChanged()));

    connect(outDirEdit->lineEdit(), &QLineEdit::textChanged, this, &MainWindow::setOutDir);
    connect(outPatternEdit->lineEdit(), &QLineEdit::textChanged, this, &MainWindow::setPattern);

    connect(outProfileCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setOutProfile()));

    connect(codepageCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCodePage()));

    connect(trackView, SIGNAL(selectCueFile(Disc *)), this, SLOT(setCueForDisc(Disc *)));
    connect(trackView, &TrackView::selectAudioFile, this, &MainWindow::setAudioForDisc);
    connect(trackView, SIGNAL(selectCoverImage(Disc *)), this, SLOT(setCoverImage(Disc *)));
    connect(trackView, SIGNAL(downloadInfo(Disc *)), this, SLOT(downloadDiscInfo(Disc *)));

    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(trackView->model(), SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(refreshEdits()));
    connect(trackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(layoutChanged()), trackView, SLOT(layoutChanged()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(layoutChanged()), this, SLOT(setControlsEnable()));

    connect(project, SIGNAL(discChanged(Disc *)), this, SLOT(refreshEdits()));
    connect(project, SIGNAL(discChanged(Disc *)), this, SLOT(setControlsEnable()));

    connect(Application::instance(), &Application::visualModeChanged,
            []() {
                Icon::setDarkMode(Application::instance()->isDarkVisualMode());
            });

    Icon::setDarkMode(Application::instance()->isDarkVisualMode());

    refreshEdits();
    setControlsEnable();
    polishView();
}

/************************************************
 *
 ************************************************/
#ifdef Q_OS_MAC
void MainWindow::polishView()
{
    QList<QLabel *> labels;
    labels << outFilesBox->findChildren<QLabel *>();
    labels << tagsBox->findChildren<QLabel *>();

    for (QLabel *label : labels) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
}
#else
void MainWindow::polishView()
{
}
#endif

/************************************************
 *
 ************************************************/
void MainWindow::showEvent(QShowEvent *)
{
    if (project->count())
        trackView->selectDisc(project->disc(0));
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
    foreach (QUrl url, event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }
}

/************************************************

 ************************************************/
void MainWindow::dropEvent(QDropEvent *event)
{
    foreach (QUrl url, event->mimeData()->urls()) {
        addFileOrDir(url.toLocalFile());
    }
}

/************************************************

 ************************************************/
void MainWindow::setPattern()
{
    Settings::i()->currentProfile().setOutFilePattern(outPatternEdit->currentText());
    trackView->model()->layoutChanged();
}

/************************************************

 ************************************************/
void MainWindow::setOutDir()
{
    Settings::i()->currentProfile().setOutFileDir(outDirEdit->currentText());
    trackView->model()->layoutChanged();
}

/************************************************

 ************************************************/
void MainWindow::setCueForDisc(Disc *disc)
{
    QString flt = getOpenFileFilter(false, true);

    QString dir;
    {
        QStringList audioFiles = disc->audioFilePaths();
        audioFiles.removeAll("");

        if (!audioFiles.isEmpty()) {
            dir = QFileInfo(audioFiles.first()).dir().absolutePath();
        }
        else if (!disc->cueFilePath().isEmpty()) {
            dir = QFileInfo(disc->cueFilePath()).dir().absolutePath();
        }
        else {
            dir = Settings::i()->value(Settings::Misc_LastDir).toString();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select CUE file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    try {
        Cue     cue(fileName);
        QString oldDir = QFileInfo(disc->cueFilePath()).dir().path();
        disc->setCueFile(cue);
        QString newDir = QFileInfo(disc->cueFilePath()).dir().path();

        if (disc->isMultiAudio()) {
            disc->searchAudioFiles(false);
        }

        if (newDir != oldDir) {
            disc->searchCoverImage(false);
        }
    }
    catch (FlaconError &err) {
        Messages::error(err.what());
    }
}

/************************************************

 ************************************************/
void MainWindow::setOutProfile()
{
    int n = outProfileCombo->currentIndex();
    if (n > -1) {
        Settings::i()->selectProfile(outProfileCombo->itemData(n).toString());
    }
    trackView->model()->layoutChanged();
}

/************************************************

 ************************************************/
void MainWindow::setControlsEnable()
{
    bool convert = mConverter && mConverter->isRunning();
    bool scan    = mScanner;
    bool running = scan || convert;

    bool tracksSelected = !trackView->selectedTracks().isEmpty();
    bool discsSelected  = !trackView->selectedDiscs().isEmpty();
    bool canConvert     = Converter::canConvert();

    bool canDownload = false;
    foreach (const Disc *disc, trackView->selectedDiscs())
        canDownload = canDownload || !disc->discId().isEmpty();

    outFilesBox->setEnabled(!running);
    tagsBox->setEnabled(!running && tracksSelected);
    actionAddDisc->setEnabled(!running);
    actionRemoveDisc->setEnabled(!running && discsSelected);
    actionStartConvert->setEnabled(!running && canConvert);
    actionDownloadTrackInfo->setEnabled(!running && canDownload);
    actionAddFolder->setEnabled(!running);
    actionConfigure->setEnabled(!running);
    actionConfigureEncoder->setEnabled(!running);

    actionStartConvert->setEnabled(!scan && canConvert);
    actionAbortConvert->setEnabled(convert);

    actionStartConvert->setVisible(!convert);
    actionAbortConvert->setVisible(convert);
}

/************************************************

 ************************************************/
void MainWindow::refreshEdits()
{

    // Discs ...............................
    QSet<int>     startNums;
    QSet<QString> discId;
    QSet<QString> codePage;

    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        startNums << disc->startTrackNum();
        discId << disc->discId();
        codePage << disc->codecName();
    }

    // Tracks ..............................
    QSet<QString> genre;
    QSet<QString> artist;
    QSet<QString> album;
    QSet<QString> date;
    QSet<QString> discPerformer;

    QList<Track *> tracks = trackView->selectedTracks();
    foreach (Track *track, tracks) {
        genre << track->genre();
        artist << track->artist();
        album << track->album();
        date << track->date();
        discPerformer << track->tag(TagId::AlbumArtist);
    }

    tagGenreEdit->setMultiValue(genre);
    tagYearEdit->setMultiValue(date);
    tagArtistEdit->setMultiValue(artist);
    tagAlbumEdit->setMultiValue(album);
    tagStartNumEdit->setMultiValue(startNums);
    tagDiscIdEdit->setMultiValue(discId);
    codepageCombo->setMultiValue(codePage);
    tagDiscPerformerEdit->setMultiValue(discPerformer);

    const Profile &profile = Settings::i()->currentProfile();
    if (outDirEdit->currentText() != profile.outFileDir())
        outDirEdit->lineEdit()->setText(profile.outFileDir());

    if (outPatternEdit->currentText() != profile.outFilePattern())
        outPatternEdit->lineEdit()->setText(profile.outFilePattern());

    refreshOutProfileCombo();
}

/************************************************

 ************************************************/
void MainWindow::refreshOutProfileCombo()
{
    outProfileCombo->blockSignals(true);
    int n = 0;
    for (const Profile &p : Settings::i()->profiles()) {
        if (n < outProfileCombo->count()) {
            outProfileCombo->setItemText(n, p.name());
            outProfileCombo->setItemData(n, p.id());
        }
        else {
            outProfileCombo->addItem(p.name(), p.id());
        }
        n++;
    }
    while (outProfileCombo->count() > n)
        outProfileCombo->removeItem(outProfileCombo->count() - 1);

    outProfileCombo->blockSignals(false);

    n = outProfileCombo->findData(Settings::i()->currentProfile().id());
    outProfileCombo->setCurrentIndex(qMax(0, n));
}

/************************************************

 ************************************************/
void MainWindow::setCodePage()
{
    int n = codepageCombo->currentIndex();
    if (n > -1) {
        QString codepage = codepageCombo->itemData(n).toString();

        QList<Disc *> discs = trackView->selectedDiscs();
        foreach (Disc *disc, discs)
            disc->setCodecName(codepage);

        Settings::i()->setValue(Settings::Tags_DefaultCodepage, codepage);
    }
}

/************************************************

 ************************************************/
void MainWindow::setTrackTag()
{
    TagLineEdit *edit = qobject_cast<TagLineEdit *>(sender());
    if (!edit)
        return;

    QList<Track *> tracks = trackView->selectedTracks();
    foreach (Track *track, tracks) {
        track->setTag(edit->tagId(), edit->text());
        trackView->update(*track);
    }
}

/************************************************
 *
 ************************************************/
void MainWindow::setDiscTag()
{
    TagLineEdit *edit = qobject_cast<TagLineEdit *>(sender());
    if (!edit)
        return;

    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        disc->setDiscTag(edit->tagId(), edit->text());
        trackView->update(*disc);
    }
}

/************************************************
 *
 ************************************************/
void MainWindow::setDiscTagInt()
{
    TagSpinBox *spinBox = qobject_cast<TagSpinBox *>(sender());
    if (!spinBox)
        return;

    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        disc->setDiscTag(spinBox->tagId(), QString::number(spinBox->value()));
        trackView->update(*disc);
    }
}

/************************************************
 *
 ************************************************/
void MainWindow::startConvertAll()
{
    Converter::Jobs jobs;
    for (int d = 0; d < project->count(); ++d) {
        Converter::Job job;
        job.disc = project->disc(d);
        for (int t = 0; t < job.disc->count(); ++t)
            job.tracks << job.disc->track(t);
        jobs << job;
    }

    startConvert(jobs);
}

/************************************************
 *
 ************************************************/
void MainWindow::startConvertSelected()
{
    Converter::Jobs jobs;
    for (Disc *disc : trackView->selectedDiscs()) {
        Converter::Job job;
        job.disc = disc;

        for (int t = 0; t < disc->count(); ++t) {
            Track *track = disc->track(t);
            if (trackView->isSelected(*track))
                job.tracks << track;
        }

        jobs << job;
    }

    startConvert(jobs);
}

/************************************************

 ************************************************/
void MainWindow::startConvert(const Converter::Jobs &jobs)
{
    if (!Settings::i()->currentProfile().isValid())
        return;

    trackView->setFocus();

    bool ok = true;
    for (int i = 0; i < project->count(); ++i) {
        ok = ok && project->disc(i)->canConvert();
    }

    if (!ok) {
        int res = QMessageBox::warning(this,
                                       windowTitle(),
                                       tr("Some albums will not be converted, they contain errors.\nDo you want to continue?"),
                                       QMessageBox::Ok | QMessageBox::Cancel);
        if (res != QMessageBox::Ok)
            return;
    }

    for (int d = 0; d < project->count(); ++d) {
        Disc *disc = project->disc(d);
        for (int t = 0; t < disc->count(); ++t)
            trackView->model()->trackProgressChanged(*disc->track(t), TrackState::NotRunning, 0);
    }

    trackView->setColumnWidth(TrackView::ColumnPercent, 200);
    mConverter = new Converter();
    connect(mConverter, &Converter::finished,
            this, &MainWindow::setControlsEnable);

    connect(mConverter, &Converter::finished,
            mConverter, &Converter::deleteLater);

    connect(mConverter, &Converter::trackProgress,
            trackView->model(), &TrackViewModel::trackProgressChanged);

    mConverter->start(jobs, Settings::i()->currentProfile());
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
    ConfigDialog::createAndShow(nullptr, this);
}

/************************************************

 ************************************************/
void MainWindow::configureEncoder()
{
    ConfigDialog::createAndShow(Settings::i()->currentProfile().id(), this);
}

/************************************************

 ************************************************/
void MainWindow::downloadInfo()
{
    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        this->downloadDiscInfo(disc);
    }
}

/************************************************

 ************************************************/
QString MainWindow::getOpenFileFilter(bool includeAudio, bool includeCue)
{
    QStringList flt;
    QStringList allFlt;
    QString     fltPattern = tr("%1 files", "OpenFile dialog filter line, like \"WAV files\"") + " (*.%2)";

    if (includeAudio) {
        foreach (const InputFormat *format, InputFormat::allFormats()) {
            allFlt << QString(" *.%1").arg(format->ext());
            flt << fltPattern.arg(format->name(), format->ext());
        }
    }

    flt.sort();

    if (includeCue) {
        allFlt << QString("*.cue");
        flt.insert(0, fltPattern.arg("CUE", "cue"));
    }

    if (allFlt.count() > 1)
        flt.insert(0, tr("All supported formats", "OpenFile dialog filter line") + " (" + allFlt.join(" ") + ")");

    flt << tr("All files", "OpenFile dialog filter line like \"All files\"") + " (*)";

    return flt.join(";;");
}

/************************************************

 ************************************************/
void MainWindow::openAddFileDialog()
{
    QString     flt       = getOpenFileFilter(true, true);
    QString     lastDir   = Settings::i()->value(Settings::Misc_LastDir).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add CUE or audio file", "OpenFile dialog title"), lastDir, flt);

    foreach (const QString &fileName, fileNames) {
        Settings::i()->setValue(Settings::Misc_LastDir, QFileInfo(fileName).dir().path());
        addFileOrDir(fileName);
    }
}

/************************************************

 ************************************************/
void MainWindow::setAudioForDisc(Disc *disc, int audioFileNum)
{
    QString flt = getOpenFileFilter(true, false);

    QString dir;
    {
        QStringList audioFiles = disc->audioFilePaths();

        if (!disc->cueFilePath().isEmpty()) {
            dir = QFileInfo(disc->cueFilePath()).dir().absolutePath();
        }
        else if (audioFileNum < audioFiles.count() && !audioFiles[audioFileNum].isEmpty()) {
            dir = QFileInfo(audioFiles[audioFileNum]).dir().absolutePath();
        }
        else {
            dir = Settings::i()->value(Settings::Misc_LastDir).toString();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select audio file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    InputAudioFile audio(fileName);
    if (!audio.isValid()) {
        Messages::error(audio.errorString());
    }

    disc->setAudioFile(audio, audioFileNum);
    trackView->update(*disc);
}

/************************************************
 *
 ************************************************/
void MainWindow::setCoverImage(Disc *disc)
{
    CoverDialog::createAndShow(disc, this);
}

/************************************************
 *
 ************************************************/
void MainWindow::downloadDiscInfo(Disc *disc)
{
    if (!disc->canDownloadInfo())
        return;

    DataProvider *provider = new FreeDbProvider(*disc);
    connect(provider, SIGNAL(finished()), provider, SLOT(deleteLater()));
    connect(provider, &DataProvider::finished,
            [disc, this]() { trackView->downloadFinished(*disc); });

    connect(provider, &DataProvider::ready,
            [disc](const QVector<Tracks> data) { disc->addTagSets(data); });

    provider->start();
    trackView->downloadStarted(*disc);
}

/************************************************

 ************************************************/
void MainWindow::addFileOrDir(const QString &fileName)
{
    bool isFirst   = true;
    bool showError = false;
    auto addFile   = [&](const QString &file) {
        try {
            QFileInfo fi = QFileInfo(file);
            DiscList  discs;
            if (fi.size() > 102400)
                discs << project->addAudioFile(file);
            else
                discs << project->addCueFile(file);

            if (!discs.isEmpty() && isFirst) {
                isFirst = false;
                this->trackView->selectDisc(discs.first());
            }
        }

        catch (FlaconError &err) {
            if (showError)
                showErrorMessage(err.what());
        }
    };

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFileInfo fi = QFileInfo(fileName);

    if (fi.isDir()) {
        mScanner = new Scanner;
        setControlsEnable();
        showError = false;
        connect(mScanner, &Scanner::found, addFile);
        mScanner->start(fi.absoluteFilePath());
        delete mScanner;
        mScanner = nullptr;
        setControlsEnable();
    }
    else {
        showError = true;
        addFile(fileName);
    }
    QApplication::restoreOverrideCursor();
}

/************************************************

 ************************************************/
void MainWindow::removeDiscs()
{
    QList<Disc *> discs = trackView->selectedDiscs();
    if (discs.isEmpty())
        return;

    int n = project->indexOf(discs.first());
    project->removeDisc(&discs);

    n = qMin(n, project->count() - 1);
    if (n > -1)
        trackView->selectDisc(project->disc(n));

    setControlsEnable();
}

/************************************************

 ************************************************/
void MainWindow::openScanDialog()
{
    QString lastDir = Settings::i()->value(Settings::Misc_LastDir).toString();
    QString dir     = QFileDialog::getExistingDirectory(this, tr("Select directory"), lastDir);

    if (!dir.isEmpty()) {
        Settings::i()->setValue(Settings::Misc_LastDir, dir);
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
    Updater::sharedUpdater().checkForUpdates("io.github.flacon");
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

    Disc *disc = trackView->model()->discByIndex(index);
    if (!disc)
        return;

    QMenu    menu;
    QAction *act = new QAction(tr("Edit tags…", "context menu"), &menu);
    connect(act, &QAction::triggered, this, &MainWindow::openEditTagsDialog);
    menu.addAction(act);

    menu.addSeparator();

    if (disc->audioFiles().count() == 1) {
        act = new QAction(tr("Select another audio file…", "context menu"), &menu);
        connect(act, &QAction::triggered, [this, disc]() { this->setAudioForDisc(disc, 0); });
        menu.addAction(act);
    }
    else {
        int n = 0;
        for (TrackPtrList &l : disc->tracksByFileTag()) {
            QString msg;
            if (l.count() == 1) {
                msg = tr("Select another audio file for %1 track…", "context menu. Placeholders are track number")
                              .arg(l.first()->trackNum());
            }
            else {
                msg = tr("Select another audio file for tracks (%1 to %2)…", "context menu. Placeholders are track numbers")
                              .arg(l.first()->trackNum())
                              .arg(l.last()->trackNum());
            }

            act = new QAction(msg, &menu);
            connect(act, &QAction::triggered, [this, disc, n]() { this->setAudioForDisc(disc, n); });
            menu.addAction(act);

            n++;
        }
    }

    act = new QAction(tr("Select another CUE file…", "context menu"), &menu);
    connect(act, &QAction::triggered, [this, disc]() { this->setCueForDisc(disc); });
    menu.addAction(act);

    act = new QAction(tr("Get data from CDDB", "context menu"), &menu);
    act->setEnabled(disc->canDownloadInfo());
    connect(act, &QAction::triggered, [this, disc]() { this->downloadDiscInfo(disc); });
    menu.addAction(act);

    menu.exec(trackView->viewport()->mapToGlobal(pos));
}

/************************************************
 *
 ************************************************/
void MainWindow::openEditTagsDialog()
{
    TagEditor editor(trackView->selectedTracks(), trackView->selectedDiscs(), this);
    editor.exec();
    refreshEdits();
}

/************************************************

 ************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (mScanner)
            mScanner->stop();
    }
}

/************************************************

 ************************************************/
bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
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

    int           value = tagStartNumEdit->value();
    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        disc->setStartTrackNum(value);
    }
}

/************************************************

 ************************************************/
void MainWindow::initActions()
{
    actionAddDisc->setIcon(Icon("add-disk"));
    connect(actionAddDisc, SIGNAL(triggered()), this, SLOT(openAddFileDialog()));

    actionRemoveDisc->setIcon(Icon("remove-disk"));
    connect(actionRemoveDisc, SIGNAL(triggered()), this, SLOT(removeDiscs()));

    actionAddFolder->setIcon(Icon("scan"));
    connect(actionAddFolder, SIGNAL(triggered()), this, SLOT(openScanDialog()));

    actionDownloadTrackInfo->setIcon(Icon("download-info"));
    connect(actionDownloadTrackInfo, SIGNAL(triggered()), this, SLOT(downloadInfo()));

    actionStartConvert->setIcon(Icon("start-convert"));
    connect(actionStartConvert, SIGNAL(triggered()), this, SLOT(startConvertAll()));

    actionStartConvertSelected->setIcon(Icon("start-convert"));
    connect(actionStartConvertSelected, SIGNAL(triggered()), this, SLOT(startConvertSelected()));

    actionAbortConvert->setIcon(Icon("abort-convert"));
    connect(actionAbortConvert, SIGNAL(triggered()), this, SLOT(stopConvert()));

    actionConfigure->setIcon(Icon("configure"));
    connect(actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));
    actionConfigure->setMenuRole(QAction::PreferencesRole);

    actionConfigureEncoder->setIcon(actionConfigure->icon());
    connect(actionConfigureEncoder, SIGNAL(triggered()), this, SLOT(configureEncoder()));

    connect(actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    actionAbout->setMenuRole(QAction::AboutRole);

    int w = 0;
    foreach (QAction *act, toolBar->actions()) {
        QToolButton *btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(act));
        if (btn)
            w = qMax(w, btn->sizeHint().width());
    }

    foreach (QAction *act, toolBar->actions()) {
        QToolButton *btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(act));
        if (btn)
            btn->setMinimumWidth(w);
    }

#ifdef MAC_UPDATER
    actionUpdates->setVisible(true);
    actionUpdates->setMenuRole(QAction::ApplicationSpecificRole);

    connect(actionUpdates, SIGNAL(triggered()),
            this, SLOT(checkUpdates()));
#else
    actionUpdates->setVisible(false);
#endif
}

/************************************************
  Load settings
 ************************************************/
void MainWindow::loadSettings()
{
    // MainWindow geometry
    int width  = Settings::i()->value(Settings::MainWindow_Width, QVariant(987)).toInt();
    int height = Settings::i()->value(Settings::MainWindow_Height, QVariant(450)).toInt();
    this->resize(width, height);

    splitter->restoreState(Settings::i()->value("MainWindow/Splitter").toByteArray());
    trackView->header()->restoreState(Settings::i()->value("MainWindow/TrackView").toByteArray());

    outDirEdit->setHistory(Settings::i()->value(Settings::OutFiles_DirectoryHistory).toStringList());
    outPatternEdit->setHistory(Settings::i()->value(Settings::OutFiles_PatternHistory).toStringList());
}

/************************************************
  Write settings
 ************************************************/
void MainWindow::saveSettings()
{
    Settings::i()->setValue("MainWindow/Width", QVariant(size().width()));
    Settings::i()->setValue("MainWindow/Height", QVariant(size().height()));
    Settings::i()->setValue("MainWindow/Splitter", QVariant(splitter->saveState()));
    Settings::i()->setValue("MainWindow/TrackView", QVariant(trackView->header()->saveState()));

    Settings::i()->setValue(Settings::OutFiles_DirectoryHistory, outDirEdit->history());
    Settings::i()->setValue(Settings::OutFiles_PatternHistory, outPatternEdit->history());
}

/************************************************
 *
 ************************************************/
QIcon MainWindow::loadMainIcon()
{
    if (QIcon::themeName() == "hicolor") {
        QStringList failback;
        failback << "oxygen";
        failback << "Tango";
        failback << "Prudence-icon";
        failback << "Humanity";
        failback << "elementary";
        failback << "gnome";

        QDir usrDir("/usr/share/icons/");
        QDir usrLocalDir("/usr/local/share/icons/");
        foreach (QString s, failback) {
            if (usrDir.exists(s) || usrLocalDir.exists(s)) {
                QIcon::setThemeName(s);
                break;
            }
        }
    }

    return QIcon::fromTheme("flacon", Icon("mainicon"));
}

/************************************************
 *
 ************************************************/
void MainWindow::showErrorMessage(const QString &message)
{
    const QString name = "errorMessage";
    ErrorBox *    box  = this->findChild<ErrorBox *>(name);
    if (!box) {
        box = new ErrorBox(this);
        box->setObjectName(name);
        box->setWindowTitle(QObject::tr("Flacon", "Error"));
        box->setAttribute(Qt::WA_DeleteOnClose, true);
    }

    QString msg = message;
    msg.replace('\n', "<br>\n");
    box->addMessage(msg);
    box->open();
}
