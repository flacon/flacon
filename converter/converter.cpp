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
#include "project.h"
#include "settings.h"
#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "diskpipline.h"
#include "resampler.h"

#include <iostream>
#include <math.h>
#include <QFileInfo>
#include <QDir>


/************************************************

 ************************************************/
Converter::Converter(QObject *parent) :
    QObject(parent),
    mThreadCount(0)
{
}


/************************************************
 *
 ************************************************/
Converter::~Converter()
{
}


/************************************************

 ************************************************/
void Converter::start(const Profile &profile)
{
    Jobs jobs;
    for (int d=0; d<project->count(); ++d)
    {
        Job job;
        job.disk = project->disk(d);

        for (int t=0; t<job.disk->count(); ++t)
            job.tracks << job.disk->track(t);

        jobs << job;
    }

    start(jobs, profile);
}


/************************************************
 *
 ************************************************/
void Converter::start(const Converter::Jobs &jobs, const Profile &profile)
{
    if (jobs.isEmpty())
    {
        emit finished();
        return;
    }

    if (!check(profile))
    {
        emit finished();
        return;
    }

    bool ok;
    mThreadCount = Settings::i()->value(Settings::Encoder_ThreadCount).toInt(&ok);
    if (!ok || mThreadCount < 1)
        mThreadCount = qMax(6, QThread::idealThreadCount());

    for (const Job &job: jobs)
    {
        if (job.tracks.isEmpty())
            continue;

        if (!job.disk->canConvert())
            continue;

        DiskPipeline * pipeline = new DiskPipeline(job, profile, this);

        connect(pipeline, SIGNAL(readyStart()),
                this, SLOT(startThread()));

        connect(pipeline, SIGNAL(threadFinished()),
                this, SLOT(startThread()));

        connect(pipeline, &DiskPipeline::trackProgressChanged,
                this, &Converter::trackProgress);

        mDiskPiplines << pipeline;

        if (!pipeline->init())
        {
            qDeleteAll(mDiskPiplines);
            mDiskPiplines.clear();
            emit finished();
            return;
        }
    }

    startThread();
    emit started();
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
bool Converter::canConvert()
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

    foreach (DiskPipeline *pipe, mDiskPiplines)
        count-=pipe->runningThreadCount();

    foreach (DiskPipeline *pipe, mDiskPiplines)
    {
        pipe->startWorker(&splitterCount, &count);
        if (count <= 0)
            break;
    }


    foreach (DiskPipeline *pipe, mDiskPiplines)
    {
        if (pipe->isRunning())
            return;
    }

    emit finished();
}


/************************************************

 ************************************************/
bool Converter::check(const Profile &profile) const
{
    QStringList errors;
    bool ok = profile.check(&errors);

    if (Settings::i()->value(Settings::Resample_BitsPerSample).toInt() ||
        Settings::i()->value(Settings::Resample_SampleRate).toInt() )
    {
        if (!Settings::i()->checkProgram(Resampler::programName()))
        {
            errors << QObject::tr("I can't find program <b>%1</b>.").arg(Resampler::programName());
            ok = false;
        }
    }

    if (!ok)
    {
        QString s;
        foreach (QString e, errors)
        {
            s += QString("<li style='margin-top: 4px;'> %1</li>").arg(e);
        }

        Messages::error(QString("<html>%1<ul>%2</ul></html>")
                      .arg(tr("Conversion is not possible:"), s));
    }

    return ok;
}
