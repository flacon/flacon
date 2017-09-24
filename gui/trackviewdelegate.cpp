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


#include "trackviewdelegate.h"
#include "trackview.h"
#include "trackviewmodel.h"
#include "project.h"
#include "internet/dataprovider.h"

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>

#include <QDebug>

#define SELECTION_MARK      8
#define MARGIN              6
#define TOP_PADDING        16
#define BOTTOM_PADDING      2
#define IMG_HEIGHT         60
#define MARK_HEIGHT        32
#define LINE_MARK_HEIGHT   22


struct TrackViewCacheItem
{
    TrackViewCacheItem()
    {
        isWaiting = false;
    }

    QRect trackBtn;
    QRect trackLbl;
    QRect audioBtn;
    QRect audioLbl;
    QRect markBtn;
    QRect coverRect;
    bool isWaiting;
};

class TrackViewCache
{
public:
    enum Keys {
        None,
        TrackBtn,
        AudioBtn,
        MarkBtn
    };

    TrackViewCache();
    ~TrackViewCache()
    {
        qDeleteAll(mItems);
    }

    TrackViewCacheItem *item(const QModelIndex &index);
    Keys clickType(const QModelIndex &index, const QPoint &point);

private:
    QHash<QModelIndex, TrackViewCacheItem*> mItems;
    QModelIndex currentIndex;
    TrackViewCacheItem *currentItem;
};


/************************************************

 ************************************************/
TrackViewCache::TrackViewCache():
    currentIndex(QModelIndex()),
    currentItem(0)
{
}


/************************************************

 ************************************************/
TrackViewCacheItem *TrackViewCache::item(const QModelIndex &index)
{
    if (currentIndex != index)
    {
        currentIndex = index;
        currentItem = mItems.value(index, 0);

        if (!currentItem)
        {
            currentItem = new TrackViewCacheItem();
            mItems.insert(currentIndex, currentItem);
        }
    }

    return currentItem;
}


/************************************************

 ************************************************/
TrackViewDelegate::TrackViewDelegate(TrackView *parent):
    QStyledItemDelegate(parent),
    mTrackView(parent),
    mCache(new TrackViewCache),
    mDiskHeightHint(0)
{
    mTrackBtnPix = QPixmap(":trackBtn");
    mAudioBtnPix = QPixmap(":audioBtn");
    mWarnPix = Project::getIcon("dialog-warning", "messagebox_warning", ":/icons/32/disk-warning").pixmap(MARK_HEIGHT, MARK_HEIGHT);
    mOkPix = Project::getIcon("dialog-ok", "button_ok", ":/icons/16/track-ok").pixmap(LINE_MARK_HEIGHT, LINE_MARK_HEIGHT);
    mErrorPix = Project::getIcon("dialog-cancel", "button_cancel",  ":/icons/16/track-cancel").pixmap(LINE_MARK_HEIGHT, LINE_MARK_HEIGHT);
    mNoCoverImg = QImage(":noCover");

    mDownloadMovie.setFileName(":wait");
    connect(project, SIGNAL(downloadingStarted(DataProvider*)), this, SLOT(downloadingStarted(DataProvider*)));
    connect(project, SIGNAL(downloadingFinished(DataProvider*)), this, SLOT(downloadingFinished(DataProvider*)));
    connect(&mDownloadMovie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated()));
}


/************************************************

 ************************************************/
TrackViewDelegate::~TrackViewDelegate()
{
    delete mCache;
}


/************************************************

 ************************************************/
void TrackViewDelegate::drawSelectionMark(QPainter *painter, const QRect &rect) const
{
    QRect r=rect;
    r.setWidth(SELECTION_MARK);
    painter->fillRect(r, mTrackView->palette().highlight().color());
}


/************************************************

 ************************************************/
void TrackViewDelegate::drawBranch(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QColor bgColor = mTrackView->palette().base().color();
    if (rect.isValid())
        painter->fillRect(rect, bgColor);

    if (mTrackView-> selectionModel()->isRowSelected(index.row(), index.parent()))
        drawSelectionMark(painter, rect);
}


/************************************************

 ************************************************/
void TrackViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;

    TrackViewModel::ItemType type = TrackViewModel::ItemType(index.data(TrackViewModel::RoleItemType).toInt());

    if (type == TrackViewModel::TrackItem)
    {
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
        if (index.row() % 2)
            painter->fillRect(option.rect, mTrackView->palette().base().color());
        else
            painter->fillRect(option.rect, mTrackView->palette().alternateBase().color());
#else
        if (index.row() % 2)
            opt.features &= ~QStyleOptionViewItemV2::Alternate;
        else
            opt.features |= QStyleOptionViewItemV2::Alternate;
#endif
        paintTrack(painter, opt, index);
        return;
    }


    if (type == TrackViewModel::DiskItem && index.column() == 0)
    {
        QColor bgColor = mTrackView->palette().base().color();
        painter->fillRect(opt.rect, bgColor);

        if (mTrackView-> selectionModel()->isSelected(index))
        {
            QRect rect = opt.rect;
            if (index.row() > 0)
                rect.setTop(rect.top() + TOP_PADDING);
            drawSelectionMark(painter, rect);
        }

        paintDisk(painter, opt, index);
        return;
    }
}


/************************************************

 ************************************************/
void TrackViewDelegate::paintTrack(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (index.column() != TrackView::ColumnPercent)
        return;

    const QPixmap *icon = 0;
    QString txt;
    int progress = index.data(TrackViewModel::RolePercent).toInt();

    switch (index.data(TrackViewModel::RoleStatus).toInt())
    {
    case Track::NotRunning: txt = "";               progress = -1;      break;
    case Track::Canceled:   txt = "";                                   break;
    case Track::Error:      txt = tr("Error");      icon = &mErrorPix;  break;
    case Track::Aborted:    txt = tr("Aborted");                        break;
    case Track::OK:         txt = tr("OK");         icon = &mOkPix;     break;
    case Track::Splitting:  txt = tr("Extracting");                     break;
    case Track::Encoding:   txt = tr("Encoding");                       break;
    case Track::Queued:     txt = tr("Queued");                         break;
    case Track::CalcGain:   txt = tr("Calculate gain");                 break;
    case Track::WaitGain:   txt = tr("Wait gain");                      break;
    case Track::WriteGain:  txt = tr("Write gain");                     break;

    }


    painter->save();
    painter->translate(option.rect.left() + 30, option.rect.top());
    QRect windowRect(0, 0, option.rect.width() - 31, option.rect.height());
    painter->setClipRect(windowRect);

    if (progress > -1)
    {
        QStyleOptionProgressBar opt;
        opt.rect = windowRect.adjusted(4, 3, -4, -3);
        opt.minimum = 0;
        opt.maximum = 100;
        opt.progress = progress;
        opt.text = QString("%1 %2%").arg(txt).arg(opt.progress);

        QApplication::style()->drawControl(QStyle::CE_ProgressBarContents, &opt, painter);
        QApplication::style()->drawControl(QStyle::CE_ProgressBarLabel, &opt, painter);
    }
    else
    {
        if (icon)
        {
            int textWidth = painter->fontMetrics().width(txt);
            int imgLeft = (windowRect.width() - LINE_MARK_HEIGHT - 4 - textWidth) / 2;
            painter->drawPixmap(imgLeft, (windowRect.height() - LINE_MARK_HEIGHT) / 2, *icon);

            QRect textRect(QPoint(imgLeft + LINE_MARK_HEIGHT + 4, 0), windowRect.bottomRight());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, txt);
        }
        else
        {
            painter->drawText(windowRect, Qt::AlignCenter | Qt::AlignVCenter, txt);
        }
    }

    painter->restore();
}


/************************************************

 ************************************************/
void TrackViewDelegate::paintDisk(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect paintRect = option.rect;
    paintRect.setLeft(0);

    painter->save();
    painter->setClipRect(option.rect);
    QFont titleFont = this->titleFont(painter->font());
    QFont filesFont = this->filesFont(painter->font());


    painter->translate(option.rect.topLeft());

    int topPadding = index.row() ? TOP_PADDING : 0;
    QRect windowRect(MARGIN + MARGIN,
                     MARGIN + topPadding,
                     option.rect.right() - 2 * MARGIN,
                     option.rect.height() - 2 * MARGIN - topPadding - BOTTOM_PADDING);


    TrackViewCacheItem *cache = mCache->item(index);

    // Draw cover image ................................
    QImage img = index.data(TrackViewModel::RoleCoverImg).value<QImage>();
    if (img.isNull())
        img = mNoCoverImg;

    if (img.height() != windowRect.height())
        img = img.scaledToHeight(windowRect.height(), Qt::SmoothTransformation);

    QRect imgRect(windowRect.topLeft(), img.size());
    painter->fillRect(imgRect, mTrackView->palette().base().color());
    painter->fillRect(imgRect, Qt::white);
    painter->drawImage(imgRect, img);
    cache->coverRect = imgRect;

    // Rectangle for text drawing ......................
    QRect textRect(windowRect);
    textRect.setLeft(imgRect.right() + MARGIN);

    // Draw album & artist .............................
    painter->setFont(titleFont);
    QString album =  index.sibling(index.row(), TrackView::ColumnAlbum).data().toString();
    QString artist = index.sibling(index.row(), TrackView::ColumnArtist).data().toString();
    if (!album.isEmpty() || !artist.isEmpty())
        painter->drawText(textRect, Qt::AlignLeft, QString("%1 / %2").arg(artist, album));

    // Draw audio filename .............................
    painter->setFont(filesFont);
    int th = painter->fontMetrics().height();
    int tTop = windowRect.bottom() - 2 * th - 2;
    int aTop = windowRect.bottom() - th + 1;

    // Draw labels ........
    QRect tLabelRect(textRect.left(), tTop, windowRect.width(), th);
    QRect aLabelRect(textRect.left(), aTop, windowRect.width(), th);

    tLabelRect = drawLabel(tr("Tracks:"), tLabelRect, painter);
    aLabelRect = drawLabel(tr("Audio:"),  aLabelRect, painter);

    // Draw filenames .....
    int l = qMax(tLabelRect.right(), aLabelRect.right()) + 6;
    QRect tFileRect(l, tTop, windowRect.width(), th);
    QRect aFileRect(l, aTop, windowRect.width(), th);

    tFileRect = drawFile(index.data(TrackViewModel::RoleTitle).toString(), tFileRect, painter);
    QFileInfo fi(index.data(TrackViewModel::RoleAudioFileName).toString());
    aFileRect = drawFile(fi.fileName(), aFileRect, painter);


    // Draw buttons ......
    l = qMax(tLabelRect.right() + 80, qMax(tFileRect.right(), aFileRect.right()) + 8);

    QRect tBtnRect(0, 0, mTrackBtnPix.height(), mTrackBtnPix.width());
    tBtnRect.moveCenter(tLabelRect.center());
    tBtnRect.moveLeft(l);
    painter->drawPixmap(tBtnRect, mTrackBtnPix);

    QRect aBtnRect(0, 0, mAudioBtnPix.height(), mAudioBtnPix.width());
    aBtnRect.moveCenter(aLabelRect.center());
    aBtnRect.moveLeft(l);
    painter->drawPixmap(aBtnRect, mAudioBtnPix);


    QRect tClickRect = tBtnRect.united(tLabelRect).adjusted(0, -3, 4, 1);
    cache->trackBtn = tClickRect;
    //painter->drawRect(tClickRect);

    QRect aClickRect = aBtnRect.united(aLabelRect).adjusted(0, -3, 4, 1);
    cache->audioBtn = aClickRect;
    //painter->drawRect(aClickRect);

    cache->trackLbl = QRect(tFileRect.topLeft(), tBtnRect.bottomLeft());
    cache->audioLbl = QRect(aFileRect.topLeft(), aBtnRect.bottomLeft());


    // Draw bottom line ................................
    painter->setPen(mTrackView->palette().dark().color());
    int y = option.rect.height() - BOTTOM_PADDING - 2;
    painter->drawLine(MARGIN * 2, y, windowRect.right(), y);

    // Draw warning mark ...............................
    QRect markRect(imgRect.right() - MARK_HEIGHT, imgRect.bottom() - MARK_HEIGHT, MARK_HEIGHT, MARK_HEIGHT);
    if (!index.data(TrackViewModel::RoleCanConvert).toBool())
        painter->drawPixmap(markRect, mWarnPix);
    cache->markBtn = markRect;

    cache->isWaiting = index.data(TrackViewModel::RoleIsDownloads).toBool();
    if (cache->isWaiting)
    {
        painter->drawPixmap(markRect, mDownloadMovie.currentPixmap());
    }

    painter->restore();    
}


/************************************************

 ************************************************/
QRect TrackViewDelegate::drawLabel(const QString &text, QRect rect, QPainter *painter) const
{
    QRect res;
    painter->save();
    painter->setPen(mTrackView->palette().dark().color());
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text, &res);
    painter->restore();
    return res;
}


/************************************************

 ************************************************/
QRect TrackViewDelegate::drawFile(const QString &text, QRect rect, QPainter *painter) const
{
    QRect res;
    if (!text.isEmpty())
    {
        painter->save();
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text, &res);
        painter->restore();
    }
    else
    {
        res = rect;
        res.setWidth(0);
    }
    return res;
}


/************************************************

 ************************************************/
QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize res = QStyledItemDelegate::sizeHint(option, index);

    if (!index.parent().isValid())
    {
        if (!mDiskHeightHint)
        {
            int h = 8;

            QFont titleFont = this->titleFont(option.font);
            QFont filesFont = this->filesFont(option.font);
            h += QFontMetrics(titleFont).height();
            h += QFontMetrics(filesFont).height() * 2;
            mDiskHeightHint = qMax(IMG_HEIGHT, h) + 2 * MARGIN + BOTTOM_PADDING; //For Line
        }

        res.rheight() = mDiskHeightHint;
        if (index.row())
            res.rheight() += TOP_PADDING;
        if (index.column() == 0)
            res.rwidth() = 600;
        else
            res.rwidth() = 0;
    }
    else
    {
        res.rheight() = res.height() + 8;
    }

    return res;
}


/************************************************

 ************************************************/
bool TrackViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.parent().isValid())
    {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *ev = static_cast<QMouseEvent*>(event);
        QPoint m = ev->pos() - option.rect.topLeft();

        TrackViewCacheItem *cache = mCache->item(index);

        if (cache->trackBtn.contains(m))
        {
            if (event->type() == QEvent::MouseButtonRelease)
                emit trackButtonClicked(index, cache->trackBtn);

            return true;
        }

        if (cache->audioBtn.contains(m))
        {
            if (event->type() == QEvent::MouseButtonRelease)
                emit audioButtonClicked(index, cache->audioBtn);

            return true;
        }

        if (cache->markBtn.contains(m))
        {
            if (event->type() == QEvent::MouseButtonRelease)
                emit markClicked(index, cache->markBtn);

            return true;
        }

        if (cache->coverRect.contains(m))
        {
            if (event->type() == QEvent::MouseButtonRelease)
                emit coverImageClicked(index);

            return true;
        }

        return false;
    }

    return true;
}


/************************************************

 ************************************************/
bool TrackViewDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.parent().isValid())
        return QStyledItemDelegate::helpEvent(event, view, option, index);

    TrackViewCacheItem *cache = mCache->item(index);
    if (cache == nullptr)
        return true;

    QPoint m = event->pos() - option.rect.topLeft();

    if (cache->trackLbl.contains(m))
    {
        QToolTip::showText(
                    event->globalPos(),
                    view->model()->data(index, TrackViewModel::RoleCueFilePath).toString(),
                    view);
        return true;
    }


    if (cache->audioLbl.contains(m))
    {
        QToolTip::showText(
                    event->globalPos(),
                    view->model()->data(index, TrackViewModel::RoleAudioFilePath).toString(),
                    view);
        return true;
    }

    if (cache->markBtn.contains(m))
    {
        QToolTip::showText(
                    event->globalPos(),
                    view->model()->data(index, TrackViewModel::RoleDiskWarning).toString(),
                    view);
        return true;
    }
    return false;
}


/************************************************

 ************************************************/
void TrackViewDelegate::movieUpdated()
{
    TrackViewModel *model = qobject_cast<TrackViewModel*>(mTrackView->model());
    if (!model)
        return;

    bool active = false;
    for(int i=0; i<model->rowCount(QModelIndex()); ++i)
    {
        QModelIndex index = model->index(i, 0, QModelIndex());
        if (mCache->item(index)->isWaiting)
        {
            project->emitDiskChanged(project->disk(0));
            active = true;
        }
    }

    if (!active)
        mDownloadMovie.stop();
}


/************************************************

 ************************************************/
void TrackViewDelegate::downloadingStarted(DataProvider *provider)
{
    mDownloadMovie.start();
}


/************************************************

 ************************************************/
void TrackViewDelegate::downloadingFinished(DataProvider *provider)
{
    project->emitDiskChanged(provider->disk());
}


/************************************************

 ************************************************/
QFont TrackViewDelegate::titleFont(const QFont &font) const
{
    QFont res = font;
    res.setPointSize(res.pointSize() + 1);
    res.setBold(true);
    return res;
}


/************************************************

 ************************************************/
QFont TrackViewDelegate::filesFont(const QFont &font) const
{
    QFont res = font;
    return res;
}


