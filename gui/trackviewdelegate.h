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
#include "movie.h"

class TrackView;
class TrackViewCache;
class DataProvider;

class TrackViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TrackViewDelegate(TrackView *parent);
    ~TrackViewDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void drawBranch(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void trackButtonClicked(const QModelIndex &index, const QRect &buttonRect);
    void audioButtonClicked(const QModelIndex &index, int fileNum, const QRect &buttonRect);
    void markClicked(const QModelIndex &index, const QRect &buttonRect);
    void coverImageClicked(const QModelIndex &index);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private slots:
    void movieUpdated();

private:
    TrackView      *mTrackView;
    TrackViewCache *mCache;

    mutable QImage mNoCoverImg;
    QPixmap        mTrackBtnPix;
    QPixmap        mAudioBtnPix;
    QPixmap        mDiscErrorPix;
    QPixmap        mDiscWarnPix;
    QPixmap        mTrackOkPix;
    QPixmap        mTrackErrorPix;
    mutable Movie  mDownloadMovie;
    mutable int    mDiscHeightHint  = 0;
    mutable int    mAudioFileHeight = 0;

    QFont titleFont(const QFont &font) const;
    QFont filesFont(const QFont &font) const;
    void  paintTrack(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void  paintDisc(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect drawCoverImage(QPainter *painter, const QRect &windowRect, const QModelIndex &index) const;
    QRect drawTitle(QPainter *painter, const QRect &windowRect, const QModelIndex &index) const;
    QRect drawMark(QPainter *painter, bool isWaiting, const QRect &imgRect, const QModelIndex &index) const;

    QRect drawButton(const QPixmap &pixmap, const QRect &windowRect, QPainter *painter) const;
    QRect drawLabel(const QString &text, const QRect &rect, QPainter *painter) const;
    QRect drawFile(const QString &text, const QRect &rect, QPainter *painter) const;
    void  drawSelectionMark(QPainter *painter, const QRect &rect) const;
};

#endif // TRACKVIEWDELEGATE_H
