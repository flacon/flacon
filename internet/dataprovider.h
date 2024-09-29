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

#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>
#include <QList>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "tags.h"

class Disc;
class QNetworkAccessManager;
class InterntService;

class DataProvider : public QObject
{
    Q_OBJECT
public:
    static bool canDownload(const Disc &disk);

public:
    explicit DataProvider(QObject *parent = nullptr);
    virtual ~DataProvider();

    void start(const Disc &disk);
    void stop();

    bool isFinished() const;

signals:
    void finished(const QVector<InternetTags> &result);
    void errorOccurred(const QString &err);

private:
    QList<InterntService *> mServices;
    QVector<InternetTags>   mResult;

    void serviceFinished(const QVector<InternetTags> &result);
};

class InterntService : public QObject
{
    Q_OBJECT
public:
    explicit InterntService(const Disc &disk, QObject *parent = nullptr);

    bool isFinished() const;

    virtual void start() = 0;
    virtual void stop();

signals:
    void finished(const QVector<InternetTags> result);
    void errorOccurred(const QString &err);

protected:
    const Disc            &mDisk;
    QVector<InternetTags>  mResult;
    QList<QNetworkReply *> mReplies;

    QNetworkAccessManager *networkAccessManager() const;
    virtual QNetworkReply *get(const QNetworkRequest &request);

    void error(const QString &message);
    void removeDuplicates();
};

#endif // DATAPROVIDER_H
