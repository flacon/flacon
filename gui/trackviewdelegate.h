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


#ifndef TRACKVIEWDELEGATE_H
#define TRACKVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QMovie>

class Track;
class Disk;
class TrackView;
class TrackViewCache;
class DataProvider;

class TrackViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TrackViewDelegate(TrackView *parent);
    ~TrackViewDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawBranch(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

signals:
    void trackButtonClicked(const QModelIndex &index, const QRect &buttonRect);
    void audioButtonClicked(const QModelIndex &index, const QRect &buttonRect);
    void markClicked(const QModelIndex &index, const QRect &buttonRect);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private slots:
    void movieUpdated();
    void downloadingStarted(DataProvider *provider);
    void downloadingFinished(DataProvider *provider);

private:
    TrackView *mTrackView;
    TrackViewCache *mCache;

    mutable QImage mNoCoverImg;
    QPixmap mTrackBtnPix;
    QPixmap mAudioBtnPix;
    QPixmap mWarnPix;
    QPixmap mOkPix;
    QPixmap mErrorPix;
    QMovie mDownloadMovie;
    mutable int mDiskHeightHint;

    QFont titleFont(const QFont &font) const;
    QFont filesFont(const QFont &font) const;
    void paintTrack(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const Track *track) const;
    void paintDisk(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const Disk *disk) const;
    QRect drawLabel(const QString &text, QRect rect, QPainter *painter) const;
    QRect drawFile(const QString &text, QRect rect, QPainter *painter) const;
    void drawSelectionMark(QPainter *painter, const QRect &rect) const;
};

#endif // TRACKVIEWDELEGATE_H
