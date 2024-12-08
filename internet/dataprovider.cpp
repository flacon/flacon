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
#include "profiles.h"

#include "musicbrainz.h"
#include "discogs.h"
#include <QNetworkProxy>

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

static QNetworkProxy::ProxyType proxyTypeToQProxyType(ProxyType type)
{
    // clang-format off
    switch (type) {
        case ProxyType::NoProxy:     return QNetworkProxy::ProxyType::NoProxy;
        case ProxyType::HttpProxy:   return QNetworkProxy::ProxyType::HttpProxy;
        case ProxyType::Socks5Proxy: return QNetworkProxy::ProxyType::Socks5Proxy;
    }
    // clang-format on

    return QNetworkProxy::ProxyType::NoProxy;
}

/************************************************

 ************************************************/
DataProvider::DataProvider(Profile *profile, QObject *parent) :
    QObject(parent)
{

    QNetworkProxy proxy;
    proxy.setType(proxyTypeToQProxyType(profile->proxyType()));
    proxy.setHostName(profile->proxyHost());
    proxy.setPort(profile->proxyPort());
    proxy.setUser(profile->proxyUserName());
    proxy.setPassword(profile->proxyPassword());
    QNetworkProxy::setApplicationProxy(proxy);
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
    req.setRawHeader("User-Agent", QStringLiteral("Flacon/%1 (https://github.com/flacon/flacon)").arg(FLACON_VERSION).toUtf8());
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

    qCDebug(LOG) << "(" << objectName() << ") Request error" << message;
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
            if (compareTags(mResult[i], mResult[j])) {
                mResult.removeAt(i);
                break;
            }
        }
    }
}

/**************************************

 **************************************/
bool InterntService::compareTags(const InternetTags &tags1, const InternetTags &tags2) const
{
    bool res = true;

    res = res && tags1.album() == tags2.album();
    res = res && tags1.albumPerformer() == tags2.albumPerformer();

    res = res && tags1.tracks().count() == tags2.tracks().count();

    int n = -1;
    for (const Tags::Track &t : tags1.tracks()) {
        n++;
        res = res && compareTrackTags(t, tags2.tracks().at(n));
    }

    return res;
}

/**************************************

 **************************************/
bool InterntService::compareTrackTags(const Tags::Track &tags1, const Tags::Track &tags2) const
{
    bool res = true;

    res = res && tags1.title() == tags2.title();
    res = res && tags1.trackNum() == tags2.trackNum();

    return res;
}
