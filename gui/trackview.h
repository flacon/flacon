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


#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QTreeView>
#include <QPoint>

class TrackViewModel;
class TrackViewDelegate;
class Project;
class Track;
class Disk;

class TrackView : public QTreeView
{
    Q_OBJECT
public:
    enum Columns
    {
        ColumnPercent   = 0,
        ColumnTracknum  = 0,
        ColumnDuration  = 1,
        ColumnTitle     = 2,
        ColumnArtist    = 3,
        ColumnAlbum     = 4,
        ColumnComment   = 5,
        ColumnFileName  = 6,
        ColumnCount     = 7
    };

    explicit TrackView(QWidget *parent = 0);

    QList<Track*> selectedTracks() const;
    QList<Disk*> selectedDisks() const;

    TrackViewModel *model() const { return mModel; }

public slots:
    void layoutChanged();
    void selectDisk(const Disk *disk);
    void downloadStarted(const Disk &disk);
    void downloadFinished(const Disk &disk);
    void update(const Track &track);
    void update(const Disk &disk);

signals:
    void selectCueFile(Disk *disk);
    void selectAudioFile(Disk *disk);
    void selectCoverImage(Disk *disk);
    void downloadInfo(Disk *disk);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void keyPressEvent(QKeyEvent *event);

private slots:
    void headerContextMenu(QPoint pos);
    void showHideColumn(bool show);

    void showTrackMenu(const QModelIndex &index, const QRect &buttonRect);
    void emitSelectAudioFile(const QModelIndex &index, const QRect &buttonRect);
    void emitSelectCoverImage(const QModelIndex &index);

    void openEditor();

private:
    TrackViewModel *mModel;
    TrackViewDelegate *mDelegate;
};


class TrackViewSelectionModel: public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit TrackViewSelectionModel(QAbstractItemModel *model, QObject *parent);

public slots:
    virtual void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);
};

#endif // TRACKVIEW_H
