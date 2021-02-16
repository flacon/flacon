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
class Disc;

class TrackView : public QTreeView
{
    Q_OBJECT
public:
    enum Columns {
        ColumnPercent  = 0,
        ColumnTracknum = 0,
        ColumnDuration = 1,
        ColumnTitle    = 2,
        ColumnArtist   = 3,
        ColumnAlbum    = 4,
        ColumnComment  = 5,
        ColumnFileName = 6,
        ColumnCount    = 7
    };

    explicit TrackView(QWidget *parent = nullptr);

    QList<Track *> selectedTracks() const;
    QList<Disc *>  selectedDiscs() const;

    bool isSelected(const Disc &disc) const;
    bool isSelected(const Track &track) const;

    TrackViewModel *model() const { return mModel; }

public slots:
    void layoutChanged();
    void selectDisc(const Disc *disc);
    void downloadStarted(const Disc &disc);
    void downloadFinished(const Disc &disc);
    void update(const Track &track);
    void update(const Disc &disc);

signals:
    void selectCueFile(Disc *disc);
    void selectAudioFile(Disc *disc, int audioFileNum);
    void selectCoverImage(Disc *disc);
    void downloadInfo(Disc *disc);

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void headerContextMenu(QPoint pos);
    void showHideColumn(bool show);

    void showTrackMenu(const QModelIndex &index, const QRect &buttonRect);
    void emitSelectAudioFile(const QModelIndex &index, int audioFileNum);
    void emitSelectCoverImage(const QModelIndex &index);

private:
    TrackViewModel *   mModel;
    TrackViewDelegate *mDelegate;
};

class TrackViewSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    explicit TrackViewSelectionModel(QAbstractItemModel *model, QObject *parent);

public slots:
    virtual void select(const QItemSelection &selection, SelectionFlags command) override;
};

#endif // TRACKVIEW_H
