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


#include "diskpipline.h"
#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "project.h"

#include <QThread>
#include <QDebug>


/************************************************
 *
 ************************************************/
class WorkerThread: public QThread
{
public:
    explicit WorkerThread(Worker *worker):
        QThread(),
        mWorker(worker)
    {
        worker->moveToThread(this);
        connect(this, SIGNAL(finished()),
                this, SLOT(deleteLater()));
    }

    virtual ~WorkerThread(){
        delete mWorker;
    }

    void run()
    {
        mWorker->run();
    }


private:
    Worker *mWorker;
};


/************************************************
 *
 ************************************************/
class DiskPipeline::Data
{
public:
    Data():
        pipeline(nullptr),
        disk(nullptr),
        needStartSplitter(true),
        interrupted(false)
    {
    }

    DiskPipeline *pipeline;
    const Disk *disk;
    ConverterEnv env;
    QList<const Track*> tracks;
    bool needStartSplitter;
    QHash<const Track*, Track::Status> trackStatuses;
    QList<WorkerRequest> encoderRequests;
    QList<WorkerRequest> gainRequests;
    bool interrupted;

    void interrupt(Track::Status status);
    Splitter::PreGapType preGapType();
    void startSplitterThread();
    void startEncoderThread(const WorkerRequest &req);
    void startTrackGainThread(const WorkerRequest &req);
    void startAlbumGainThread(QList<WorkerRequest> &reqs);

};


/************************************************
 *
 ************************************************/
Splitter::PreGapType DiskPipeline::Data::preGapType()
{
    if (env.createCue && disk->track(0)->cueIndex(1).milliseconds() > 0)
    {
        if (env.format->preGapType() == OutFormat::PreGapExtractToFile)
            return Splitter::PreGapExtractToFile;
        else
            return Splitter::PreGapAddToFirstTrack;
    }

    return Splitter::PreGapSkip;
}



/************************************************
 *
 ************************************************/
DiskPipeline::DiskPipeline(const Disk *disk, const ConverterEnv &env, QObject *parent) :
    QObject(parent),
    mData(new Data())
{  
    mData->pipeline = this;
    mData->disk = disk;
    mData->env  = env;

    Splitter splitter(mData->disk, mData->env);
    splitter.setPregapType(mData->preGapType());
    mData->tracks = splitter.tracks();

    foreach (const Track *track, mData->tracks)
    {
        mData->trackStatuses.insert(track, Track::NotRunning);
    }
}


/************************************************

 ************************************************/
DiskPipeline::~DiskPipeline()
{
    delete mData;
}


/************************************************
 CREATE WORKER CHAINS
 ************************************************
              +--> Encoder ---> Track gain -->+
   Splitter ->+            ...                +-> Album gain --> this
              +--> Encoder ---> Track gain -->+

                                optional step    optional step

 ************************************************/
void DiskPipeline::startWorker(int *splitterCount, int *count)
{
    if (mData->interrupted)
        return;

    if (*count < 0)
        return;

    if (*splitterCount > 0 && mData->needStartSplitter)
    {
        mData->startSplitterThread();
        --(*splitterCount);
        --(*count);
        return;
    }

    if (mData->env.format->gainType() == OutFormat::GainTrack)
    {
        while (*count > 0 && !mData->gainRequests.isEmpty())
        {
            mData->startTrackGainThread(mData->gainRequests.takeFirst());
            --(*count);
        }
    }
    else if (mData->env.format->gainType() == OutFormat::GainAlbum)
    {
        if (*count > 0 && mData->gainRequests.count() == mData->tracks.count())
        {
            mData->startAlbumGainThread(mData->gainRequests);
            mData->gainRequests.clear();
            --(*count);
        }
    }

    while (*count > 0 && !mData->encoderRequests.isEmpty())
    {
        mData->startEncoderThread(mData->encoderRequests.takeFirst());
        --(*count);
    }
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startSplitterThread()
{
    Splitter *worker = new Splitter(disk, env);
    worker->setPregapType(preGapType());

    WorkerThread *thread = new WorkerThread(worker);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(addEncoderRequest(const Track*,QString)));

    thread->start();
    needStartSplitter = false;
    trackStatuses.insert(disk->track(0), Track::Splitting);
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startEncoderThread(const WorkerRequest &req)
{
    Encoder *worker = new Encoder(req, env);
    QThread *thread = new WorkerThread(worker);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker, SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    if (env.format->gainType() == OutFormat::GainDisable)
    {
        connect(worker, SIGNAL(trackReady(const Track*,QString)),
                pipeline, SLOT(trackDone(const Track*)));
    }
    else
    {
        connect(worker, SIGNAL(trackReady(const Track*,QString)),
                pipeline, SLOT(addGainRequest(const Track*,QString)));
    }

    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startTrackGainThread(const WorkerRequest &req)
{
    Gain *worker = new Gain(req, env);
    QThread *thread = new WorkerThread(worker);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(trackDone(const Track*)));

    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startAlbumGainThread(QList<WorkerRequest> &reqs)
{
    Gain *worker = new Gain(reqs, env);
    QThread *thread = new WorkerThread(worker);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(trackDone(const Track*)));

    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addEncoderRequest(const Track *track, const QString &fileName)
{
    mData->encoderRequests << WorkerRequest(track, fileName);
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addGainRequest(const Track *track, const QString &fileName)
{
    if (mData->env.format->gainType() == OutFormat::GainAlbum)
    {
        const_cast<Track*>(track)->setProgress(Track::WaitGain);
    }
    mData->gainRequests << WorkerRequest(track, fileName);
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::trackDone(const Track *track)
{
    mData->trackStatuses.insert(track, Track::OK);
    const_cast<Track*>(track)->setProgress(Track::OK);

    emit threadFinished();

    if (!isRunning())
        emit finished();
}



/************************************************

 ************************************************/
void DiskPipeline::Data::interrupt(Track::Status status)
{
    interrupted = true;
    encoderRequests.clear();

    QHash<const Track*, Track::Status>::iterator it;
    for (it = trackStatuses.begin(); it != trackStatuses.end(); ++it)
    {
        switch (it.value())
        {
        case Track::Splitting:
        case Track::Encoding:
        case Track::Queued:
        case Track::WaitGain:
        case Track::CalcGain:
        case Track::WriteGain:
        case Track::NotRunning:
            it.value() = status;
            const_cast<Track*>(it.key())->setProgress(status);
            break;


        case Track::Canceled:
        case Track::Error:
        case Track::Aborted:
        case Track::OK:
            break;
        }
    }
}


/************************************************

 ************************************************/
void DiskPipeline::stop()
{
    mData->interrupt(Track::Aborted);
    emit threadQuit();
    emit threadFinished();

    emit finished();
}


/************************************************

 ************************************************/
void DiskPipeline::trackError(const Track *track, const QString &message)
{
    mData->trackStatuses.insert(track, Track::Error);
    const_cast<Track*>(track)->setProgress(Track::Error);
    mData->interrupt(Track::Aborted);
    emit threadQuit();
    emit threadFinished();

    emit finished();
    Project::error(message);
}


/************************************************

 ************************************************/
bool DiskPipeline::isRunning() const
{
    QHash<const Track*, Track::Status>::const_iterator it;
    for (it = mData->trackStatuses.begin(); it != mData->trackStatuses.end(); ++it)
    {
        switch (it.value())
        {
        case Track::Splitting:
        case Track::Encoding:
        case Track::Queued:
        case Track::WaitGain:
        case Track::CalcGain:
        case Track::WriteGain:
            return true;

        case Track::NotRunning:
        case Track::Canceled:
        case Track::Error:
        case Track::Aborted:
        case Track::OK:
            break;
        }
    }

    return false;
}


/************************************************

 ************************************************/
void DiskPipeline::trackProgress(const Track *track, Track::Status status, int percent)
{
    if (mData->interrupted)
        return;

    if (!track)
        return;

    mData->trackStatuses.insert(track, status);
    const_cast<Track*>(track)->setProgress(status, percent);
}

