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
#include "inputaudiofile.h"
#include "formats_in/informat.h"
#include "preferences/preferencesdialog.h"
#include "aboutdialog/aboutdialog.h"
#include "scanner.h"
#include "gui/coverdialog/coverdialog.h"
#include "internet/dataprovider.h"
#include "gui/trackviewmodel.h"
#include "gui/tageditor/tageditor.h"
#include "controls.h"
#include "gui/icon.h"
#include "application.h"
#include "gui/messagebox/messagebox.h"
#include "gui/logview/logview.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QQueue>
#include <QKeyEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QToolBar>
#include <QToolButton>
#include <QStandardPaths>
#include "qtbackports/movetotrash.h"
#include "audiofilematcher.h"

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

static constexpr auto SETTINGS_LASTDIR_KEY              = "Misc/LastDirectory";
static constexpr auto SETTINGS_PATTERN_HISTORY_KEY      = "OutFiles/PatternHistory";
static constexpr auto SETTINGS_OUTFILES_DIR_HISTORY_KEY = "OutFiles/DirectoryHistory";

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

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
    connect(tagGenreEdit, &TagLineEdit::textEdited, this, &MainWindow::setGenreTag);

    tagYearEdit->setTagId(TagId::Date);
    connect(tagYearEdit, &TagLineEdit::textEdited, this, &MainWindow::setTrackTag);

    tagArtistEdit->setTagId(TagId::Artist);
    connect(tagArtistEdit, &TagLineEdit::textEdited, this, &MainWindow::setArtistTag);
    // connect(tagArtistEdit, &TagLineEdit::textEdited, this, &MainWindow::refreshEdits);

    tagDiscPerformerEdit->setTagId(TagId::AlbumArtist);
    // connect(tagDiscPerformerEdit, &TagLineEdit::textEdited, this, &MainWindow::setDiscTag);
    connect(tagDiscPerformerEdit, &TagLineEdit::textEdited, this, &MainWindow::refreshEdits);

    tagAlbumEdit->setTagId(TagId::Album);
    connect(tagAlbumEdit, &TagLineEdit::textEdited, this, &MainWindow::setAlbumTag);

    tagDiscIdEdit->setTagId(TagId::DiscId);
    connect(tagDiscIdEdit, &TagLineEdit::textEdited, this, &MainWindow::setTrackTag);
    connect(tagDiscIdEdit, &TagLineEdit::textEdited, this, &MainWindow::setControlsEnable);

    connect(tagStartNumEdit, &MultiValuesSpinBox::editingFinished, this, &MainWindow::setStartTrackNum);
    connect(tagStartNumEdit, qOverload<int>(&MultiValuesSpinBox::valueChanged),
            this, &MainWindow::setStartTrackNum);

    connect(trackView, &TrackView::customContextMenuRequested, this, &MainWindow::trackViewMenu);

    connect(editTagsButton, &QPushButton::clicked, this, &MainWindow::openEditTagsDialog);

    connect(&Project::instance()->validator(), &Validator::changed, this, &MainWindow::setControlsEnable);
    connect(&Project::instance()->validator(), &Validator::changed, this, &MainWindow::refreshEdits);
    connect(&Project::instance()->validator(), &Validator::changed, trackView, &TrackView::layoutChanged);

    initActions();
    initToolBar();
    initStatusBar();

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

    // Signals .................................................
    connect(outDirEdit->lineEdit(), &QLineEdit::textEdited, this, &MainWindow::setOutDir);
    connect(outPatternEdit->lineEdit(), &QLineEdit::textEdited, this, &MainWindow::setPattern);

    connect(outProfileCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::setOutProfile);

    connect(codepageCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::setCodePage);

    connect(trackView, &TrackView::selectCueFile, this, &MainWindow::setCueForDisc);
    connect(trackView, &TrackView::selectAudioFile, this, &MainWindow::setAudioForDisc);
    connect(trackView, &TrackView::showAudioMenu, this, &MainWindow::showDiskAudioFileMenu);
    connect(trackView, &TrackView::selectCoverImage, this, &MainWindow::setCoverImage);
    connect(trackView, &TrackView::downloadInfo, this, &MainWindow::downloadDiscInfo);

    connect(trackView->model(), &TrackViewModel::layoutChanged, this, &MainWindow::refreshEdits);
    connect(trackView->model(), &TrackViewModel::layoutChanged, this, &MainWindow::setControlsEnable);

    connect(trackView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::refreshEdits);
    connect(trackView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::setControlsEnable);

    connect(Project::instance(), &Project::layoutChanged, trackView, &TrackView::layoutChanged);
    connect(Project::instance(), &Project::layoutChanged, this, &MainWindow::refreshEdits);
    connect(Project::instance(), &Project::layoutChanged, this, &MainWindow::setControlsEnable);

    connect(Project::instance(), &Project::discChanged, this, &MainWindow::refreshEdits);
    connect(Project::instance(), &Project::discChanged, this, &MainWindow::setControlsEnable);

    connect(Application::instance(), &Application::visualModeChanged,
            []() {
                Icon::setDarkMode(Application::instance()->isDarkVisualMode());
            });

    Icon::setDarkMode(Application::instance()->isDarkVisualMode());

    loadSettings();

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
    if (Project::instance()->count())
        trackView->selectDisc(Project::instance()->disc(0));
}

/************************************************

 ************************************************/
MainWindow::~MainWindow()
{
}

/************************************************

 ************************************************/
bool MainWindow::showExitDialog()
{
    QMessageBox dialog(this);
    dialog.setText(tr("Conversion in progress.<br>Are you sure you want to exit?", "Message box text"));
    dialog.setTextFormat(Qt::RichText);
    dialog.setIconPixmap(QPixmap(":/64/mainicon"));

    dialog.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    dialog.button(QMessageBox::Yes)->setText(tr("Exit", "Button caption"));
    dialog.setDefaultButton(QMessageBox::No);

    dialog.setWindowModality(Qt::WindowModal);

    return dialog.exec() == QMessageBox::Yes;
}

/************************************************

 ************************************************/
void MainWindow::setStartButtonAction(QAction *action)
{
    QToolButton *runBtn = qobject_cast<QToolButton *>(toolBar->widgetForAction(actionStartConvert));
    if (runBtn) {
        runBtn->setDefaultAction(action);
    }
}

/************************************************

 ************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mConverter && mConverter->isRunning()) {
        if (!showExitDialog()) {
            event->ignore();
            return;
        }
    }

    if (mConverter) {
        mConverter->stop();
    }

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
    Project::instance()->profile()->setOutFilePattern(outPatternEdit->currentText());
    emit trackView->model()->layoutChanged();
    Project::instance()->validator().revalidate();
}

/************************************************

 ************************************************/
void MainWindow::setOutDir()
{
    Project::instance()->profile()->setOutFileDir(outDirEdit->currentText());
    emit trackView->model()->layoutChanged();
    Project::instance()->validator().revalidate();
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
            dir = Settings::i()->value(SETTINGS_LASTDIR_KEY).toString();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select CUE file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    try {
        Cue     cue(fileName);
        QString oldDir = QFileInfo(disc->cueFilePath()).dir().path();
        disc->setCue(cue);
        QString newDir = QFileInfo(disc->cueFilePath()).dir().path();

        bool hasAuioFiles = false;
        for (auto f : disc->audioFiles()) {
            hasAuioFiles = hasAuioFiles || !f.isNull();
        }

        if (!hasAuioFiles) {
            AudioFileMatcher matcher;
            matcher.matchForCue(cue);
            disc->setCue(matcher.cue());
            disc->setAudioFiles(matcher.audioFiles());
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
        Project::instance()->selectProfile(outProfileCombo->itemData(n).toString());
    }
    emit trackView->model()->layoutChanged();
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

    bool canDownload = false;
    bool canConvert  = false;
    bool haveOK      = false;
    foreach (const Disc *disc, Project::instance()->disks()) {
        canDownload = canDownload || DataProvider::canDownload(*disc);
        canConvert  = canConvert || !Project::instance()->validator().diskHasErrors(disc);
        haveOK      = haveOK || disc->state() == DiskState::OK;
    }

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

    mTotalProgressLabel.setVisible(convert);

    actionRemoveSourceFiles->setEnabled(haveOK);
}

/************************************************

 ************************************************/
void MainWindow::refreshEdits()
{

    // Discs ...............................
    QSet<int>     startNums;
    QSet<QString> discId;
    QSet<QString> codePage;

    bool hasErrors   = false;
    bool hasWarnings = false;
    for (const Disk *d : Project::instance()->disks()) {
        hasErrors   = hasErrors || Project::instance()->validator().diskHasErrors(d);
        hasWarnings = hasWarnings || Project::instance()->validator().diskHasWarnings(d);
    }

    QList<Disc *> discs = trackView->selectedDiscs();
    foreach (Disc *disc, discs) {
        startNums << disc->startTrackNum();
        discId << disc->discIdTag();
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
        genre << track->genreTag();
        artist << track->artistTag();
        album << track->albumTag();
        date << track->dateTag();
        discPerformer << track->disc()->performerTag();
    }

    tagGenreEdit->setMultiValue(genre);
    tagYearEdit->setMultiValue(date);
    tagArtistEdit->setMultiValue(artist);
    tagAlbumEdit->setMultiValue(album);
    tagStartNumEdit->setMultiValue(startNums);
    tagDiscIdEdit->setMultiValue(discId);
    codepageCombo->setMultiValue(codePage);
    tagDiscPerformerEdit->setMultiValue(discPerformer);

    const Profile *profile = Project::instance()->profile();
    if (outDirEdit->currentText() != profile->outFileDir()) {
        outDirEdit->lineEdit()->setText(profile->outFileDir());
    }

    if (outPatternEdit->currentText() != profile->outFilePattern())
        outPatternEdit->lineEdit()->setText(profile->outFilePattern());

    refreshOutProfileCombo();

    actionWarnings->setVisible(hasWarnings);
    actionErrors->setVisible(hasErrors);
}

/************************************************

 ************************************************/
void MainWindow::refreshOutProfileCombo()
{
    outProfileCombo->blockSignals(true);
    int n = 0;
    for (const Profile &p : Project::instance()->profiles()) {
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

    n = outProfileCombo->findData(Project::instance()->profile()->id());

    if (n > -1) {
        outProfileCombo->setCurrentIndex(n);
    }
    else {
        outProfileCombo->setCurrentIndex(0);
        setOutProfile();
    }
}

/************************************************

 ************************************************/
void MainWindow::preferencesDialogDone()
{
    PreferencesDialog *dlg = qobject_cast<PreferencesDialog *>(sender());
    if (!dlg) {
        return;
    }

    Project::instance()->setProfiles(dlg->profiles());
    Project::instance()->save(Settings::i());

    refreshEdits();
    emit trackView->model()->layoutChanged();
}

/************************************************

 ************************************************/
void MainWindow::setCodePage()
{
    QString codepage = codepageCombo->codePage();
    if (!codepage.isEmpty()) {
        QList<Disc *> discs = trackView->selectedDiscs();
        foreach (Disc *disc, discs)
            disc->setCodecName(codepage);
    }
}

/************************************************

 ************************************************/
void MainWindow::setTrackTag()
{
#warning REMOVE ME
    //     TagLineEdit *edit = qobject_cast<TagLineEdit *>(sender());
    //     if (!edit)
    //         return;

    //     QList<Disc *> disks = trackView->selectedDiscs();
    //     for (Disk *d : disks) {
    //         d->blockSignals(true);
    //     }
    //     QList<Track *> tracks = trackView->selectedTracks();
    //     foreach (Track *track, tracks) {
    //         track->setTag(edit->tagId(), edit->text());
    //     }

    //     for (Disk *d : disks) {
    //         d->blockSignals(false);
    //         emit d->tagChanged();
    //     }

    //     trackView->updateAll();
}

/************************************************
 *
 ************************************************/
// void MainWindow::setDiscTag()
// {
//     TagLineEdit *edit = qobject_cast<TagLineEdit *>(sender());
//     if (!edit)
//         return;

//     QList<Disc *> discs = trackView->selectedDiscs();
//     foreach (Disc *disc, discs) {
//         disc->setDiscTag(edit->tagId(), edit->text());
//     }

//     trackView->updateAll();
// }

// /************************************************
//  *
//  ************************************************/
// void MainWindow::setDiscTagInt()
// {
//     TagSpinBox *spinBox = qobject_cast<TagSpinBox *>(sender());
//     if (!spinBox)
//         return;

//     QList<Disc *> discs = trackView->selectedDiscs();
//     foreach (Disc *disc, discs) {
//         disc->setDiscTag(spinBox->tagId(), QString::number(spinBox->value()));
//     }

//     trackView->updateAll();
// }

/************************************************
 *
 ************************************************/
void MainWindow::startConvertAll()
{
    Conv::Converter::Jobs jobs;
    for (int d = 0; d < Project::instance()->count(); ++d) {
        Conv::Converter::Job job;
        job.disc = Project::instance()->disc(d);

        for (int t = 0; t < job.disc->tracks().count(); ++t) {
            job.tracks << job.disc->tracks().at(t);
        }

        jobs << job;
    }

    startConvert(jobs);
}

/************************************************
 *
 ************************************************/
void MainWindow::startConvertSelected()
{
    Conv::Converter::Jobs jobs;
    for (Disc *disc : trackView->selectedDiscs()) {
        Conv::Converter::Job job;
        job.disc = disc;

        for (const Track *track : disc->tracks()) {
            if (trackView->isSelected(*track)) {
                job.tracks << track;
            }
        }

        jobs << job;
    }

    startConvert(jobs);
}

/************************************************

 ************************************************/
void MainWindow::startConvert(const Conv::Converter::Jobs &jobs)
{
    if (!Project::instance()->profile()->isValid())
        return;

    trackView->setFocus();

    bool hasErrors = true;
    for (int i = 0; i < Project::instance()->count(); ++i) {
        hasErrors = hasErrors || Project::instance()->validator().diskHasErrors(Project::instance()->disc(i));
    }

    if (!hasErrors) {
        int res = QMessageBox::warning(this,
                                       windowTitle(),
                                       tr("Some albums will not be converted, they contain errors.\nDo you want to continue?"),
                                       QMessageBox::Ok | QMessageBox::Cancel);
        if (res != QMessageBox::Ok)
            return;
    }

    for (int d = 0; d < Project::instance()->count(); ++d) {
        Disc *disc = Project::instance()->disc(d);
        for (int t = 0; t < disc->tracks().count(); ++t)
            trackView->model()->trackProgressChanged(*disc->tracks().at(t), TrackState::NotRunning, 0);
    }

    trackView->setColumnWidth(TrackView::ColumnPercent, 200);
    mConverter = new Conv::Converter();
    connect(mConverter, &Conv::Converter::finished,
            this, &MainWindow::setControlsEnable);

    connect(mConverter, &Conv::Converter::finished,
            mConverter, &Conv::Converter::deleteLater);

    connect(mConverter, &Conv::Converter::trackProgress,
            trackView->model(), &TrackViewModel::trackProgressChanged);

    connect(mConverter, &Conv::Converter::totalProgress,
            this, &MainWindow::updateTotalProgress);

    updateTotalProgress(0);

    connect(mConverter, &Conv::Converter::error,
            this, &MainWindow::showErrorMessage);

    setWindowTitle(tr("Flacon - Converting", "Main window title"));
    connect(mConverter, &Conv::Converter::finished, this, [this]() {
        setWindowTitle(tr("Flacon"));
    });

    mConverter->start(jobs, *(Project::instance()->profile()));
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
    auto dlg = PreferencesDialog::createAndShow(Project::instance()->profiles(), this);
    connect(dlg, &PreferencesDialog::accepted, this, &MainWindow::preferencesDialogDone, Qt::UniqueConnection);
}

/************************************************

 ************************************************/
void MainWindow::configureEncoder()
{
    auto dlg = PreferencesDialog::createAndShow(Project::instance()->profiles(), Project::instance()->profile()->id(), this);
    connect(dlg, &PreferencesDialog::accepted, this, &MainWindow::preferencesDialogDone, Qt::UniqueConnection);
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
    QString     lastDir   = Settings::i()->value(SETTINGS_LASTDIR_KEY).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add CUE or audio file", "OpenFile dialog title"), lastDir, flt);

    if (fileNames.isEmpty()) {
        return;
    }

    Settings::i()->setValue(SETTINGS_LASTDIR_KEY, QFileInfo(fileNames.last()).dir().path());

    foreach (const QString &fileName, fileNames) {
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
            dir = Settings::i()->value(SETTINGS_LASTDIR_KEY).toString();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select audio file", "OpenFile dialog title"), dir, flt);

    if (fileName.isEmpty())
        return;

    InputAudioFile audio(fileName);
    if (!audio.isValid()) {
        Messages::error(tr("\"%1\" was not set.", "Error message, %1 is an filename.")
                                .arg(QFileInfo(fileName).fileName())
                        + "<br>" + audio.errorString());
        return;
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
    if (!DataProvider::canDownload(*disc)) {
        return;
    }

    DataProvider *provider = new DataProvider();
    connect(provider, &DataProvider::finished,
            provider, &DataProvider::deleteLater);

    connect(provider, &DataProvider::finished, provider,
            [disc, this]() { trackView->downloadFinished(*disc); });

    connect(provider, &DataProvider::finished, provider,
            [disc](const QVector<InternetTags> data) { disc->addInternetTags(data); });

    provider->start(*disc);
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
                discs << Project::instance()->addAudioFile(file);
            else
                discs << Project::instance()->addCueFile(file);

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

    int n = Project::instance()->indexOf(discs.first());
    Project::instance()->removeDisc(discs);

    n = qMin(n, Project::instance()->count() - 1);
    if (n > -1)
        trackView->selectDisc(Project::instance()->disc(n));

    setControlsEnable();
}

/************************************************

 ************************************************/
void MainWindow::openScanDialog()
{
    QString lastDir = Settings::i()->value(SETTINGS_LASTDIR_KEY).toString();
    QString dir     = QFileDialog::getExistingDirectory(this, tr("Select directory"), lastDir);

    if (!dir.isEmpty()) {
        Settings::i()->setValue(SETTINGS_LASTDIR_KEY, dir);
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
void MainWindow::openLogViewDialog()
{
    LogView *dialog = findChild<LogView *>();

    if (!dialog) {
        dialog = new LogView(this);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
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
void MainWindow::fillAudioMenu(Disc *disc, QMenu &menu)
{
    QAction *act;
    if (disc->audioFiles().count() == 1) {
        act = new QAction(tr("Select another audio file…", "context menu"), &menu);
        connect(act, &QAction::triggered, this, [this, disc]() { this->setAudioForDisc(disc, 0); });
        menu.addAction(act);
    }
    else {
        int n = 0;
        for (TrackPtrList &l : disc->tracksByFileTag()) {
            QString msg;
            if (l.count() == 1) {
                msg = tr("Select another audio file for %1 track…", "context menu. Placeholders are track number")
                              .arg(l.first()->trackNumTag());
            }
            else {
                msg = tr("Select another audio file for tracks %1 to %2…", "context menu. Placeholders are track numbers")
                              .arg(l.first()->trackNumTag())
                              .arg(l.last()->trackNumTag());
            }

            act = new QAction(msg, &menu);
            connect(act, &QAction::triggered, this, [this, disc, n]() { this->setAudioForDisc(disc, n); });
            menu.addAction(act);

            n++;
        }
    }
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
    fillAudioMenu(disc, menu);

    act = new QAction(tr("Select another CUE file…", "context menu"), &menu);
    connect(act, &QAction::triggered, this, [this, disc]() { this->setCueForDisc(disc); });
    menu.addAction(act);

    act = new QAction(tr("Get data from Internet", "context menu"), &menu);
    act->setEnabled(DataProvider::canDownload(*disc));
    connect(act, &QAction::triggered, this, [this, disc]() { this->downloadDiscInfo(disc); });
    menu.addAction(act);

    menu.exec(trackView->viewport()->mapToGlobal(pos));
}

/************************************************
 *
 ************************************************/
void MainWindow::showDiskAudioFileMenu(Disc *disc, const QPoint &pos)
{
    QMenu menu;
    fillAudioMenu(disc, menu);
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
    setControlsEnable();
}

void MainWindow::removeSourceFiles()
{
    QStringList   files;
    QList<Disk *> disks;
    for (Disk *d : Project::instance()->disks()) {
        if (d->state() == DiskState::OK) {
            disks << d;
            files << d->cueFilePath();
            files << d->audioFilePaths();
        }
    }

    if (files.isEmpty()) {
        return;
    }

    QuestionBox dialog(this);
    dialog.setText("<b>" + tr("The following files will be moved to the trash. Remove the following files?", "Message box text") + "</b>");
    dialog.setDescription("<ul><li>" + files.join("</li><li>") + "</li></ul>");
    if (dialog.exec() != QMessageBox::Yes) {
        return;
    }

    Project::instance()->removeDisc(disks);
    for (const QString &f : std::as_const(files)) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
        moveFileToTrash(f);
#else
        QFile::moveToTrash(f);
#endif
    }
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

void MainWindow::setGenreTag()
{
    //     TagLineEdit *edit = qobject_cast<TagLineEdit *>(sender());
    //     if (!edit)
    //         return;

    QList<Disc *> disks = trackView->selectedDiscs();
    for (Disk *d : disks) {
        d->blockSignals(true);
        d->setGenreTag(tagGenreEdit->text());
    }

    for (Disk *d : disks) {
        d->blockSignals(false);
        emit d->tagChanged();
    }

    trackView->updateAll();
}

void MainWindow::setArtistTag()
{
    QList<Disc *> disks = trackView->selectedDiscs();
    for (Disk *d : disks) {
        d->blockSignals(true);
        d->setPerformerTag(tagArtistEdit->text());
    }

    for (Disk *d : disks) {
        d->blockSignals(false);
        emit d->tagChanged();
    }

    trackView->updateAll();
    refreshEdits();
}

void MainWindow::setAlbumTag()
{
    QList<Disc *> disks = trackView->selectedDiscs();
    for (Disk *d : disks) {
        d->blockSignals(true);
        d->setAlbumTag(tagAlbumEdit->text());
    }

    for (Disk *d : disks) {
        d->blockSignals(false);
        emit d->tagChanged();
    }

    trackView->updateAll();
    refreshEdits();
}

/************************************************

 ************************************************/
void MainWindow::initActions()
{
    actionAddDisc->setIcon(Icon("add-disk"));
    connect(actionAddDisc, &QAction::triggered, this, &MainWindow::openAddFileDialog);

    actionRemoveDisc->setIcon(Icon("remove-disk"));
    connect(actionRemoveDisc, &QAction::triggered, this, &MainWindow::removeDiscs);

    actionAddFolder->setIcon(Icon("scan"));
    connect(actionAddFolder, &QAction::triggered, this, &MainWindow::openScanDialog);

    actionDownloadTrackInfo->setIcon(Icon("download-info"));
    connect(actionDownloadTrackInfo, &QAction::triggered, this, &MainWindow::downloadInfo);

    actionStartConvert->setIcon(Icon("start-convert"));
    connect(actionStartConvert, &QAction::triggered, this, &MainWindow::startConvertAll);

    actionStartConvertSelected->setIcon(Icon("start-convert"));
    connect(actionStartConvertSelected, &QAction::triggered, this, &MainWindow::startConvertSelected);

    actionAbortConvert->setIcon(Icon("abort-convert"));
    connect(actionAbortConvert, &QAction::triggered, this, &MainWindow::stopConvert);

    actionConfigure->setIcon(Icon("configure"));
    connect(actionConfigure, &QAction::triggered, this, &MainWindow::configure);
    actionConfigure->setMenuRole(QAction::PreferencesRole);

    actionConfigureEncoder->setIcon(actionConfigure->icon());
    connect(actionConfigureEncoder, &QAction::triggered, this, &MainWindow::configureEncoder);

    connect(actionAbout, &QAction::triggered, this, &MainWindow::openAboutDialog);
    actionAbout->setMenuRole(QAction::AboutRole);

    connect(actionShowLogs, &QAction::triggered, this, &MainWindow::openLogViewDialog);

    Controls::arangeTollBarButtonsWidth(toolBar);

    actionWarnings->setIcon(Icon("warning"));
    connect(actionWarnings, &QAction::triggered, this, &MainWindow::showWarnings);

    actionErrors->setIcon(Icon("error"));
    connect(actionErrors, &QAction::triggered, this, &MainWindow::showErrors);

    connect(actionRemoveSourceFiles, &QAction::triggered, this, &MainWindow::removeSourceFiles);

#ifdef MAC_UPDATER
    actionUpdates->setVisible(true);
    actionUpdates->setMenuRole(QAction::ApplicationSpecificRole);

    connect(actionUpdates, &QAction::triggered,
            this, &MainWindow::checkUpdates);
#else
    actionUpdates->setVisible(false);
#endif
}

/************************************************
 *
 ************************************************/
void MainWindow::initToolBar()
{
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(spacer);

    toolBar->addAction(actionWarnings);
    toolBar->addAction(actionErrors);

    QToolButton *runBtn = qobject_cast<QToolButton *>(toolBar->widgetForAction(actionStartConvert));
    if (runBtn) {
        runBtn->setPopupMode(QToolButton::MenuButtonPopup);
        runBtn->addAction(actionStartConvertSelected);
        connect(runBtn, &QToolButton::triggered, this, &MainWindow::setStartButtonAction);
    }
}

/************************************************
 *
 ************************************************/
void MainWindow::initStatusBar()
{
    auto margins = mTotalProgressLabel.contentsMargins();
    margins.setRight(10);
    mTotalProgressLabel.setContentsMargins(margins);

    statusBar()->addPermanentWidget(&mTotalProgressLabel);
}

/************************************************
  Load settings
 ************************************************/
void MainWindow::loadSettings()
{
    Settings *settings = Settings::i();

    // MainWindow geometry
    int x      = settings->value("MainWindow/Left", geometry().left()).toInt();
    int y      = settings->value("MainWindow/Top", geometry().top()).toInt();
    int width  = settings->value("MainWindow/Width", QVariant(987)).toInt();
    int height = settings->value("MainWindow/Height", QVariant(450)).toInt();
    this->setGeometry(x, y, width, height);

    splitter->restoreState(settings->value("MainWindow/Splitter").toByteArray());
    trackView->header()->restoreState(settings->value("MainWindow/TrackView").toByteArray());

    outDirEdit->setHistory(Settings::i()->value(SETTINGS_OUTFILES_DIR_HISTORY_KEY).toStringList());
    outPatternEdit->setHistory(Settings::i()->value(SETTINGS_PATTERN_HISTORY_KEY).toStringList());
}

/************************************************
  Write settings
 ************************************************/
void MainWindow::saveSettings()
{
    Settings *settings = Settings::i();

    settings->setValue("MainWindow/Left", geometry().left());
    settings->setValue("MainWindow/Top", geometry().top());
    settings->setValue("MainWindow/Width", QVariant(size().width()));
    settings->setValue("MainWindow/Height", QVariant(size().height()));
    settings->setValue("MainWindow/Splitter", QVariant(splitter->saveState()));
    settings->setValue("MainWindow/TrackView", QVariant(trackView->header()->saveState()));

    Settings::i()->setValue(SETTINGS_OUTFILES_DIR_HISTORY_KEY, outDirEdit->history());
    Settings::i()->setValue(SETTINGS_PATTERN_HISTORY_KEY, outPatternEdit->history());
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
    ErrorBox     *box  = this->findChild<ErrorBox *>(name);
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
} //-V773

/************************************************
 *
 ************************************************/
static QStringList diskMsgsToHtml(int diskNum, const Disk *disk, const QStringList &msgs)
{
    QStringList res;
    res << "<div>";
    if (disk->tracks().count()) {
        res << "<b>" + MainWindow::tr("Disk %1 \"%2 - %3\"", "Error message, %1, %2 and %3 is the number, artist and album for the disc, respectively").arg(diskNum).arg(disk->artistTag(), disk->albumTag()) + "</b>";
    }
    else {
        res << "<b>" + MainWindow::tr("Disk %1", "Error message, %1 is the disc number").arg(diskNum) + "</b>";
    }

    res << "<ul>";
    for (QString msg : msgs) {
        msg = msg.replace("\n", "<br>");
        res << QString("<li>%1</li>").arg(msg);
    }
    res << "</ul>";
    res << "</div>";

    return res;
}

/************************************************
 *
 ************************************************/
void MainWindow::showWarnings()
{
    const Validator &validator = Project::instance()->validator();

    QString text = tr("Some disks have warnings:", "Error message title");

    QStringList html;

    int n = 0;
    for (const Disk *d : Project::instance()->disks()) {
        n++;
        QStringList warns = validator.diskWarnings(d);
        if (!warns.isEmpty()) {
            html << diskMsgsToHtml(n, d, warns);
        }
    }

    WarningBox dialog = WarningBox(this);
    dialog.setText(text);
    dialog.setDescription(html.join(""));
    dialog.exec();
}

/************************************************
 *
 ************************************************/
void MainWindow::showErrors()
{
    const Validator &validator = Project::instance()->validator();

    QString text = tr("Some disks have errors, and will be skipped when converting:", "Error message title");

    QStringList html;

    int n = 0;
    for (const Disk *d : Project::instance()->disks()) {
        n++;
        QStringList errs = validator.diskErrors(d);
        if (!errs.isEmpty()) {
            html << diskMsgsToHtml(n, d, errs);
        }
    }

    CriticalBox dialog = CriticalBox(this);
    dialog.setText(text);
    dialog.setDescription(html.join(""));
    dialog.exec();
}

/************************************************
 *
 ************************************************/
void MainWindow::updateTotalProgress(double percent)
{
    mTotalProgressLabel.setText(tr("%1% completed", "Status bar, progress text").arg(percent, 0, 'f', 0));
}
