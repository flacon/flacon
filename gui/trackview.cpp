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

#include "disc.h"
#include "trackview.h"
#include "trackviewmodel.h"
#include "trackviewdelegate.h"
#include "project.h"
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QAction>
#include <QFlags>
#include <QContextMenuEvent>
#include "../internet/dataprovider.h"

#include <QDebug>

#include "gui/tageditor/tageditor.h"

/************************************************

 ************************************************/
TrackView::TrackView(QWidget *parent) :
    QTreeView(parent)
{
    mDelegate = new TrackViewDelegate(this);
    setItemDelegate(mDelegate);

    connect(mDelegate, &TrackViewDelegate::trackButtonClicked,
            this, &TrackView::showTrackMenu);

    connect(mDelegate, &TrackViewDelegate::audioButtonClicked,
            this, &TrackView::audioButtonClicked);

    connect(mDelegate, &TrackViewDelegate::coverImageClicked,
            this, &TrackView::emitSelectCoverImage);

    mModel = new TrackViewModel(this);
    TrackView::setModel(mModel);

    TrackView::setSelectionModel(new TrackViewSelectionModel(mModel, this));

    setUniformRowHeights(false);
    this->setAttribute(Qt::WA_MacShowFocusRect, false);

    // Context menu ....................................
    setContextMenuPolicy(Qt::CustomContextMenu);

    header()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(header(), &QHeaderView::customContextMenuRequested,
            this, &TrackView::headerContextMenu);
}

/************************************************

 ************************************************/
QList<Track *> TrackView::selectedTracks() const
{
    QList<Track *>  res;
    QModelIndexList idxs = selectionModel()->selectedIndexes();
    foreach (QModelIndex index, idxs) {
        if (index.column() == 0) {
            Track *track = mModel->trackByIndex(index);
            if (track)
                res << track;
        }
    }

    return res;
}

/************************************************

 ************************************************/
QList<Disc *> TrackView::selectedDiscs() const
{
    QSet<Disc *>    set;
    QModelIndexList idxs = selectionModel()->selectedIndexes();
    foreach (QModelIndex index, idxs) {
        Disc *disc = mModel->discByIndex(index);
        if (disc)
            set << disc;
    }

    QList<Disc *> res;
    foreach (Disc *disc, set) {
        res << disc;
    }

    return res;
}

/************************************************
 *
 ************************************************/
bool TrackView::isSelected(const Disc &disc) const
{
    return selectionModel()->isSelected(mModel->index(disc));
}

/************************************************
 *
 ************************************************/
bool TrackView::isSelected(const Track &track) const
{
    return selectionModel()->isSelected(mModel->index(track, 0));
}

/************************************************

 ************************************************/
void TrackView::layoutChanged()
{
    for (int i = 0; i < Project::instance()->count(); ++i)
        setFirstColumnSpanned(i, QModelIndex(), true);

    expandAll();
}

/************************************************
 *
 ************************************************/
void TrackView::selectDisc(const Disc *disc)
{
    for (int i = 0; i < this->model()->rowCount(); ++i) {
        QModelIndex index = this->model()->index(i, 0);

        Disc *d = mModel->discByIndex(index);
        if (d && d == disc) {
            this->selectionModel()->select(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
            break;
        }
    }
}

/************************************************
 *
 ************************************************/
void TrackView::downloadStarted(const Disc &disc)
{
    mModel->downloadStarted(disc);
}

/************************************************
 *
 ************************************************/
void TrackView::downloadFinished(const Disc &disc)
{
    mModel->downloadFinished(disc);
}

/************************************************
 *
 ************************************************/
void TrackView::update(const Track &track)
{
    QModelIndex idx = mModel->index(track, 0);
    QTreeView::update(idx);

    int cnt = mModel->columnCount(idx.parent());
    for (int i = 1; i < cnt; ++i)
        QTreeView::update(mModel->index(track, i));

    QTreeView::update(idx.parent());
}

/************************************************
 *
 ************************************************/
void TrackView::update(const Disc &disc)
{
    QModelIndex idx = mModel->index(disc, 0);
    QTreeView::update(idx);

    int rows = mModel->rowCount(idx);
    int cols = mModel->columnCount(idx);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QTreeView::update(mModel->index(r, c, idx));
        }
    }
}

/************************************************
 *
 ************************************************/
void TrackView::updateAll()
{
    emit layoutChanged();
}

/************************************************

 ************************************************/
void TrackView::headerContextMenu(const QPoint &pos)
{
    QMenu menu;

    for (int i = 1; i < model()->columnCount(QModelIndex()); ++i) {
        QAction *act = new QAction(&menu);
        act->setText(model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        act->setData(i);
        act->setCheckable(true);
        act->setChecked(!isColumnHidden(i));
        connect(act, &QAction::toggled,
                this, &TrackView::showHideColumn);
        menu.addAction(act);
    }

    menu.exec(mapToGlobal(pos));
}

/************************************************

 ************************************************/
void TrackView::showHideColumn(bool show)
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (act)
        setColumnHidden(act->data().toInt(), !show);
}

/************************************************

 ************************************************/
void TrackView::showTrackMenu(const QModelIndex &index, const QRect &buttonRect)
{
    Disc *disc = mModel->discByIndex(index);
    if (!disc)
        return;

    QMenu menu;
    foreach (const TagsId &tags, disc->tagSets()) {
        QAction *act = new QAction(tags.title, &menu);
        act->setCheckable(true);
        act->setChecked(tags.uri == disc->currentTagSet().uri);
        connect(act, &QAction::triggered, [disc, tags]() { disc->activateTagSet(tags.uri); });
        menu.addAction(act);
    }

    menu.addSeparator();

    QAction *act;

    act = new QAction(tr("Select another CUE file…"), &menu);
    connect(act, &QAction::triggered, [this, disc] { this->selectCueFile(disc); });
    menu.addAction(act);

    act = new QAction(tr("Get data from Internet"), &menu);
    act->setEnabled(DataProvider::canDownload(*disc));
    connect(act, &QAction::triggered, [this, disc]() { emit downloadInfo(disc); });
    menu.addAction(act);

    QPoint vpPos = viewport()->pos() + visualRect(index).topLeft();
    QPoint p     = buttonRect.bottomLeft() + vpPos + QPoint(0, 2);
    menu.exec(mapToGlobal(p));
}

/************************************************

 ************************************************/
void TrackView::audioButtonClicked(const QModelIndex &index, int audioFileNum, const QRect &buttonRect)
{
    Disc *disc = mModel->discByIndex(index);
    if (disc) {
        if (audioFileNum < 0) {
            emit showAudioMenu(disc, buttonRect.bottomLeft());
        }
        else {
            emit selectAudioFile(disc, audioFileNum);
        }
    }
}

/************************************************
 *
 ************************************************/
void TrackView::emitSelectCoverImage(const QModelIndex &index)
{
    Disc *disc = mModel->discByIndex(index);
    if (disc)
        emit selectCoverImage(disc);
}

/************************************************

 ************************************************/
TrackViewSelectionModel::TrackViewSelectionModel(QAbstractItemModel *model, QObject *parent) :
    QItemSelectionModel(model, parent)
{
}

/************************************************

 ************************************************/
void TrackViewSelectionModel::select(const QItemSelection &selection, SelectionFlags command)
{
    if (selection.count() == 0)
        return;

    QItemSelection newSelection = selection;

    QModelIndexList idxs = selection.indexes();
    foreach (const QModelIndex &index, idxs) {
        if (index.parent().isValid())
            continue;

        QModelIndex index1 = model()->index(0, 0, index);
        QModelIndex index2 = model()->index(model()->rowCount(index) - 1, model()->columnCount(index) - 1, index);
        newSelection.select(index1, index2);
    }

    QItemSelectionModel::select(newSelection, command);
}

/************************************************

 ************************************************/
void TrackView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    mDelegate->drawBranch(painter, rect, index);
}

/************************************************

 ************************************************/
void TrackView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Left: {
            QModelIndex parent = selectionModel()->currentIndex().parent();
            int         row    = selectionModel()->currentIndex().row();
            for (int i = selectionModel()->currentIndex().column() - 1; i >= 0; --i) {
                QModelIndex idx = model()->index(row, i, parent);
                if (idx.isValid() && !isColumnHidden(i)) {
                    selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
                    return;
                }
            }
            break;
        }

        case Qt::Key_Right: {

            QModelIndex parent = selectionModel()->currentIndex().parent();
            int         cnt    = model()->columnCount(parent);
            int         row    = selectionModel()->currentIndex().row();
            for (int i = selectionModel()->currentIndex().column() + 1; i < cnt; ++i) {
                QModelIndex idx = model()->index(row, i, parent);
                if (idx.isValid() && !isColumnHidden(i)) {
                    selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
                    return;
                }
            }
            break;
        }

        default:
            QTreeView::keyPressEvent(event);
    }
}
