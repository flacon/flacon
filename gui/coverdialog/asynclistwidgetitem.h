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


#ifndef ASYNCLISTWIDGETITEM_H
#define ASYNCLISTWIDGETITEM_H

#include <QListWidgetItem>
#include <QtConcurrent/QtConcurrent>


class AsyncListWidgetItem: public QListWidgetItem
{
public:
    explicit AsyncListWidgetItem(QListWidget *view = Q_NULLPTR, int type = Type);
    explicit AsyncListWidgetItem(const QString &text, QListWidget *view = Q_NULLPTR, int type = Type);
    AsyncListWidgetItem(const AsyncListWidgetItem &other);
    virtual ~AsyncListWidgetItem();

    void setIconAsync(const QString &fileName);

private:
    QFutureWatcher<QImage*> *mWatcher;
    void imageReady();
};


#endif // ASYNCLISTWIDGETITEM_H


