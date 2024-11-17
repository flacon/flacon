/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2018
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

#include "tageditor.h"
#include "ui_tageditor.h"

#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include "settings.h"

#include "gui/controls.h"

#include "track.h"
#include "disc.h"

static constexpr auto TAG_EDIT_DIALOG_WIDTH_KEY  = "TagEditor/Width";
static constexpr auto TAG_EDIT_DIALOG_HEIGHT_KEY = "TagEditor/Height";

static QList<Disk *> disksfromTracks(QList<Track *> tracks)
{
    QSet<Disk *> disks;

    for (const Track *track : tracks) {
        disks << track->disk();
    }

    return disks.values();
}

/************************************************
 *
 ************************************************/
TagEditor::TagEditor(const QList<Track *> &tracks, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagEditor),
    mTracks(tracks),
    mDisks(disksfromTracks(tracks))
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

    ui->performerEdit->setTagId(TrackTags::TagId::Performer);
    ui->albumPerformerEdit->setTagId(AlbumTags::TagId::AlbumPerformer);
    ui->songWriterEdit->setTagId(TrackTags::TagId::SongWriter);
    ui->albumEdit->setTagId(AlbumTags::TagId::Album);
    ui->dateEdit->setTagId(TrackTags::TagId::Date);
    ui->genreEdit->setTagId(TrackTags::TagId::Genre);
    ui->trackTitleEdit->setTagId(TrackTags::TagId::Title);
    ui->commentEdit->setTagId(TrackTags::TagId::Comment);

    updateWidgets();

    int width  = Settings::i()->value(TAG_EDIT_DIALOG_WIDTH_KEY).toInt();
    int height = Settings::i()->value(TAG_EDIT_DIALOG_HEIGHT_KEY).toInt();
    resize(width, height);
}

/************************************************
 *
 ************************************************/
TagEditor::~TagEditor()
{
    Settings::i()->setValue(TAG_EDIT_DIALOG_WIDTH_KEY, size().width());
    Settings::i()->setValue(TAG_EDIT_DIALOG_HEIGHT_KEY, size().height());
    Settings::i()->sync();

    delete ui;
}

/**************************************
 *
 **************************************/
void TagEditor::done(int res)
{
    if (res) {
        apply();
    }

    QDialog::done(res);
}

/**************************************
 *
 **************************************/
void TagEditor::updateWidgets()
{
    QSet<Disk *> disks;

    for (const Track *track : mTracks) {
        disks << track->disk();
    }

    QSet<int> startTrackNum;
    QSet<int> trackCount;
    QSet<int> diskNum;
    QSet<int> diskCount;

    for (const Track *track : mTracks) {
        startTrackNum << track->disk()->startTrackNum();
        trackCount << track->disk()->tracks().count();
        diskNum << track->disk()->discNumTag();
        diskCount << track->disk()->discCountTag();
    }

    for (auto *edit : findChildren<TrackTagLineEdit *>()) {
        edit->loadFromTracks(mTracks);
    }

    for (auto *edit : findChildren<TrackTagTextEdit *>()) {
        edit->loadFromTracks(mTracks);
    }

    for (auto *edit : findChildren<AlbumTagLineEdit *>()) {
        edit->loadFromDisks(mDisks);
    }

    ui->startTrackEdit->setMultiValue(startTrackNum);
    ui->trackCountEdit->setMultiValue(trackCount);
    ui->diskNumEdit->setMultiValue(diskNum);
    ui->diskCountEdit->setMultiValue(diskCount);
}

/**************************************
 *
 **************************************/
void TagEditor::apply()
{
    for (Disk *disk : mDisks) {
        // clang-format off
        if (ui->startTrackEdit->isModified()) disk->setStartTrackNum(ui->startTrackEdit->value());
        if (ui->diskNumEdit->isModified())    disk->setDiscNumTag(ui->diskNumEdit->value());
        if (ui->diskCountEdit->isModified())  disk->setDiscCountTag(ui->diskCountEdit->value());
        // clang-format off
    }

    for (auto *edit : findChildren<TrackTagLineEdit *>()) {
        edit->writeToTracks(mTracks);
    }

    for (auto *edit : findChildren<TrackTagTextEdit *>()) {
        edit->writeToTracks(mTracks);
    }

    for (auto *edit : findChildren<AlbumTagLineEdit *>()) {
        edit->writeToDisks(mDisks);
    }
}
