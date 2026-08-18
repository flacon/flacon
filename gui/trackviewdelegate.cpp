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
static constexpr int MARGIN               = 12;
static constexpr int PADDING              = 4;
static constexpr int TOP_PADDING          = 16;
static constexpr int BOTTOM_PADDING       = 2;
static constexpr int IMG_HEIGHT           = 80;
static constexpr int MARK_HEIGHT          = 32;
static constexpr int LINE_MARK_HEIGHT     = 22;
static constexpr int BUTTON_SIZE          = 16;
static constexpr int MAX_AUDIO_FILES_ROWS = 2;

struct TrackViewCacheItem
{
    QRect trackBtn;
    QRect trackLbl;

    QRect audioBtn;
    QRect audioLbl;

    QRect markBtn;
    QRect coverRect;
    bool  isWaiting = false;
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
    mBtnPix        = Pixmap("pattern-button", BUTTON_SIZE, BUTTON_SIZE);
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
QColor TrackViewDelegate::selectionColor() const
{
#ifdef Q_OS_MAC
    QColor hi = mTrackView->palette().color(QPalette::Active, QPalette::Highlight);
    int    h, s, l;
    hi.getHsv(&h, &s, &l);
    s        = int(s * 0.6);
    QColor c = QColor::fromHsv(h, s, l);
    return c;
#else
    return mTrackView->palette().highlight().color();
#endif
}

/************************************************

 ************************************************/
void TrackViewDelegate::drawSelectionMark(QPainter *painter, const QRect &rect) const
{
    QRect r = rect;
    r.setWidth(SELECTION_MARK);
    painter->fillRect(r, selectionColor());
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
    if (index.column() != TrackView::ColumnTracknum)
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
    QStringList audioFiles      = index.data(TrackViewModel::RoleAudioFileName).toStringList();
    int         audioLinesCount = audioFiles.count() > MAX_AUDIO_FILES_ROWS ? 1 : audioFiles.count();

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

    // Draw badge ....................................
    bool  hasCue    = !index.data(TrackViewModel::RoleCueFilePath).toString().isEmpty();
    QRect badgeRect = titleRect;
    badgeRect.moveLeft(badgeRect.right() + 8);
    drawBadge(hasCue, painter, badgeRect);

    // Draw labels ...................................
    QFontMetrics aLabelFm   = painter->fontMetrics();
    QRect        aLabelRect = windowRect;
    aLabelRect.setTop(aLabelRect.bottom() - aLabelFm.lineSpacing() * audioLinesCount);
    aLabelRect = drawLabel(tr("Audio:"), aLabelRect, painter);

    QRect tLabelRect = windowRect;
    tLabelRect.setTop(aLabelRect.bottom() - aLabelRect.height() - aLabelFm.lineSpacing() - 5);
    tLabelRect = drawLabel(tr("Tags:", "Disk item in disk table"), tLabelRect, painter);
    windowRect.adjust(qMax(tLabelRect.width(), aLabelRect.width()) + PADDING, 0, 0, 0);

    // Draw files ....................................
    QRect tmp = windowRect;
    tmp.setTop(tLabelRect.top());
    tmp.setHeight(tLabelRect.height());
    QRect tFileRect = drawFile(index.data(TrackViewModel::RoleTagSetTitle).toString(), tmp, painter);
    windowRect.setTop(aLabelRect.top());

    if (audioFiles.count() > MAX_AUDIO_FILES_ROWS) {
        audioFiles.clear();
        audioFiles << tr("Multiple files", "Disk preview, audio file placeholder");
    }

    QRect aFilesRect = QRect(windowRect);
    aFilesRect       = drawFile(audioFiles.join("\n"), aFilesRect, painter);

    // Draw buttons ..................................
    QRect tButtonRect(
            std::max(aFilesRect.right(), tFileRect.right()) + PADDING,
            tFileRect.top(),
            tFileRect.height(),
            tFileRect.height());
    tButtonRect = drawButton(mBtnPix, tButtonRect, painter);

    QRect aButtonRect = tButtonRect;
    aButtonRect.moveTop(aFilesRect.top());
    aButtonRect = drawButton(mBtnPix, aButtonRect, painter);

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

    cache->coverRect = imgRect;
    cache->trackBtn  = tButtonRect;
    cache->trackLbl  = tFileRect;
    cache->audioBtn  = aButtonRect;
    cache->audioLbl  = aFilesRect;
    cache->isWaiting = isWaiting;
    cache->markBtn   = markRect;

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
void TrackViewDelegate::drawBadge(bool hasCue, QPainter *painter, const QRect &rect) const
{
    if (!rect.isValid() || rect.isEmpty()) {
        return;
    }

    if (hasCue) {
        return;
    }

    QString text = tr("TRACKS", "Tree view bage text");

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font = painter->font();
    font.setPointSize(qMax(6, font.pointSize() - 2));
    font.setBold(true);
    painter->setFont(font);

    QFontMetrics fm(font);
    int          paddingH = 8;
    int          paddingV = 2;

    int width  = fm.horizontalAdvance(text) + (paddingH * 2);
    int height = fm.height() + (paddingV * 2);

    QRectF badgeRect(0, 0, width, height);
    badgeRect.moveCenter(rect.center());
    badgeRect.moveLeft(rect.left());
    badgeRect.adjust(0, 1, 0, 1);

    QColor bgColor = selectionColor();

    QPen pen(bgColor, 1.5);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(bgColor);

    qreal radius = badgeRect.height() / 2.0;
    painter->drawRoundedRect(badgeRect, radius, radius);

    QColor textColor = Qt::white;
    painter->setPen(textColor);
    painter->drawText(badgeRect, Qt::AlignCenter, text);

    painter->restore();
}

/************************************************

 ************************************************/
QSize TrackViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize res = QStyledItemDelegate::sizeHint(option, index);

    if (!index.parent().isValid()) {

        if (!mDiscHeightHint) {
            int   h         = 8;
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

        if (cache->trackLbl.contains(m) || cache->trackBtn.contains(m)) {
            emit trackButtonClicked(index, cache->trackBtn);
            return true;
        }

        if (cache->audioLbl.contains(m) || cache->audioBtn.contains(m)) {
            emit audioButtonClicked(index, cache->audioBtn);
            return true;
        }

        if (cache->markBtn.contains(m)) {
            emit markClicked(index, cache->markBtn);
            return true;
        }

        if (cache->coverRect.contains(m)) {
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
    if (index.parent().isValid()) {
        return QStyledItemDelegate::helpEvent(event, view, option, index);
    }

    TrackViewCacheItem *cache = mCache->item(index);
    if (cache == nullptr) {
        return true;
    }

    QPoint m = event->pos() - option.rect.topLeft();

    if (cache->trackLbl.contains(m)) {
        QToolTip::showText(
                event->globalPos(),
                view->model()->data(index, TrackViewModel::RoleCueFilePath).toString(),
                view);
        return true;
    }

    if (cache->audioLbl.contains(m)) {
        QString toolTip = view->model()->data(index, TrackViewModel::RoleAudioFilePath).toStringList().join("\n");
        QToolTip::showText(event->globalPos(), toolTip, view);
        return true;
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
