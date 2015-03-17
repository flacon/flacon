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

#include "../disk.h"
#include <QtNetwork/QNetworkAccessManager>

//--------
#include "settings.h"
#include "project.h"


#include <QStringList>

#include <QDebug>
#include <QTextCodec>

/************************************************

 ************************************************/
DataProvider::DataProvider(Disk *disk) :
    QObject(disk),
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
    addReply(reply);
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
    project->error(message);
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
            dataReady(reply);
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
void DataProvider::addReply(QNetworkReply *reply)
{
    mReplies << reply;
    connect(reply, SIGNAL(finished()), this, SLOT(replayFinished()));
}


/************************************************

 ************************************************/
FreeDbProvider::FreeDbProvider(Disk *disk):
    DataProvider(disk)
{
}


/************************************************

 ************************************************/
void FreeDbProvider::start()
{
    QString host= settings->value(Settings::Inet_CDDBHost).toString();

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
                           disk()->discId(),
                           "anonimous",     // Hello user
                           "127.0.0.1",     // Hello host
                           "flacon",        //Hello client name
                           FLACON_VERSION   // Hello client version
                           ));
        request.setAttribute(QNetworkRequest::User, category);
        get(request);
    }
    project->emitDownloadingStarted(this);
}


/************************************************

 ************************************************/
void FreeDbProvider::dataReady(QNetworkReply *reply)
{
    QString statusLine = reply->readLine();
    int status = statusLine.section(' ', 0, 0).toInt();

    // CDDB errors .....................................
    switch(status)
    {
    case 210:   // OK
        parse(reply);
        break;

    case 401:   // No such CD entry in database, skip.
        break;

    default:    // Error
        error(statusLine);
    }


    project->emitDownloadingFinished(this);

    if (!isFinished())
        return;

    if (mResult.count() == 0)
        return;


    int minDist = disk()->distance(mResult.at(0));
    int bestTagSet = 0;

    for (int i=1; i<mResult.count(); ++i)
    {
        int n = disk()->distance(mResult.at(i));
        if (n<minDist)
        {
            minDist = n;
            bestTagSet = i;
        }
    }

    for (int i=0; i<mResult.count(); ++i)
    {
        disk()->addTagSet(mResult.at(i), i == bestTagSet);
    }

}


/************************************************

 ************************************************/
void FreeDbProvider::parse(QNetworkReply *reply)
{
    QByteArray category = reply->request().attribute(QNetworkRequest::User).toByteArray();

    TagSet res(reply->url().toString());
    res.setDiskTag(TAG_DISCID, disk()->discId());

    QByteArray artist;
    QByteArray album;
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
            res.setDiskTag(TAG_DATE, value, false);
            continue;
        }

        if (key == "DGENRE")
        {
            res.setDiskTag(TAG_GENRE, value, false);
            continue;
        }

        if (key == "DTITLE")
        {
            // The artist and disc title (in that order) separated by a "/" with a
            // single space on either side to separate it from the text.
            artist = leftPart(value, '/').trimmed();
            res.setDiskTag(TAG_PERFORMER, artist, false);

            album = rightPart(value, '/').trimmed();
            res.setDiskTag(TAG_ALBUM, album, false);
            continue;
        }

        if (key.startsWith("TTITLE"))
        {
            tracks << line;
            continue;
        }

    }

    foreach (QByteArray line, tracks)
    {
        QByteArray key = leftPart(line, '=').toUpper();
        QByteArray value = rightPart(line, '=').trimmed();

        bool ok;
        int n = key.mid(6).toInt(&ok);
        if(!ok)
        {
            error("I can't parse CDDB result.");
            return;
        }


        if (value.contains('/'))
        {
            // If the disc is a sampler and there are different artists for the
            // track titles, the track artist and the track title (in that order)
            // should be separated by a "/" with a single space on either side
            // to separate it from the text.
            res.setTrackTag(n, TAG_PERFORMER, leftPart(value, '/').trimmed(), false);
            res.setTrackTag(n, TAG_TITLE, rightPart(value, '/').trimmed(), false);
        }
        else
        {
            res.setTrackTag(n, TAG_PERFORMER, artist, false);
            res.setTrackTag(n, TAG_TITLE, value, false);
        }

    }

    res.setTitle(category + " / " + artist + " [CDDB " + album + "]", false);

    res.setTextCodecName(disk()->textCodecName());
    mResult << res;
}
