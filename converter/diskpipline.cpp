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

#include <QThread>
#include <QDebug>
#include "project.h"


class WorkerThread: public QThread
{
public:
    explicit WorkerThread(Worker *worker):
        QThread(),
        mWorker(worker)
    {
    }

    virtual ~WorkerThread(){
        delete mWorker;
    }

    void run() {
        mWorker->run();
        this->deleteLater();
    }


private:
    Worker *mWorker;
};



class DiskPipeline::Data {
public:
    Data():
        disk(nullptr),
        needStartSplitter(true),
        preGapType(Splitter::PreGapSkip),
        interrupted(false)
    {
    }

    const Disk *disk;
    ConverterEnv env;
    QList<Track*> tracks;
    bool needStartSplitter;
    Splitter::PreGapType preGapType;
    QHash<const Track*, Track::Status> trackStatuses;
    QList<WorkerRequest> encoderRequests;
    bool interrupted;

    void interrupt(Track::Status status);

};

/************************************************
 CREATE WORKERES CHAINS
 ************************************************
         SIGNAL(trackReady) --> SLOT(inputDataReady)

              +--> Encoder ---> Track gain -->+
   Splitter ->+            ...                +-> Album gain --> this
              +--> Encoder ---> Track gain -->+

                                optional step    optional step
 ************************************************/
DiskPipeline::DiskPipeline(const Disk *disk, const ConverterEnv &env, QObject *parent) :
    QObject(parent),
    mData(new Data())
{  
    mData->disk = disk;
    mData->env  = env;

    bool addPregapToFirstTrack = false;
    if (env.createCue && disk->track(0)->cueIndex(1).milliseconds() > 0)
    {
        if (env.format->preGapType() == OutFormat::PreGapExtractToFile)
        {
            mData->tracks << mData->disk->preGapTrack();
        }
        else
        {
            addPregapToFirstTrack = true;
        }
    }

    //if ()
    //for (int i=0; i<0)

    if (mData->preGapType == Splitter::PreGapExtractToFile)
        mData->trackStatuses.insert(disk->preGapTrack(), Track::NotRunning);

    for (int i=0; i<mData->disk->count(); ++i)
    {
        mData->trackStatuses.insert(mData->disk->track(i), Track::NotRunning);
    }


}


/************************************************

 ************************************************/
DiskPipeline::~DiskPipeline()
{
    delete mData;
}


/************************************************

 ************************************************/
int DiskPipeline::startWorker(int *splitterCount, int *count)
{
    if (mData->interrupted)
        return 0;

    if (*count < 0)
        return 0;

    if (*splitterCount > 0 && mData->needStartSplitter)
    {
        Splitter *splitter = new Splitter(mData->disk, mData->env);
        splitter->setPregapType(mData->preGapType);

        WorkerThread *thread = new WorkerThread(splitter);

        connect(this, SIGNAL(threadQuit()), thread, SLOT(terminate()));
        connect(splitter, SIGNAL(trackProgress(const Track*,int)),  this, SLOT(trackProgress(const Track*,int)));
        connect(splitter, SIGNAL(error(const Track*,QString)),      this, SLOT(splitterError(const Track*,QString)));
        connect(splitter, SIGNAL(trackReady(const Track*,QString)), this, SLOT(splitterTrackReady(const Track*,QString)));

        thread->start();
        mData->needStartSplitter = false;
        mData->trackStatuses.insert(mData->disk->track(0), Track::Splitting);
        --(*splitterCount);
        --(*count);
        return 1;
    }

    int res = 0;

    while (*count > 0 && !mData->encoderRequests.isEmpty())
    {
        WorkerRequest req = mData->encoderRequests.takeFirst();
        Encoder *encoder = new Encoder(req.track(), req.fileName(), mData->env);
        QThread *thread = new WorkerThread(encoder);

        connect(this, SIGNAL(threadQuit()), thread, SLOT(terminate()));
        connect(encoder, SIGNAL(trackProgress(const Track*,int)), this, SLOT(trackProgress(const Track*,int)));
        connect(encoder, SIGNAL(error(const Track*,QString)),     this, SLOT(splitterError(const Track*,QString)));
        connect(encoder, SIGNAL(trackReady(const Track*,QString)),this, SLOT(encoderTrackReady(const Track*,QString)));

        thread->start();
        --(*count);
        ++res;
    }

    return res;
}


/************************************************

 ************************************************/
void DiskPipeline::Data::interrupt(Track::Status status)
{
    interrupted = true;
    encoderRequests.clear();
/*
//TODO:
    QHashIterator<Track*, Track::Status> i(trackStatuses);
    while (i.hasNext())
    {
        i.next();
        switch (i.value())
        {
        case Track::Splitting:
        case Track::Encoding:
        case Track::Queued:
        case Track::WaitGain:
        case Track::CalcGain:
        case Track::WriteGain:
        case Track::NotRunning:
            i.value() = status;
            const_cast<Track*>(i.key())->setProgress(status);
            break;


        case Track::Canceled:
        case Track::Error:
        case Track::Aborted:
        case Track::OK:
            break;
        }
    }
    */
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
}


/************************************************

 ************************************************/
void DiskPipeline::splitterError(const Track *track, const QString &message)
{
    mData->trackStatuses.insert(track, Track::Error);
    const_cast<Track*>(track)->setProgress(Track::Error);
    mData->interrupt(Track::Aborted);

    emit threadQuit();
    emit threadFinished();
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
void DiskPipeline::trackProgress(const Track *track, int percent)
{
    if (mData->interrupted)
        return;

    if (!track)
        return;

    Track::Status status = Track::NotRunning;

    if (qobject_cast<Splitter*>(sender())) status = Track::Splitting;
    if (qobject_cast<Encoder*>(sender()))  status = Track::Encoding;
    mData->trackStatuses.insert(track, status);
    const_cast<Track*>(track)->setProgress(status, percent);
}


/************************************************

 ************************************************/
void DiskPipeline::splitterTrackReady(const Track *track, const QString &outFileName)
{
    mData->encoderRequests.append(WorkerRequest(track, outFileName));
    emit readyStart();
}


/************************************************

 ************************************************/
void DiskPipeline::encoderTrackReady(const Track *track, const QString &outFileName)
{
    mData->trackStatuses.insert(track, Track::OK);
    const_cast<Track*>(track)->setProgress(Track::OK);
    emit readyStart();
}



