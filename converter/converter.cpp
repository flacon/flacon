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
#include "diskpipline.h"
#include "converterenv.h"

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
    OutFormat *format = OutFormat::currentFormat();
    if (!check(format))
    {
        emit finished();
        return;
    }

    mStartTime = QDateTime::currentDateTime();

    bool ok;
    mThreadCount = settings->value(Settings::Encoder_ThreadCount).toInt(&ok);
    if (!ok || mThreadCount < 1)
        mThreadCount = qMax(6, QThread::idealThreadCount());

    QString tmpDir = settings->value(Settings::Encoder_TmpDir).toString();
    for (int i=0; i<project->count(); ++i)
    {
        ConverterEnv env;
        env.format      = format;
        env.tmpDir      = tmpDir;
        env.createCue   = format->createCue();

        DiskPipeline * pipeline = new DiskPipeline(project->disk(i), env, this);

        connect(pipeline, SIGNAL(readyStart()),
                this, SLOT(startThread()));

        connect(pipeline, SIGNAL(threadFinished()),
                this, SLOT(startThread()));

        mDiskPiplines << pipeline;

    }

    if (!createDirs())
    {
        qDeleteAll(mDiskPiplines);
        mDiskPiplines.clear();
        emit finished();
        return;
    }


    startThread();
}




/************************************************

 ************************************************/
bool Converter::createDirs()
{
    bool res = true;
    QSet<QString> dirs;
    //foreach(ConverterThread *thread, mThreads)
    //{
    //    if (!thread->workDir().isEmpty())
    //        dirs << thread->workDir();
    //}

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
    foreach (DiskPipeline *pipe, mDiskPiplines)
    {
        if (pipe->isRunning())
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

    foreach (DiskPipeline *pipe, mDiskPiplines)
    {
        pipe->stop();
    }
}


/************************************************

 ************************************************/
void Converter::startThread()
{
    int count = mThreadCount;
    int splitterCount = qMax(1.0, ceil(count / 2.0));

    int running = 0;
    foreach (DiskPipeline *pipe, mDiskPiplines)
    {
        running += pipe->startWorker(&splitterCount, &count);
        if (count <= 0)
            break;
    }

    if (running == 0)
        emit finished();
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
