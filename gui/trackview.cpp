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


#include "disk.h"
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

#include <QDebug>

#include "gui/tageditor/tageditor.h"

/************************************************

 ************************************************/
TrackView::TrackView(QWidget *parent):
    QTreeView(parent)
{
    mDelegate = new TrackViewDelegate(this);
    setItemDelegate(mDelegate);

    connect(mDelegate, SIGNAL(trackButtonClicked(QModelIndex,QRect)),
            this, SLOT(showTrackMenu(QModelIndex,QRect)));

    connect(mDelegate, SIGNAL(audioButtonClicked(QModelIndex,QRect)),
            this, SLOT(emitSelectAudioFile(QModelIndex, QRect)));

    connect(mDelegate, SIGNAL(coverImageClicked(QModelIndex)),
            this, SLOT(emitSelectCoverImage(QModelIndex)));

    mModel = new TrackViewModel(this);
    setModel(mModel);

    setSelectionModel(new TrackViewSelectionModel(mModel, this));

    setUniformRowHeights(false);
    this->setAttribute(Qt::WA_MacShowFocusRect, false);

    // Context menu ....................................
    setContextMenuPolicy(Qt::CustomContextMenu);

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(headerContextMenu(QPoint)));
}


/************************************************

 ************************************************/
QList<Track*> TrackView::selectedTracks() const
{
    QList<Track*> res;
    QModelIndexList idxs = selectionModel()->selectedIndexes();
    foreach(QModelIndex index, idxs)
    {
        if (index.column() == 0)
        {
            Track *track = mModel->trackByIndex(index);
            if (track)
                res << track;
        }
    }

    return res;
}


/************************************************

 ************************************************/
QList<Disk*> TrackView::selectedDisks() const
{
    QSet<Disk*> set;
    QModelIndexList idxs = selectionModel()->selectedIndexes();
    foreach(QModelIndex index, idxs)
    {
        Disk *disk =  mModel->diskByIndex(index);
        if (disk)
            set << disk;
    }

    QList<Disk*> res;
    foreach (Disk *disk, set)
    {
        res << disk;
    }

    return res;
}


/************************************************
 *
 ************************************************/
bool TrackView::isSelected(const Disk &disk) const
{
    return selectionModel()->isSelected(mModel->index(disk));
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
    for(int i=0; i < project->count(); ++i)
        setFirstColumnSpanned(i, QModelIndex(), true);

    expandAll();
}


/************************************************
 *
 ************************************************/
void TrackView::selectDisk(const Disk *disk)
{
    for (int i=0; i<this->model()->rowCount(); ++i)
    {
        QModelIndex index = this->model()->index(i, 0);

        Disk *d =  mModel->diskByIndex(index);
        if (d && d == disk)
        {
            this->selectionModel()->select(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
            break;
        }
    }
}


/************************************************
 *
 ************************************************/
void TrackView::downloadStarted(const Disk &disk)
{
    mModel->downloadStarted(disk);
}


/************************************************
 *
 ************************************************/
void TrackView::downloadFinished(const Disk &disk)
{
    mModel->downloadFinished(disk);
}


/************************************************
 *
 ************************************************/
void TrackView::update(const Track &track)
{
    QModelIndex idx = mModel->index(track, 0);
    QTreeView::update(idx);

    int cnt = mModel->columnCount(idx.parent());
    for (int i=1; i<cnt; ++i)
        QTreeView::update(mModel->index(track, i));

    QTreeView::update(idx.parent());
}


/************************************************
 *
 ************************************************/
void TrackView::update(const Disk &disk)
{
    QModelIndex idx = mModel->index(disk, 0);
    QTreeView::update(idx);

    int rows = mModel->rowCount(idx);
    int cols = mModel->columnCount(idx);
    for (int r=0; r<rows; ++r)
    {
        for (int c=0; c<cols; ++c)
        {
            QTreeView::update(mModel->index(r, c, idx));
        }
    }
}


/************************************************

 ************************************************/
void TrackView::headerContextMenu(QPoint pos)
{
    QMenu menu;

    for (int i=1; i < model()->columnCount(QModelIndex()); ++i)
    {
        QAction *act = new QAction(&menu);
        act->setText(model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
        act->setData(i);
        act->setCheckable(true);
        act->setChecked(! isColumnHidden(i));
        connect(act, SIGNAL(toggled(bool)), this, SLOT(showHideColumn(bool)));
        menu.addAction(act);
    }

    menu.exec(mapToGlobal(pos));
}


/************************************************

 ************************************************/
void TrackView::showHideColumn(bool show)
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
        setColumnHidden(act->data().toInt(), !show);
}


/************************************************

 ************************************************/
void TrackView::showTrackMenu(const QModelIndex &index, const QRect &buttonRect)
{
    Disk *disk = mModel->diskByIndex(index);
    if(!disk)
        return;


    QMenu menu;
    foreach (const Disk::TagSet &tags, disk->tagSets())
    {
        QAction *act = new QAction(tags.name, &menu);
        act->setCheckable(true);
        act->setChecked(tags.uri == disk->tagsUri());
        connect(act, &QAction::triggered, [disk, tags](){ disk->activateTagSet(tags.uri); });
        menu.addAction(act);
    }

    menu.addSeparator();

    QAction *act;

    act = new QAction(tr("Select another CUE file…"), &menu);
    connect(act, &QAction::triggered, [this, disk] { this->selectCueFile(disk);});
    menu.addAction(act);

    act = new QAction(tr("Get data from CDDB"), &menu);
    act->setEnabled(disk->canDownloadInfo());
    connect(act, &QAction::triggered, [this, disk](){ emit downloadInfo(disk);});
    menu.addAction(act);

    QPoint vpPos = viewport()->pos() + visualRect(index).topLeft();
    QPoint p = buttonRect.bottomLeft() + vpPos + QPoint(0, 2);
    menu.exec(mapToGlobal(p));
}


/************************************************

 ************************************************/
void TrackView::emitSelectAudioFile(const QModelIndex &index, const QRect &buttonRect)
{
    Q_UNUSED(buttonRect);
    Disk *disk = mModel->diskByIndex(index);
    if (disk)
        emit selectAudioFile(disk);
}


/************************************************
 *
 ************************************************/
void TrackView::emitSelectCoverImage(const QModelIndex &index)
{
    Disk *disk = mModel->diskByIndex(index);
    if (disk)
        emit selectCoverImage(disk);
}


/************************************************

 ************************************************/
TrackViewSelectionModel::TrackViewSelectionModel(QAbstractItemModel *model, QObject *parent):
    QItemSelectionModel(model, parent)
{
}


/************************************************

 ************************************************/
void TrackViewSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if (selection.count() == 0)
        return;

    QItemSelection newSelection = selection;


    QModelIndexList idxs = selection.indexes();
    foreach (const QModelIndex &index, idxs)
    {
        if (index.parent().isValid())
            continue;

        QModelIndex index1 = model()->index(0, 0, index);
        QModelIndex index2 = model()->index(model()->rowCount(index)-1, model()->columnCount(index)-1, index);
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
    switch (event->key())
    {
    case Qt::Key_Left:
    {
        QModelIndex parent = selectionModel()->currentIndex().parent();
        int row = selectionModel()->currentIndex().row();
        for (int i=selectionModel()->currentIndex().column() - 1; i>=0; --i)
        {
            QModelIndex idx = model()->index(row, i, parent);
            if (idx.isValid() && !isColumnHidden(i))
            {
                selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
                return;
            }
        }
        break;
    }

    case Qt::Key_Right:
    {

        QModelIndex parent = selectionModel()->currentIndex().parent();
        int cnt = model()->columnCount(parent);
        int row = selectionModel()->currentIndex().row();
        for (int i=selectionModel()->currentIndex().column() + 1; i<cnt ; ++i)
        {
            QModelIndex idx = model()->index(row, i, parent);
            if (idx.isValid() && !isColumnHidden(i))
            {
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


/************************************************
 Open treeview inline editor
 ************************************************/
void TrackView::openEditor()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (!act)
        return;

    QModelIndex index = currentIndex();

    if (!index.parent().isValid())
    {
        index = index.child(0, 0);
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }

    if (index.isValid())
    {
        index = index.sibling(index.row(), act->data().toInt());
        edit(index);
    }
}



