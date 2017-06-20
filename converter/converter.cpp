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


#include <QThread>
#include <QDebug>

#include "converter.h"
#include "converterthread.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"
#include "splitter.h"
#include "encoder.h"
#include "gain.h"

#include <iostream>
#include <math.h>
#include <QFileInfo>
#include <QDir>

/************************************************

 ************************************************/
Converter::Converter(QObject *parent) :
    QObject(parent),
    mThreadCount(0),
    mShowStatistic(true)
{
}


/************************************************

 ************************************************/
void Converter::start()
{
    mStartTime = QDateTime::currentDateTime();

    bool ok;
    mThreadCount = settings->value(Settings::Encoder_ThreadCount).toInt(&ok);
    if (!ok || mThreadCount < 1)
        mThreadCount = qMax(6, QThread::idealThreadCount());


    OutFormat *format = OutFormat::currentFormat();
    //Gain::GainType gainType = OutputFormat::currentGainType();


    if (!check(format))
    {
        emit finished();
        return;
    }

    for (int i=0; i<project->count(); ++i)
    {
        Disk *disk = project->disk(i);
        if (disk->canConvert())
            createDiscThreads(disk, format);
    }


    foreach(ConverterThread* thread, mThreads)
    {
        connect(thread, SIGNAL(trackError(Track*,QString)), this, SLOT(threadError(Track*,QString)));
        connect(thread, SIGNAL(readyStart()), this, SLOT(startThread()));
        connect(thread, SIGNAL(finished()), this, SLOT(threadFinished()));
        connect(thread, SIGNAL(trackProgress(Track*,Track::Status,int)), this, SLOT(setTrackProgress(Track*,Track::Status,int)));
    }

    if (!createDirs())
    {
        qDeleteAll(mThreads);
        mThreads.clear();
        emit finished();
        return;
    }

    if (mThreads.isEmpty())
    {
        qWarning() << "No job for converter";
        emit finished();
        return;
    }

    startThread();
}


/************************************************
 CREATE THREADS CHAINS
 ************************************************
         SIGNAL(trackReady) --> SLOT(inputDataReady)

              +--> Encoder ---> Track gain -->+
   Splitter ->+            ...                +-> Album gain --> this
              +--> Encoder ---> Track gain -->+

                                optional step    optional step
 ************************************************/
void Converter::createDiscThreads(Disk *disk, const OutFormat *format)
{
    Splitter *splitter = new Splitter(disk, format);
    mThreads << splitter;

    Gain *albumGain = 0;
    if (format->gainType() == OutFormat::GainAlbum)
    {
        albumGain = format->createGain(disk, 0);
    }

    if (albumGain)
    {
        mThreads << albumGain;
        connect(albumGain, SIGNAL(trackReady(Track*,QString)), this, SLOT(trackReady(Track*,QString)));
    }


    createTrackThreads(disk->preGapTrack(), format, splitter, albumGain);

    for (int i=0; i<disk->count(); ++i)
    {
        createTrackThreads(disk->track(i), format, splitter, albumGain);
    }
}


/************************************************

 ************************************************/
void Converter::createTrackThreads(Track *track, const OutFormat *format, ConverterThread *prevThread, ConverterThread *nextThread)
{
    Encoder *encoder = format->createEncoder(track);
    mThreads << encoder;
    connect(prevThread, SIGNAL(trackReady(Track*,QString)), encoder, SLOT(inputDataReady(Track*,QString)));
    ConverterThread *last = encoder;

    Gain *gain = 0;
    if (format->gainType() == OutFormat::GainTrack)
        gain = format->createGain(track->disk(), track);

    if (gain)
    {
        last = gain;
        mThreads << gain;
        connect(encoder, SIGNAL(trackReady(Track*,QString)), gain, SLOT(inputDataReady(Track*,QString)));
    }


    if (nextThread)
        connect(last, SIGNAL(trackReady(Track*,QString)), nextThread, SLOT(inputDataReady(Track*,QString)));
    else
        connect(last, SIGNAL(trackReady(Track*,QString)), this, SLOT(trackReady(Track*,QString)));
}


/************************************************

 ************************************************/
bool Converter::createDirs()
{
    bool res = true;
    QSet<QString> dirs;
    foreach(ConverterThread *thread, mThreads)
    {
        if (!thread->workDir().isEmpty())
            dirs << thread->workDir();
    }

    foreach(QString dirName, dirs)
    {
        QDir dir(dirName);

        if (! dir.mkpath("."))
        {
            Project::error(tr("I can't create directory \"%1\".").arg(dir.path()));
            res = false;
            continue;
        }

        if (!QFileInfo(dir.path()).isWritable())
        {
            Project::error(tr("I can't write to directory \"%1\".").arg(dir.path()));
            res = false;
            continue;
        }
    }

    return res;
}



/************************************************

 ************************************************/
bool Converter::isRunning()
{
    foreach (QThread *thread, mThreads)
    {
        if (thread->isRunning())
            return true;
    }

    return false;
}


/************************************************

 ************************************************/
bool Converter::canConvert() const
{
    for(int i=0; i<project->count(); ++i)
    {
        if (project->disk(i)->canConvert())
            return true;
    }

    return false;
}


/************************************************

 ************************************************/
void Converter::setShowStatistic(bool value)
{
    mShowStatistic = value;
}


/************************************************

 ************************************************/
void Converter::stop()
{
    if (!isRunning())
        return;

    foreach(ConverterThread *thread, mThreads)
        thread->stop();
}


/************************************************

 ************************************************/
void Converter::threadError(Track *track, const QString &message)
{
    foreach(ConverterThread *thread, mThreads)
    {
        if (thread->disk() == track->disk())
            thread->stop();
    }

    Project::error(message);
}


/************************************************

 ************************************************/
void Converter::startThread()
{
    int count = mThreadCount;

    int splitterCount = qMax(1.0, ceil(count / 2.0));

    foreach(ConverterThread *thread, mThreads)
    {
        if (count == 0)
            break;

        if (thread->isFinished())
            continue;

        Splitter *splitter = qobject_cast<Splitter*>(thread);
        if (splitter)
        {
            if (thread->isRunning())
            {
                count--;
                splitterCount--;
            }
            else
            {
                if (thread->isReadyStart() and splitterCount > 0)
                {
                    thread->start();
                    count--;
                    splitterCount--;
                }
            }
        }

        else
        {
            if (thread->isRunning())
            {
                count--;
            }
            else
            {
                if (thread->isReadyStart())
                {
                    thread->start();
                    count--;
                }
            }
        }

    }
}


/************************************************

 ************************************************/
void Converter::threadFinished()
{
    ConverterThread *thread = qobject_cast<ConverterThread*>(sender());
    if (!thread)
        return;

    mThreads.removeAll(thread);
    thread->deleteLater();

    startThread();

    if (!isRunning())
    {
        emit finished();

        if (mShowStatistic)
        {
            int duration = QDateTime::currentDateTime().toTime_t() - mStartTime.toTime_t();
            if (!duration)
                duration = 1;

            int h = duration / 3600;
            int m = (duration - (h * 3600)) / 60;
            int s =  duration - (h * 3600) - (m * 60);

            QString str;

            if (h)
                str = QString("Encoding time %4h %3m %2s [%1 sec]").arg(duration).arg(s).arg(m).arg(h);
            else if (m)
                str = QString("Encoding time %3m %2s [%1 sec]").arg(duration).arg(s).arg(m);
            else
                str = QString("Encoding time %1 sec").arg(duration);

            std::cout << str.toLocal8Bit().constData() << std::endl;
        }

        qDeleteAll(mThreads);
        mThreads.clear();
    }
}


/************************************************

 ************************************************/
void Converter::trackReady(Track *track, const QString &outFileName)
{
    setTrackProgress(track, Track::OK, -1);
}


/************************************************

 ************************************************/
void Converter::setTrackProgress(Track *track, Track::Status status, int percent)
{
    if (track)
        track->setProgress(status, percent);
}




/************************************************

 ************************************************/
bool Converter::check(OutFormat *format) const
{
    QStringList errors;
    bool ok = format->check(&errors);

    if (!ok)
    {
        QString s;
        foreach (QString e, errors)
        {
            s += QString("<li style='margin-top: 4px;'> %1</li>").arg(e);
        }

        Project::error(QString("<html>%1<ul>%2</ul></html>")
                      .arg(tr("Conversion is not possible:"), s));
    }

    return ok;
}
