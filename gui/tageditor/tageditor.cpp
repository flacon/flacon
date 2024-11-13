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

/************************************************
 *
 ************************************************/
TagEditor::TagEditor(const QList<Track *> &tracks, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagEditor),
    mTracks(tracks)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

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
    QSet<int> startTrackNum;
    QSet<int> trackCount;
    QSet<int> diskNum;
    QSet<int> diskCount;

    QSet<QString> performer;
    QSet<QString> albumPerformer;
    QSet<QString> songWriter;
    QSet<QString> album;
    QSet<QString> date;
    QSet<QString> genre;
    QSet<QString> trackTitle;
    QSet<QString> comment;

    for (const Track *track : mTracks) {
        startTrackNum << track->disk()->startTrackNum();
        trackCount << track->disk()->tracks().count();
        diskNum << track->disk()->discNumTag();
        diskCount << track->disk()->discCountTag();

        performer << track->performerTag();
        // albumPerformer << track->disk()->alb
        songWriter << track->songWriterTag();
        album << track->disk()->albumTag();
        date << track->dateTag();
        genre << track->genreTag();
        trackTitle << track->titleTag();
        comment << track->commentTag();
    }

    ui->startTrackEdit->setMultiValue(startTrackNum);
    ui->trackCountEdit->setMultiValue(trackCount);
    ui->diskNumEdit->setMultiValue(diskNum);
    ui->diskCountEdit->setMultiValue(diskCount);

    ui->performerEdit->setMultiValue(performer);
    ui->albumPerformerEdit->setMultiValue(albumPerformer);
    ui->songWriterEdit->setMultiValue(songWriter);
    ui->albumEdit->setMultiValue(album);
    ui->dateEdit->setMultiValue(date);
    ui->genreEdit->setMultiValue(genre);
    ui->trackTitleEdit->setMultiValue(trackTitle);
    ui->commentEdit->setMultiValue(comment);

    // ui->trackTitleValue->setMultiValue(trackTitle);
    // ui->trackPerformerValue->setMultiValue(trackPerformer);
    // ui->trackSongWriterValue->setMultiValue(trackSongWriter);
    // ui->trackDateValue->setMultiValue(trackDate);
    // ui->trackGenreValue->setMultiValue(trackGenre);
    // ui->trackIsrcValue->setMultiValue(trackIsrc);
    // ui->trackCommentValue->setMultiValue(trackComment);

    /*

    QSet<QString> diskAlbum;
    QSet<QString> diskPerformer;
    QSet<QString> diskSongWriter;
    QSet<QString> diskComment;

    for (const Disk *disk : mDiscs) {
        mDiskStartTrack << disk->startTrackNum();
        mDiskTrackCount << disk->trackCountTag();
        mDiskNum << disk->discNumTag();
        mDiskCount << disk->discCountTag();

        diskAlbum << disk->albumTag();
        diskPerformer << disk->performerTag();
        diskSongWriter << disk->songWriterTag();
        diskDate << disk->dateTag();
        diskGenre << disk->genreTag();
        diskComment << disk->commentTag();
    }

    ui->diskStartTrackValue->setMultiValue(mDiskStartTrack);
    ui->diskTrackCountValue->setMultiValue(mDiskTrackCount);
    ui->diskNumValue->setMultiValue(mDiskNum);
    ui->diskCountValue->setMultiValue(mDiskCount);

    ui->diskAlbumValue->setMultiValue(diskAlbum);
    ui->diskPerformerValue->setMultiValue(diskPerformer);
    ui->diskSongWriterValue->setMultiValue(diskSongWriter);
    ui->diskDateValue->setMultiValue(diskDate);
    ui->diskGenreValue->setMultiValue(diskGenre);
    ui->diskCommentValue->setMultiValue(diskComment);


    QSet<QString> trackPerformer;
    QSet<QString> trackSongWriter;
    QSet<QString> trackDate;
    QSet<QString> trackGenre;
    QSet<QString> trackIsrc;
    QSet<QString> trackComment;

    for (const Track *track : mTracks) {
        trackTitle << track->titleTag();
        trackPerformer << track->performerTag();
        trackSongWriter << track->songWriterTag();
        trackDate << track->dateTag();
        trackGenre << track->genreTag();
        trackIsrc << track->isrcTag();
        trackComment << track->commentTag();
    }

    ui->trackTitleValue->setMultiValue(trackTitle);
    ui->trackPerformerValue->setMultiValue(trackPerformer);
    ui->trackSongWriterValue->setMultiValue(trackSongWriter);
    ui->trackDateValue->setMultiValue(trackDate);
    ui->trackGenreValue->setMultiValue(trackGenre);
    ui->trackIsrcValue->setMultiValue(trackIsrc);
    ui->trackCommentValue->setMultiValue(trackComment);

    if (trackPerformer.count() == 1 && trackPerformer.cbegin()->isEmpty() && diskPerformer.count() == 1) {
        ui->trackPerformerValue->setPlaceholderText(*diskPerformer.cbegin());
    }

    if (trackSongWriter.count() == 1 && trackSongWriter.cbegin()->isEmpty() && diskSongWriter.count() == 1) {
        ui->trackSongWriterValue->setPlaceholderText(*diskSongWriter.cbegin());
    }

    if (trackDate.count() == 1 && trackDate.cbegin()->isEmpty() && diskDate.count() == 1) {
        ui->trackDateValue->setPlaceholderText(*diskDate.cbegin());
    }

    if (trackGenre.count() == 1 && trackGenre.cbegin()->isEmpty() && diskGenre.count() == 1) {
        ui->trackGenreValue->setPlaceholderText(*diskGenre.cbegin());
    }
*/
}

/**************************************
 *
 **************************************/
void TagEditor::apply()
{
    /*
    for (Disk *disk : mDiscs) {
        // clang-format off
        if (ui->diskStartTrackValue->isModified())  disk->setStartTrackNum(ui->diskStartTrackValue->value());
        if (ui->diskNumValue->isModified())         disk->setDiscNumTag(ui->diskNumValue->value());
        if (ui->diskCountValue->isModified())       disk->setDiscCountTag(ui->diskCountValue->value());

        if (ui->diskAlbumValue->isModified())       disk->setAlbumTag(ui->diskAlbumValue->text());
        if (ui->diskPerformerValue->isModified())   disk->setPerformerTag(ui->diskPerformerValue->text());
        if (ui->diskSongWriterValue->isModified())  disk->setSongWriterTag(ui->diskSongWriterValue->text());
        if (ui->diskDateValue->isModified())        disk->setDateTag(ui->diskDateValue->text());
        if (ui->diskGenreValue->isModified())       disk->setGenreTag(ui->diskGenreValue->text());
        if (ui->diskCommentValue->isModified())     disk->setCommentTag(ui->diskCommentValue->text());
        // clang-format on
    }

    for (Track *track : mTracks) {
        // clang-format off
        if (ui->trackTitleValue->isModified())      track->setTitleTag(ui->trackTitleValue->text());
        if (ui->trackPerformerValue->isModified())  track->setPerformerTag(ui->trackPerformerValue->text());
        if (ui->trackSongWriterValue->isModified()) track->setSongWriterTag(ui->trackSongWriterValue->text());
        if (ui->trackDateValue->isModified())       track->setDateTag(ui->trackDateValue->text());
        if (ui->trackGenreValue->isModified())      track->setGenreTag(ui->trackGenreValue->text());
        if (ui->trackIsrcValue->isModified())       track->setIsrcTag(ui->trackIsrcValue->text());
        if (ui->trackCommentValue->isModified())    track->setCommentTag(ui->trackCommentValue->text());
        // clang-format on
    }
*/
}
