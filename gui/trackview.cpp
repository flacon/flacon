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


/************************************************

 ************************************************/
TrackView::TrackView(QWidget *parent):
    QTreeView(parent)
{
    mDelegate = new TrackViewDelegate(this);
    setItemDelegate(mDelegate);

    connect(mDelegate, SIGNAL(trackButtonClicked(QModelIndex,QRect)), this, SLOT(showTrackMenu(QModelIndex,QRect)));
    connect(mDelegate, SIGNAL(audioButtonClicked(QModelIndex,QRect)), this, SLOT(emitSelectAudioFile(QModelIndex, QRect)));

    mModel = new TrackViewModel(this);
    setModel(mModel);

    //self._selectionModel = TrackViewSelectionModel(mModel)
    setSelectionModel(new TrackViewSelectionModel(mModel, this));

    //#self.setMouseTracking(True)
    setUniformRowHeights(false);

    // Context menu ....................................
    setContextMenuPolicy(Qt::DefaultContextMenu);

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
        QObject *obj = static_cast<QObject*>(index.internalPointer());
        Track *track = qobject_cast<Track*>(obj);
        if (track)
            res << track;
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
        QObject *obj = static_cast<QObject*>(index.internalPointer());
        Track *track = qobject_cast<Track*>(obj);
        if (track)
            set << track->disk();

        Disk *disk =  qobject_cast<Disk*>(obj);
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
        QObject *obj = static_cast<QObject*>(index.internalPointer());

        Disk *d =  qobject_cast<Disk*>(obj);
        if (d && d == disk)
        {
            this->selectionModel()->select(index, QItemSelectionModel::Clear | QItemSelectionModel::Select);
            break;
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
    QObject *obj = static_cast<QObject*>(index.internalPointer());
    Disk *disk = qobject_cast<Disk*>(obj);
    if(!disk)
        return;


    QMenu menu;

    foreach(TagSet *tags, disk->tagSets())
    {
        TagSetAction *act = new TagSetAction(tags->title(), &menu, disk, tags);
        act->setCheckable(true);
        act->setChecked(tags->uri() == disk->tagsUri());
        connect(act, SIGNAL(triggered()), this, SLOT(activateTrackSet()));
        menu.addAction(act);
    }

    menu.addSeparator();

    QAction *act;

    act = new DiskAction(Project::getIcon("document-open", "fileopen", ":/icons/16/select-cue-file"), tr("Select another CUE file"), &menu, disk);
    connect(act, SIGNAL(triggered()), this, SLOT(emitSelectCueFile()));
    menu.addAction(act);

    act = new QAction(Project::getIcon("download", "web-browser", "network", ":/icons/16/download-track-info"), tr("Get data from CDDB"), &menu);
    connect(act, SIGNAL(triggered()), disk, SLOT(downloadInfo()));
    act->setEnabled(disk->canDownloadInfo());
    menu.addAction(act);

    QPoint vpPos = viewport()->pos() + visualRect(index).topLeft();
    QPoint p = buttonRect.bottomLeft() + vpPos + QPoint(0, 2);
    menu.exec(mapToGlobal(p));
}


/************************************************

 ************************************************/
void TrackView::emitSelectCueFile()
{
    DiskAction *act = qobject_cast<DiskAction*>(sender());
    if (act && act->disk())
        emit selectCueFile(act->disk());
}


/************************************************

 ************************************************/
void TrackView::emitSelectAudioFile(const QModelIndex &index, const QRect &buttonRect)
{
    Disk *disk = mModel->diskByIndex(index);
    if (disk)
        emit selectAudioFile(disk);
}


/************************************************

 ************************************************/
void TrackView::emitSelectAudioFile()
{
    DiskAction *act = qobject_cast<DiskAction*>(sender());
    if (act && act->disk())
        emit selectAudioFile(act->disk());
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
void TrackView::contextMenuEvent(QContextMenuEvent *event)
{
    event->ignore();

    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    Disk *disk = 0;
    QModelIndex diskIndex;

    Track *track = mModel->trackByIndex(index);
    if (track)
    {
        disk = track->disk();
        diskIndex = index.parent();
    }
    else
    {
        disk = mModel->diskByIndex(index);
        if (disk)
            diskIndex = index;
    }

    if (!disk)
        return;


    QMenu menu;
    QMenu editMenu(tr("Edit"), &menu);

    for(int i=0; i< mModel->columnCount(QModelIndex()); ++i)
    {
        if (!isColumnHidden(i) &&
            mModel->flags(mModel->index(0, i, diskIndex)).testFlag(Qt::ItemIsEditable))
        {
            QAction *act = new QAction(&menu);
            act->setText(mModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
            act->setData(i);
            connect(act, SIGNAL(triggered()), this, SLOT(openEditor()));
            editMenu.addAction(act);
        }
    }

    menu.addMenu(&editMenu);
    menu.addSeparator();


    DiskAction *act = new DiskAction(Project::getIcon("document-open", "fileopen", ":/icons/16/select-audio-file"), tr("Select another audio file"), &menu, disk);
    connect(act, SIGNAL(triggered()), this, SLOT(emitSelectAudioFile()));
    menu.addAction(act);

    act = new DiskAction(Project::getIcon("document-open", "fileopen", ":/icons/16/select-cue-file"), tr("Select another CUE file"), &menu, disk);
    connect(act, SIGNAL(triggered()), this, SLOT(emitSelectCueFile()));
    menu.addAction(act);

    act = new DiskAction(Project::getIcon("download", "web-browser", "network", ":/icons/16/download-track-info"), tr("Get data from CDDB"), &menu, disk);
    connect(act, SIGNAL(triggered()), disk, SLOT(downloadInfo()));
    act->setEnabled(disk->canDownloadInfo());
    menu.addAction(act);

    menu.exec(event->globalPos());
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


/************************************************

 ************************************************/
void TrackView::activateTrackSet()
{
    TagSetAction *act = qobject_cast<TagSetAction*>(sender());
    if (!act)
        return;

    act->disk()->activateTagSet(act->tagSet());
}

