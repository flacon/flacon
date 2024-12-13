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
#include "types.h"
#include "icon.h"

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>
#include <QLineEdit>

#include <QDebug>

static constexpr int SELECTION_MARK       = 4;
static constexpr int MARGIN               = 6;
static constexpr int TOP_PADDING          = 16;
static constexpr int BOTTOM_PADDING       = 2;
static constexpr int IMG_HEIGHT           = 60;
static constexpr int MARK_HEIGHT          = 32;
static constexpr int LINE_MARK_HEIGHT     = 22;
static constexpr int BUTTON_SIZE          = 10;
static constexpr int MAX_AUDIO_FILES_ROWS = 3;

struct TrackViewCacheItem
{
    QRect trackBtn;
    QRect trackLbl;

    QList<QRect> audioBtns;
    QList<QRect> audioLbls;

    QRect markBtn;
    QRect coverRect;
    bool  isWaiting     = false;
    bool  audioShowMenu = false;
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
    Keys                clickType(const QModelIndex &index, const QPoint &point);

    QList<QModelIndex> keys() const { return mItems.keys(); }

private:
    TrackViewCacheItem                       nullItemCache;
    QHash<QModelIndex, TrackViewCacheItem *> mItems;
    QModelIndex                              currentIndex;
    TrackViewCacheItem                      *currentItem;
};

/************************************************

 ************************************************/
TrackViewCache::TrackViewCache() :
    currentIndex(QModelIndex()),
    currentItem(&nullItemCache)
{
}

/************************************************

 ************************************************/
TrackViewCacheItem *TrackViewCache::item(const QModelIndex &index)
{
    if (currentIndex != index) {
        currentIndex = index;
        currentItem  = mItems.value(index, nullptr);

        if (!currentItem) {
            currentItem = new TrackViewCacheItem();
            mItems.insert(currentIndex, currentItem);
        }
    }

    return currentItem;
}

/************************************************

 ************************************************/
TrackViewDelegate::TrackViewDelegate(TrackView *parent) :
    QStyledItemDelegate(parent),
    mTrackView(parent),
    mCache(new TrackViewCache),
    mDownloadMovie(QSize(32, 32))
{
    mTrackBtnPix   = Pixmap("cue-button", BUTTON_SIZE, BUTTON_SIZE);
    mAudioBtnPix   = Pixmap("audio-button", BUTTON_SIZE, BUTTON_SIZE);
    mDiscErrorPix  = Pixmap("error", MARK_HEIGHT, MARK_HEIGHT);
    mDiscWarnPix   = Pixmap("warning", MARK_HEIGHT, MARK_HEIGHT);
    mTrackOkPix    = Pixmap("track-ok", LINE_MARK_HEIGHT, LINE_MARK_HEIGHT);
    mTrackErrorPix = Pixmap("track-cancel", LINE_MARK_HEIGHT, LINE_MARK_HEIGHT);
    mNoCoverImg    = QImage(":noCover");

    mDownloadMovie.loadFrame("wait-0");
    mDownloadMovie.loadFrame("wait-1");
    mDownloadMovie.loadFrame("wait-2");
    mDownloadMovie.loadFrame("wait-3");
    mDownloadMovie.loadFrame("wait-4");
    mDownloadMovie.loadFrame("wait-5");
    mDownloadMovie.loadFrame("wait-6");
    mDownloadMovie.loadFrame("wait-7");

    connect(&mDownloadMovie, &Movie::updated,
            this, &TrackViewDelegate::movieUpdated);
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
    QRect r = rect;
    r.setWidth(SELECTION_MARK);
#ifdef Q_OS_MAC
    QColor hi = mTrackView->palette().color(QPalette::Active, QPalette::Highlight);
    int    h, s, l;
    hi.getHsv(&h, &s, &l);
    s        = int(s * 0.6);
    QColor c = QColor::fromHsv(h, s, l);
    c.setAlphaF(0.75);
    painter->fillRect(r, c);
#else
    painter->fillRect(r, mTrackView->palette().highlight().color());
#endif
}

/************************************************

 ************************************************/
void TrackViewDelegate::drawBranch(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QColor bgColor = mTrackView->palette().base().color();
    if (rect.isValid())
        painter->fillRect(rect, bgColor);

    if (mTrackView->selectionModel()->isRowSelected(index.row(), index.parent()))
        drawSelectionMark(painter, rect);
}

/**************************************
 *
 **************************************/
QWidget *TrackViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget   *res      = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(res);

    if (lineEdit) {
        lineEdit->setPlaceholderText(index.data(TrackViewModel::RolePlaceHolder).toString());
    }
    return res;
}

/************************************************

 ************************************************/
void TrackViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;

    TrackViewModel::ItemType type = TrackViewModel::ItemType(index.data(TrackViewModel::RoleItemType).toInt());

    if (type == TrackViewModel::TrackItem) {
        if (index.row() % 2)
            opt.features &= ~QStyleOptionViewItem::Alternate;
        else
            opt.features |= QStyleOptionViewItem::Alternate;

        paintTrack(painter, opt, index);
        return;
    }

    // TrackViewModel::DiscItem
    if (index.column() == 0) {
        QColor bgColor = mTrackView->palette().base().color();
        painter->fillRect(opt.rect, bgColor);

        if (mTrackView->selectionModel()->isSelected(index)) {
            QRect rect = opt.rect;
            if (index.row() > 0)
                rect.setTop(rect.top() + TOP_PADDING);
            drawSelectionMark(painter, rect);
        }

        paintDisc(painter, opt, index);
        return;
    }
}

/************************************************

 ************************************************/
void TrackViewDelegate::paintTrack(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!(index.row() % 2))
        painter->fillRect(option.rect, QColor(128, 128, 128, 20));

    QStyledItemDelegate::paint(painter, option, index);
    if (index.column() != TrackView::ColumnPercent)
        return;

    const QPixmap *icon = nullptr;
    QString        txt;
    int            progress     = index.data(TrackViewModel::RolePercent).toInt();
    bool           showProgress = false;

    switch (TrackState(index.data(TrackViewModel::RoleStatus).toInt())) {
        case TrackState::NotRunning:
            txt = "";
            break;

        case TrackState::Canceled:
            txt = "";
            break;

        case TrackState::Error:
            txt  = tr("Error", "Status of the track conversion.");
            icon = &mTrackErrorPix;
            break;

        case TrackState::Aborted:
            txt = tr("Aborted", "Status of the track conversion.");
            break;

        case TrackState::OK:
            txt  = tr("OK", "Status of the track conversion.");
            icon = &mTrackOkPix;
            break;

        case TrackState::Splitting:
            txt          = tr("Extracting", "Status of the track conversion.");
            showProgress = true;
            break;

        case TrackState::Encoding:
            txt          = tr("Encoding", "Status of the track conversion.");
            showProgress = true;
            break;

        case TrackState::Queued:
            txt = tr("Queued", "Status of the track conversion.");
            break;

        case TrackState::CalcGain:
            txt = tr("Calculating gain", "Status of the track conversion.");
            break;

        case TrackState::WaitGain:
            txt = tr("Waiting for gain", "Status of the track conversion.");
            break;

        case TrackState::WriteGain:
            txt = tr("Writing gain", "Status of the track conversion.");
            break;
    }

    painter->save();
    painter->translate(option.rect.left() + 30, option.rect.top());
    QRect windowRect(0, 0, option.rect.width() - 31, option.rect.height());
    painter->setClipRect(windowRect);

    if (showProgress) {
        QStyleOptionProgressBar opt;
        opt.rect     = windowRect.adjusted(4, 3, -4, -3);
        opt.minimum  = 0;
        opt.maximum  = 100;
        opt.progress = progress;
        opt.text     = QStringLiteral("%1 %2%").arg(txt).arg(opt.progress);

        QApplication::style()->drawControl(QStyle::CE_ProgressBarContents, &opt, painter);
        QApplication::style()->drawControl(QStyle::CE_ProgressBarLabel, &opt, painter);
    }
    else {
        if (icon) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            int textWidth = painter->fontMetrics().horizontalAdvance(txt);
#else
            int textWidth = painter->fontMetrics().width(txt);
#endif
            int imgLeft = (windowRect.width() - LINE_MARK_HEIGHT - 4 - textWidth) / 2;
            painter->drawPixmap(imgLeft, (windowRect.height() - LINE_MARK_HEIGHT) / 2, *icon);

            QRect textRect(QPoint(imgLeft + LINE_MARK_HEIGHT + 4, 0), windowRect.bottomRight());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, txt);
        }
        else {
            painter->drawText(windowRect, Qt::AlignCenter | Qt::AlignVCenter, txt);
        }
    }

    painter->restore();
}

/************************************************

 ************************************************/
void TrackViewDelegate::paintDisc(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect paintRect = option.rect;
    paintRect.setLeft(0);

    painter->save();
    painter->setClipRect(option.rect);
    painter->translate(option.rect.topLeft());

    int   topPadding = index.row() ? TOP_PADDING : 0;
    QRect windowRect(MARGIN,
                     MARGIN + topPadding,
                     option.rect.right() - 2 * MARGIN,
                     option.rect.height() - 2 * MARGIN - topPadding - BOTTOM_PADDING);

    // Draw cover image ..............................
    QRect imgRect = drawCoverImage(painter, windowRect, index);
    windowRect.adjust(imgRect.width() + MARGIN, 0, 0, 0);

    // Draw disk title .................................
    QRect titleRect = drawTitle(painter, windowRect, index);
    windowRect.adjust(0, titleRect.height() + 8, 0, 0);

    // Draw labels ...................................
    QRect tmp        = windowRect;
    QRect tLabelRect = drawLabel(tr("Tracks:"), tmp, painter);
    tmp.adjust(0, tLabelRect.height() + 4, 0, 0);

    QRect aLabelRect = drawLabel(tr("Audio:"), tmp, painter);

    windowRect.adjust(qMax(tLabelRect.width() + MARGIN, aLabelRect.width()), 0, 0, 0);

    // Draw files ....................................
    tmp = windowRect;
    tmp.setTop(tLabelRect.top());
    tmp.setHeight(tLabelRect.height());
    QRect tFileRect = drawFile(index.data(TrackViewModel::RoleTagSetTitle).toString(), tmp, painter);
    windowRect.setTop(aLabelRect.top());

    QStringList audioFiles    = index.data(TrackViewModel::RoleAudioFileName).toStringList();
    bool        showAudioMenu = audioFiles.count() > MAX_AUDIO_FILES_ROWS;
    if (showAudioMenu) {
        audioFiles.clear();
        audioFiles << tr("Multiple files", "Disk preview, audio file placeholder");
    }

    QList<QRect> aFileRects;
    aFileRects.reserve(audioFiles.count());

    tmp = windowRect;
    tmp.setTop(aLabelRect.top());
    tmp.setHeight(aLabelRect.height());

    for (int i = 0; i < audioFiles.count(); ++i) {
        QRect r = drawFile(audioFiles[i], tmp, painter);
        aFileRects << r;
        tmp.moveTop(r.bottom());
    }

    int left = tFileRect.right();
    for (const QRect &r : aFileRects) {
        left = qMax(left, r.right());
    }

    // Draw buttons ..................................
    windowRect.setLeft(left + MARGIN);

    tmp = tFileRect;
    tmp.setLeft(windowRect.left());
    QRect tButtonRect = drawButton(mTrackBtnPix, tmp, painter);

    QList<QRect> aButtonRects;
    aFileRects.reserve(audioFiles.count());

    for (int i = 0; i < audioFiles.count(); ++i) {
        tmp = aFileRects[i];
        tmp.moveLeft(windowRect.left());
        QRect r = drawButton(showAudioMenu ? mTrackBtnPix : mAudioBtnPix, tmp, painter);
        aButtonRects << r;
        tmp.moveTop(r.bottom());
    }

    // Draw download and warning mark ................
    bool  isWaiting = index.data(TrackViewModel::RoleIsDownloads).toBool();
    QRect markRect  = drawMark(painter, isWaiting, imgRect, index);

    // Draw bottom line ................................
    painter->setPen(QColor("#7F7F7F7F"));
    int y = option.rect.height() - BOTTOM_PADDING - 2;
    painter->drawLine(MARGIN * 2, y, windowRect.right(), y);

    painter->restore();

    // Fill cache ......................................
    TrackViewCacheItem *cache = mCache->item(index);

    cache->coverRect     = imgRect;
    cache->trackBtn      = tButtonRect;
    cache->trackLbl      = tFileRect;
    cache->audioBtns     = aButtonRects;
    cache->audioLbls     = aFileRects;
    cache->audioShowMenu = showAudioMenu;
    cache->isWaiting     = isWaiting;
    cache->markBtn       = markRect;

    mDownloadMovie.setRunning(qobject_cast<TrackViewModel *>(mTrackView->model())->downloading());
}

/************************************************

************************************************/
QRect TrackViewDelegate::drawCoverImage(QPainter *painter, const QRect &windowRect, const QModelIndex &index) const
{
    QImage img = index.data(TrackViewModel::RoleCoverImg).value<QImage>();
    if (img.isNull()) {
        img = mNoCoverImg;
    }

    if (img.height() != windowRect.height()) {
        const qreal dpr = qApp->devicePixelRatio();
        img             = img.scaledToHeight(windowRect.height() * dpr, Qt::SmoothTransformation);
        img.setDevicePixelRatio(dpr);
    }

    QRect imgRect(windowRect.topLeft(), img.size() / img.devicePixelRatioF());
    painter->fillRect(imgRect, mTrackView->palette().base().color());
    painter->fillRect(imgRect, Qt::white);
    painter->drawImage(imgRect, img);
    return imgRect;
}

/************************************************

************************************************/
QRect TrackViewDelegate::drawTitle(QPainter *painter, const QRect &windowRect, const QModelIndex &index) const
{
    QFont titleFont = this->titleFont(painter->font());
    painter->save();
    painter->setFont(titleFont);
    QString album  = index.sibling(index.row(), TrackView::ColumnAlbum).data().toString();
    QString artist = index.sibling(index.row(), TrackView::ColumnArtist).data().toString();

    QString text;
    if (!album.isEmpty() || !artist.isEmpty()) {
        text = QStringLiteral("%1 / %2").arg(artist, album);
    }

    QRect res;
    painter->drawText(windowRect, Qt::AlignLeft, text, &res);
    painter->restore();
    return res;
}

/************************************************

************************************************/
QRect TrackViewDelegate::drawMark(QPainter *painter, bool isWaiting, const QRect &imgRect, const QModelIndex &index) const
{
    QRect markRect(imgRect.right() - MARK_HEIGHT, imgRect.bottom() - MARK_HEIGHT, MARK_HEIGHT, MARK_HEIGHT);

    if (isWaiting) {
        painter->drawPixmap(markRect, mDownloadMovie.currentPixmap());
        return markRect;
    }

    if (index.data(TrackViewModel::RoleHasErrors).toBool()) {
        painter->drawPixmap(markRect, mDiscErrorPix);
        return markRect;
    }

    if (index.data(TrackViewModel::RoleHasWarnings).toBool()) {
        painter->drawPixmap(markRect, mDiscWarnPix);
        return markRect;
    }

    return QRect();
}

/************************************************

************************************************/
QRect TrackViewDelegate::drawButton(const QPixmap &pixmap, const QRect &windowRect, QPainter *painter) const
{
    QRect rect = QRect(QPoint(0, 0), pixmap.size() / pixmap.devicePixelRatio());
    rect.moveCenter(windowRect.center());
    rect.moveLeft(windowRect.left());
    painter->drawPixmap(rect, pixmap);
    return rect;
}

/************************************************

 ************************************************/
QRect TrackViewDelegate::drawLabel(const QString &text, const QRect &rect, QPainter *painter) const
{
    QRect res;
    painter->save();
    painter->setPen(mTrackView->palette().dark().color());
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop, text, &res);
    painter->restore();
    return res;
}

/************************************************

 ************************************************/
QRect TrackViewDelegate::drawFile(const QString &text, const QRect &rect, QPainter *painter) const
{
    QRect res;
    if (!text.isEmpty()) {
        painter->save();
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text, &res);
        painter->restore();
    }
    else {
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

    if (!index.parent().isValid()) {
        if (!mDiscHeightHint) {
            int h = 8;

            QFont titleFont = this->titleFont(option.font);
            QFont filesFont = this->filesFont(option.font);
            h += QFontMetrics(titleFont).height();
            h += QFontMetrics(filesFont).height() * 2;
            mDiscHeightHint = qMax(IMG_HEIGHT, h) + 2 * MARGIN + BOTTOM_PADDING; // For Line
        }

        res.rheight() = mDiscHeightHint;
        if (index.row()) {
            res.rheight() += TOP_PADDING;
        }

        int n = index.data(TrackViewModel::RoleAudioFileName).toStringList().count();
        if (n > 1 && n <= MAX_AUDIO_FILES_ROWS) {
            if (!mAudioFileHeight) {
                QFont filesFont  = this->filesFont(option.font);
                mAudioFileHeight = QFontMetrics(filesFont).height();
            }

            res.rheight() += mAudioFileHeight * (n - 1);
        }

        if (index.column() == 0) {
            res.rwidth() = 600;
        }
        else {
            res.rwidth() = 0;
        }
    }
    else {
        res.rheight() = res.height() + 8;
    }

    return res;
}

/************************************************

 ************************************************/
bool TrackViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.parent().isValid()) {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
        QPoint       m  = ev->pos() - option.rect.topLeft();

        TrackViewCacheItem *cache = mCache->item(index);

        if (cache->trackLbl.contains(m)) {
            if (event->type() == QEvent::MouseButtonRelease)
                emit trackButtonClicked(index, cache->trackBtn);

            return true;
        }

        if (cache->trackBtn.contains(m)) {
            if (event->type() == QEvent::MouseButtonRelease)
                emit trackButtonClicked(index, cache->trackBtn);

            return true;
        }

        for (int i = 0; i < cache->audioBtns.count(); ++i) {
            if (cache->audioBtns[i].contains(m)) {
                if (cache->audioShowMenu) {
                    emit audioButtonClicked(index, -1, cache->audioBtns[i]);
                }
                else {
                    emit audioButtonClicked(index, i, cache->audioBtns[i]);
                }
                return true;
            }
        }

        if (cache->markBtn.contains(m)) {
            if (event->type() == QEvent::MouseButtonRelease)
                emit markClicked(index, cache->markBtn);

            return true;
        }

        if (cache->coverRect.contains(m)) {
            if (event->type() == QEvent::MouseButtonRelease)
                emit coverImageClicked(index);

            return true;
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
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

    if (cache->trackLbl.contains(m)) {
        QToolTip::showText(
                event->globalPos(),
                view->model()->data(index, TrackViewModel::RoleCueFilePath).toString(),
                view);
        return true;
    }

    if (cache->audioShowMenu) {
        QStringList toolTip;
        for (const QString &f : view->model()->data(index, TrackViewModel::RoleAudioFilePath).toStringList()) {
            toolTip << "<li>" << f << "</li>";
        }
        QToolTip::showText(event->globalPos(), toolTip.join("\n"), view);
    }
    else {
        for (int i = 0; i < cache->audioBtns.count(); ++i) {

            if (cache->audioBtns[i].contains(m) || cache->audioLbls[i].contains(m)) {
                QStringList files = view->model()->data(index, TrackViewModel::RoleAudioFilePath).toStringList();

                if (i < files.count()) {
                    QToolTip::showText(event->globalPos(), files[i], view);
                }
                return true;
            }
        }
    }

    if (cache->markBtn.contains(m)) {
        QStringList errs  = view->model()->data(index, TrackViewModel::RoleDiscErrors).toStringList();
        QStringList warns = view->model()->data(index, TrackViewModel::RoleDiscWarnings).toStringList();

        if (errs.isEmpty() && warns.isEmpty()) {
            return true;
        }

        QString html;
        if (!errs.isEmpty()) {
            html += tr("<b>The conversion is not possible.</b>");
            html += "<ul>";
            for (const QString &s : errs) {
                html += QStringLiteral("<li><nobr>%1</nobr></li>").arg(s);
            }
            html += "</ul>";
        }
        else {
            html += "<ul>";
            for (const QString &s : warns) {
                html += QStringLiteral("<li><nobr>%1</nobr></li>").arg(s);
            }
            html += "</ul>";
        }

        QToolTip::showText(
                event->globalPos(),
                "<html>" + html + "</html>",
                view);

        return true;
    }

    return false;
}

/************************************************

 ************************************************/
void TrackViewDelegate::movieUpdated()
{
    for (const QModelIndex &index : mCache->keys()) {
        if (mCache->item(index)->isWaiting) {
            emit mTrackView->model()->dataChanged(index, index);
        }
    }
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
