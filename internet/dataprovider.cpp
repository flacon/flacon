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

/************************************************

 ************************************************/
DataProvider::DataProvider(const Disc &disk) :
    QObject(),
    mDisk(disk)
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
    foreach(QNetworkReply *reply, mReplies)
    {
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
void DataProvider::get(const QNetworkRequest &request)
{
    QNetworkReply *reply = networkAccessManager()->get(request);
    mReplies << reply;
    connect(reply, SIGNAL(finished()), this, SLOT(replayFinished()));
}


/************************************************

 ************************************************/
void DataProvider::error(const QString &message)
{
    foreach(QNetworkReply *reply, mReplies)
    {
        if (reply->isOpen())
            reply->abort();
    }
    Messages::error(message);
}


/************************************************

 ************************************************/
void DataProvider::replayFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply)
    {
        switch (reply->error())
        {
        case QNetworkReply::NoError:
            mResult << dataReady(reply);
            if (isFinished())
                emit ready(mResult);

            break;

        case QNetworkReply::OperationCanceledError:
            break;

        default:
            error(reply->errorString());
        }

        mReplies.removeAll(reply);
        reply->deleteLater();
    }

    if (!mReplies.count())
        emit finished();
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
FreeDbProvider::FreeDbProvider(const Disc &disk):
    DataProvider(disk)
{
}


/************************************************

 ************************************************/
void FreeDbProvider::start()
{
    QString host= Settings::i()->value(Settings::Inet_CDDBHost).toString();

    // Categories from http://freedb.freedb.org/~cddb/cddb.cgi?cmd=CDDB+LSCAT&hello=n+h+c+1&proto=6
    QStringList categories;
    categories << "folk";
    categories << "jazz";
    categories << "misc";
    categories << "rock";
    categories << "country";
    categories << "blues";
    categories << "newage";
    categories << "reggae";
    categories << "classical";
    categories << "soundtrack";

    QString mask = "http://%1/~cddb/cddb.cgi?cmd=CDDB+READ+%2+%3&hello=%4+%5+%6+%7&proto=5";

    foreach (QString category, categories)
    {
        QNetworkRequest request;
        request.setUrl(mask.arg(
                           host,
                           category,
                           disk().discId(),
                           "anonimous",     // Hello user
                           "127.0.0.1",     // Hello host
                           "flacon",        //Hello client name
                           FLACON_VERSION   // Hello client version
                           ));
        request.setAttribute(QNetworkRequest::User, category);
        get(request);
    }
}


/************************************************

 ************************************************/
QVector<Tracks> FreeDbProvider::dataReady(QNetworkReply *reply)
{
    QString statusLine = reply->readLine();
    int status = statusLine.section(' ', 0, 0).toInt();

    // CDDB errors .....................................
    switch(status)
    {
    case 210:   // OK
        return QVector<Tracks>() << parse(reply);
        break;

    case 401:   // No such CD entry in database, skip.
        break;

    default:    // Error
        error(statusLine);
    }

    return QVector<Tracks>();
}


/************************************************

 ************************************************/
Tracks FreeDbProvider::parse(QNetworkReply *reply)
{
    QByteArray category = reply->request().attribute(QNetworkRequest::User).toByteArray();

    Tracks res;
    res.setUri(reply->url().toString());

    QByteArray album;
    QByteArray year;
    QByteArray genre;
    QByteArray performer;
    QList<QByteArray> tracks;

    while (!reply->atEnd())
    {
        QByteArray line = reply->readLine();

        if (line.length() == 0 || line.startsWith('#'))
            continue;

        QByteArray key = leftPart(line, '=').toUpper();
        QByteArray value = rightPart(line, '=').trimmed();

        if (key == "DYEAR")
        {
            year = value;
            continue;
        }

        if (key == "DGENRE")
        {
            genre = value;
            continue;
        }

        if (key == "DTITLE")
        {
            // The artist and disc title (in that order) separated by a "/" with a
            // single space on either side to separate it from the text.
            performer = leftPart(value, '/').trimmed();
            album     = rightPart(value, '/').trimmed();
            continue;
        }

        if (key.startsWith("TTITLE"))
        {
            tracks << line;
            continue;
        }

    }

    int n = 0;
    res.resize(tracks.count());
    foreach (QByteArray line, tracks)
    {
        Track &track = res[n++];
        track.setCodecName(disk().codecName());
        track.setTag(TagId::DiscId, disk().discId());
        track.setTag(TagId::Date, year);
        track.setTag(TagId::Genre, genre);
        track.setTag(TagId::Album, album);

        QByteArray value = rightPart(line, '=').trimmed();

        if (value.contains('/'))
        {
            // If the disc is a sampler and there are different artists for the
            // track titles, the track artist and the track title (in that order)
            // should be separated by a "/" with a single space on either side
            // to separate it from the text.
            track.setTag(TagId::Artist, leftPart(value,  '/').trimmed());
            track.setTag(TagId::Title,  rightPart(value, '/').trimmed());
        }
        else
        {
            track.setTag(TagId::Artist, performer.trimmed());
            track.setTag(TagId::Title,  value);
        }

    }

    res.setTitle(category + " / " + performer + " [CDDB " + album + "]");
    return res;
}
