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

#include "settings.h"
#include "project.h"

#include <QStringList>
#include <QDebug>
#include <QTextCodec>
#include <QLoggingCategory>
#include <functional>

namespace {
Q_LOGGING_CATEGORY(LOG, "DataProvider")
}

/************************************************

 ************************************************/
DataProvider::DataProvider(const Disc &disc) :
    QObject(),
    mDisc(disc)
{
}

/************************************************

 ************************************************/
DataProvider::~DataProvider()
{
}

/************************************************

 ************************************************/
bool DataProvider::isFinished() const
{
    foreach (QNetworkReply *reply, mReplies) {
        if (!reply->isFinished())
            return false;
    }

    return true;
}

/************************************************

 ************************************************/
void DataProvider::stop()
{
}

/************************************************

 ************************************************/
QNetworkReply *DataProvider::get(const QNetworkRequest &request)
{
    QNetworkReply *reply = networkAccessManager()->get(request);
    mReplies << reply;
    return reply;
}

/************************************************

 ************************************************/
void DataProvider::error(const QString &message)
{
    foreach (QNetworkReply *reply, mReplies) {
        if (reply->isOpen())
            reply->abort();
    }
    Messages::error(message);
}

/************************************************

 ************************************************/
QNetworkAccessManager *DataProvider::networkAccessManager() const
{
    static QNetworkAccessManager *inst = new QNetworkAccessManager();
    return inst;
}

/************************************************

 ************************************************/
void DataProvider::removeDduplicates()
{
    for (int i = mResult.size() - 1; i >= 0; --i) {
        for (int j = 0; j < i; ++j) {
            if (mResult[i] == mResult[j]) {
                mResult.remove(i);
                break;
            }
        }
    }
}
