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

#include "gui/controls.h"

#include "track.h"
#include "disc.h"

/************************************************
 *
 ************************************************/
template <class Control>
static void initControlValue(const QList<Track *> &tracks, const QList<Control *> &controls)
{
    foreach (auto *control, controls) {
        QSet<QString> values;
        foreach (Track *track, tracks) {
            values << track->tag_DEL(control->tagId());
        }

        control->setMultiValue(values);
    }
}

/************************************************
 *
 ************************************************/
static void initControlValue(const QList<Track *> &tracks, const QList<TagSpinBox *> &controls)
{
    foreach (auto *control, controls) {
        QSet<int> values;
        foreach (Track *track, tracks) {
            values << track->tag_DEL(control->tagId()).toInt();
        }

        control->setMultiValue(values);
    }
}

/************************************************
 *
 ************************************************/
TagEditor::TagEditor(const QList<Track *> &tracks, const QList<Disc *> &discs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagEditor),
    mTracks(tracks),
    mDiscs(discs),
    mStartTrackSpin(nullptr)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

    addLineEdit(TagId::Artist, tr("Artist:", "Music tag name"));
    addLineEdit(TagId::AlbumArtist, tr("Album performer:", "Music tag name"));
    addLineEdit(TagId::Album, tr("Album:", "Music tag name"));
    addLineEdit(TagId::Genre, tr("Genre:", "Music tag name"));
    addLineEdit(TagId::Date, tr("Year:", "Music tag name"));

    mStartTrackSpin = new MultiValuesSpinBox(this);
    mStartTrackSpin->setMinimum(1);
    mStartTrackSpin->setMaximum(99);

    TagSpinBox *trackCountSpin = new TagSpinBox(this);
    trackCountSpin->setMinimum(1);
    trackCountSpin->setMaximum(99);
    trackCountSpin->setTagId(TagId::TrackCount);
    this->add2Widget(mStartTrackSpin, trackCountSpin, tr("Start track number:", "Music tag name"));

    addIntEditNumCount(TagId::DiscNum, TagId::DiscCount, tr("Disc number:", "Music tag name"));

    addLineEdit(TagId::Title, tr("Track title:", "Music tag name"));
    addTextEdit(TagId::Comment, tr("Comment:", "Music tag name"));

    // Set values ______________________________________________
    initControlValue(tracks, this->findChildren<TagLineEdit *>());
    initControlValue(tracks, this->findChildren<TagTextEdit *>());
    initControlValue(tracks, this->findChildren<TagSpinBox *>());

    QSet<int> values;
    foreach (Disc *disc, discs) {
        values << disc->startTrackNum();
    }

    mStartTrackSpin->setMultiValue(values);
}

/************************************************
 *
 ************************************************/
TagEditor::~TagEditor()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
template <class Control>
static void setValue(const QList<Track *> &tracks, const QList<Control *> &controls)
{
    foreach (Control *edit, controls) {
        if (!edit->isModified())
            continue;

        foreach (Track *track, tracks) {
            track->setTag_DEL(edit->tagId(), edit->text());
        }
    }
}

/************************************************
 *
 ************************************************/
static void setValue(const QList<Track *> &tracks, const QList<TagSpinBox *> &controls)
{
    foreach (TagSpinBox *edit, controls) {
        if (!edit->isModified())
            continue;

        foreach (Track *track, tracks) {
            track->setTag_DEL(edit->tagId(), edit->text());
        }
    }
}

/************************************************
 *
 ************************************************/
void TagEditor::done(int res)
{
    if (!res) {
        QDialog::done(res);
        return;
    }

    setValue(mTracks, this->findChildren<TagLineEdit *>());
    setValue(mTracks, this->findChildren<TagTextEdit *>());
    setValue(mTracks, this->findChildren<TagSpinBox *>());

    if (mStartTrackSpin->isModified()) {
        foreach (Disc *disc, mDiscs) {
            disc->setStartTrackNum(mStartTrackSpin->value());
        }
    }

    QDialog::done(res);
}

/************************************************
 *
 ************************************************/
void TagEditor::add2Widget(QWidget *widget1, QWidget *widget2, const QString &label)
{
    int row = ui->layout->rowCount();
    {
        QLabel *lbl = new QLabel(this);
        lbl->setText(label);
        lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());
        ui->layout->addWidget(lbl, row, 0);
    }

    ui->layout->addWidget(widget1, row, 1);

    {
        QLabel *lbl = new QLabel(this);
        lbl->setText(tr("of"));
        lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());
        ui->layout->addWidget(lbl, row, 2);
    }

    ui->layout->addWidget(widget2, row, 3);
}

/************************************************
 *
 ************************************************/
void TagEditor::addLineEdit(TagId tagId, const QString &label)
{
    QLabel *lbl = new QLabel(this);
    lbl->setText(label);
    lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());

    QGridLayout *layout = ui->layout;
    layout->addWidget(lbl, layout->rowCount(), 0);

    TagLineEdit *edit = new TagLineEdit(this);
    edit->setTagId(tagId);
    layout->addWidget(edit, layout->rowCount() - 1, 1, 1, 3);
}

/************************************************
 *
 ************************************************/
void TagEditor::addTextEdit(TagId tagId, const QString &label)
{
    QLabel *lbl = new QLabel(this);
    lbl->setText(label);
    lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());

    QGridLayout *layout = ui->layout;
    layout->addWidget(lbl, layout->rowCount(), 0);

    TagTextEdit *edit = new TagTextEdit(this);
    edit->setTagId(tagId);
    layout->addWidget(edit, layout->rowCount() - 1, 1, 1, 3);
}

/************************************************
 *
 ************************************************/
void TagEditor::addIntEditNumCount(TagId numTagId, TagId cntTagId, const QString &numLabel)
{
    int row = ui->layout->rowCount();
    {
        QLabel *lbl = new QLabel(this);
        lbl->setText(numLabel);
        lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());
        ui->layout->addWidget(lbl, row, 0);
    }

    {
        TagSpinBox *edit = new TagSpinBox(this);
        edit->setMinimum(1);
        edit->setTagId(numTagId);
        ui->layout->addWidget(edit, row, 1);
    }

    {
        QLabel *lbl = new QLabel(this);
        lbl->setText(tr("of"));
        lbl->setSizePolicy(QSizePolicy::Maximum, lbl->sizePolicy().verticalPolicy());
        ui->layout->addWidget(lbl, row, 2);
    }

    {
        TagSpinBox *edit = new TagSpinBox(this);
        edit->setMinimum(1);
        edit->setTagId(cntTagId);
        ui->layout->addWidget(edit, row, 3);
    }
}

/************************************************
 *
 ************************************************/
void TagEditor::addIntEdit(TagId tagId, const QString &label)
{
    QLabel *lbl = new QLabel(this);
    lbl->setText(label);

    QGridLayout *layout = ui->layout;
    layout->addWidget(lbl, layout->rowCount(), 0);

    TagSpinBox *edit = new TagSpinBox(this);
    edit->setTagId(tagId);
    layout->addWidget(edit, layout->rowCount() - 1, 1);
}
