/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "asynclistwidgetitem.h"
#include <QtConcurrent/QtConcurrent>
#include <QListWidgetItem>
#include <QImage>
#include <QPainter>

#include <QImageReader>

/************************************************
 *
 ************************************************/
QImage *loadImage(const QString &fileName, const QSize &size)
{
    QImage img = QImage(fileName);
    if (img.isNull())
        return nullptr;

    QSize imgSize = img.size();
    QImage *res = new QImage(size, QImage::Format_ARGB32);
    res->fill(Qt::transparent);

    QPainter painter(res);

    img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QRectF rect = img.rect();
    rect.moveCenter(res->rect().center());
    painter.drawImage(rect, img);

    //....................................
    QString sizeStr =  QString("%1x%2").arg(imgSize.width()).arg(imgSize.height());

    QFont font = painter.font();
    font.setPointSize(font.pointSize() * 0.8);
    font.setBold(true);
    painter.setFont(font);

    QPen pen = painter.pen();
    pen.setColor(QColor(255, 255, 255, 255));
    painter.setPen(pen);

    rect = painter.fontMetrics().tightBoundingRect(sizeStr);
    rect.adjust(-5, -4, 5, 4);
    rect.moveBottomRight(res->rect().bottomRight());

    painter.fillRect(rect.adjusted(1,1,-1,-1), QColor(0, 0, 0, 128));
    painter.drawText(rect, Qt::AlignCenter, sizeStr);

    pen.setColor(QColor(255, 255, 255, 128));
    painter.setPen(pen);
    painter.drawRoundedRect(rect, 2, 2);

    return res;
}


/************************************************
 *
 ************************************************/
AsyncListWidgetItem::AsyncListWidgetItem(QListWidget *view, int type):
    QListWidgetItem(view, type),
    mWatcher(nullptr)
{

}


/************************************************
 *
 ************************************************/
AsyncListWidgetItem::AsyncListWidgetItem(const QString &text, QListWidget *view, int type):
    QListWidgetItem(text, view, type),
    mWatcher(nullptr)
{

}


/************************************************
 *
 ************************************************/
AsyncListWidgetItem::~AsyncListWidgetItem()
{
    if (mWatcher)
    {
        QObject::disconnect(mWatcher, nullptr, nullptr, nullptr);
        mWatcher->deleteLater();
    }
}


/************************************************
 *
 ************************************************/
void AsyncListWidgetItem::setIconAsync(const QString &fileName)
{
    if (mWatcher)
    {
        QObject::disconnect(mWatcher, nullptr, nullptr, nullptr);
        mWatcher->deleteLater();
    }

    mWatcher = new QFutureWatcher<QImage*>(nullptr);

    mWatcher->connect(mWatcher, &QFutureWatcher<QImage*>::finished,
                     [this](){this->imageReady();});

    mWatcher->setFuture(QtConcurrent::run(
                            loadImage,
                            fileName,
                            listWidget()->iconSize()));
}


/************************************************
 *
 ************************************************/
void AsyncListWidgetItem::imageReady()
{
    QImage *img = mWatcher->future().result();
    if (img)
        setIcon(QPixmap::fromImage(*img));

    delete img;
}
