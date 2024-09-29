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

#include "dataprovider.h"

#include "../disc.h"
#include <QtNetwork/QNetworkAccessManager>

#include <QStringList>
#include <QDebug>
#include <QLoggingCategory>
#include <functional>

#include "musicbrainz.h"
#include "discogs.h"

namespace {
Q_LOGGING_CATEGORY(LOG, "DataProvider")
}

/************************************************

 ************************************************/
bool DataProvider::canDownload(const Disc &disk)
{
    bool res = false;

    res = res || MusicBrainz::canDownload(disk);
    res = res || Discogs::canDownload(disk);

    return res;
}

/************************************************

 ************************************************/
DataProvider::DataProvider(QObject *parent) :
    QObject(parent)
{
}

/************************************************

 ************************************************/
DataProvider::~DataProvider()
{
    qDeleteAll(mServices);
}

/************************************************

 ************************************************/
void DataProvider::start(const Disc &disk)
{
    mServices << new MusicBrainz(disk, this);
    mServices << new Discogs(disk, this);

    for (auto service : mServices) {
        connect(service, &InterntService::finished, this, &DataProvider::serviceFinished);
        connect(service, &InterntService::errorOccurred, this, &DataProvider::errorOccurred);
        service->start();
    }
}

/************************************************

 ************************************************/
void DataProvider::stop()
{
    for (auto service : mServices) {
        service->stop();
    }
}

/************************************************

 ************************************************/
bool DataProvider::isFinished() const
{
    for (auto service : mServices) {
        if (!service->isFinished()) {
            return false;
        }
    }

    return true;
}

/************************************************

 ************************************************/
void DataProvider::serviceFinished(const QVector<InternetTags> &result)
{
    mResult << result;

    if (isFinished()) {
        emit finished(mResult);
    }
}

/************************************************

 ************************************************/
InterntService::InterntService(const Disc &disk, QObject *parent) :
    QObject(parent),
    mDisk(disk)
{
}

/************************************************

 ************************************************/
bool InterntService::isFinished() const
{
    foreach (const QNetworkReply *reply, mReplies) {
        if (!reply->isFinished())
            return false;
    }

    return true;
}

/************************************************

 ************************************************/
void InterntService::stop()
{
}

/************************************************

 ************************************************/
QNetworkReply *InterntService::get(const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    req.setRawHeader("User-Agent", QString("Flacon/%1 (https://github.com/flacon/flacon)").arg(FLACON_VERSION).toUtf8());
    qCDebug(LOG).noquote() << req.url().toEncoded();

    QNetworkReply *reply = networkAccessManager()->get(req);
    mReplies << reply;
    return reply;
}

/************************************************

 ************************************************/
void InterntService::error(const QString &message)
{
    foreach (QNetworkReply *reply, mReplies) {
        if (reply->isOpen()) {
            reply->abort();
        }
    }

    emit errorOccurred(message);
}

/************************************************

 ************************************************/
QNetworkAccessManager *InterntService::networkAccessManager() const
{
    static QNetworkAccessManager *inst = new QNetworkAccessManager();
    return inst;
}

/************************************************

 ************************************************/
void InterntService::removeDuplicates()
{
    for (int i = mResult.size() - 1; i >= 0; --i) {
        for (int j = 0; j < i; ++j) {
            if (mResult[i].compareTags(mResult[j])) {
                mResult.removeAt(i);
                break;
            }
        }
    }
}
