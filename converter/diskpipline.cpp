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
#include "cuecreator.h"
#include "copycover.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"


#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>


/************************************************
 *
 ************************************************/
class WorkerThread: public QThread
{
public:
    explicit WorkerThread(Worker *worker, QObject *parent = nullptr):
        QThread(parent),
        mWorker(worker)
    {
        worker->moveToThread(this);
    }

    virtual ~WorkerThread()
    {
        quit();
        if (!wait(3000))
        {
            qWarning() << "Can't quit from thread" << mWorker;
            this->terminate();
            if (!wait(3000))
                qWarning() << "Can't terminate from thread" << mWorker;
        }
        mWorker->deleteLater();
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
        interrupted(false),
        preGapType(PreGapType::Skip)
    {
    }

    DiskPipeline *pipeline;
    const Disk *disk;
    QList<const Track*> tracks;
    bool needStartSplitter;
    QHash<const Track*, Track::Status> trackStatuses;
    QList<WorkerRequest> encoderRequests;
    QList<WorkerRequest> gainRequests;
    bool interrupted;
    QString workDir;
    PreGapType preGapType;


    void interrupt(Track::Status status);
    void startSplitterThread();
    void startEncoderThread(const WorkerRequest &req);
    void startTrackGainThread(const WorkerRequest &req);
    void startAlbumGainThread(QList<WorkerRequest> &reqs);
    bool createDir(const QString &dirName) const;
    bool createCue() const;
    bool copyCoverImage() const;
};



/************************************************
 *
 ************************************************/
bool DiskPipeline::Data::createDir(const QString &dirName) const
{
    QDir dir(dirName);

    if (! dir.mkpath("."))
    {
        Project::error(QObject::tr("I can't create directory \"%1\".").arg(dir.path()));
        return false;
    }

    if (!QFileInfo(dir.path()).isWritable())
    {
        Project::error(QObject::tr("I can't write to directory \"%1\".").arg(dir.path()));
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
DiskPipeline::DiskPipeline(const Disk *disk, QObject *parent) :
    QObject(parent),
    mData(new Data()),
    mTmpDir(nullptr)
{
    mData->pipeline = this;
    mData->disk = disk;
    mData->preGapType =  settings->createCue() ? settings->preGapType() : PreGapType::Skip;



}


/************************************************

 ************************************************/
DiskPipeline::~DiskPipeline()
{
    delete mData;
    delete mTmpDir;
}


/************************************************
 *
 ************************************************/
bool DiskPipeline::init()
{
    if (!settings->tmpDir().isEmpty())
    {
        if (!mData->createDir(settings->tmpDir()))
            return false;
        mTmpDir = new QTemporaryDir(QString("%1/flacon.").arg(settings->tmpDir()));
        mTmpDir->setAutoRemove(true);
        mData->workDir = mTmpDir->path();
    }
    else
        mData->workDir = QFileInfo(mData->disk->track(0)->resultFilePath()).dir().absolutePath();

    Splitter splitter(mData->disk, mData->workDir, mData->preGapType);
    mData->tracks = splitter.tracks();

    foreach (const Track *track, mData->tracks)
    {
        mData->trackStatuses.insert(track, Track::NotRunning);
    }

    if (!mData->createDir(mData->workDir))
        return false;

    foreach (const Track *track, mData->tracks)
    {
        QString dir = QFileInfo(track->resultFilePath()).absoluteDir().path();
        if (!mData->createDir(dir))
            return false;
    }

    return true;
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

    if (*count <= 0)
        return;

    if (*splitterCount > 0 && mData->needStartSplitter)
    {
        mData->startSplitterThread();
        --(*splitterCount);
        --(*count);
        return;
    }

    if (settings->outFormat()->gainType() == GainType::Track)
    {
        while (*count > 0 && !mData->gainRequests.isEmpty())
        {
            mData->startTrackGainThread(mData->gainRequests.takeFirst());
            --(*count);
        }
    }
    else if (settings->outFormat()->gainType() == GainType::Album)
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
bool DiskPipeline::Data::createCue() const
{
    if (!settings->createCue())
        return true;

    CueCreator cue(disk, preGapType);
    if (!cue.write())
    {
        pipeline->trackError(tracks.first(), cue.errorString());
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
bool DiskPipeline::Data::copyCoverImage() const
{
    CoverMode mode = settings->coverMode();

    if (mode == CoverMode::Disable)
        return true;

    int size = 0;
    if (mode == CoverMode::Scale)
        size = settings->coverImageSize();

    QString dir = QFileInfo(disk->track(0)->resultFilePath()).dir().absolutePath();

    CopyCover copyCover(disk, dir, "cover", size);
    bool res = copyCover.run();

    if (!res)
        Project::error(copyCover.errorString());

    return res;
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startSplitterThread()
{
    Splitter *worker = new Splitter(disk, workDir, preGapType);

    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(addEncoderRequest(const Track*,QString)));

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();

    needStartSplitter = false;
    trackStatuses.insert(disk->track(0), Track::Splitting);

    createCue();
    copyCoverImage();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startEncoderThread(const WorkerRequest &req)
{
    Encoder *worker = new Encoder(req, settings->outFormat());
    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker, SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    if (settings->outFormat()->gainType() == GainType::Disable)
    {
        connect(worker, SIGNAL(trackReady(const Track*,QString)),
                pipeline, SLOT(trackDone(const Track*,QString)));
    }
    else
    {
        connect(worker, SIGNAL(trackReady(const Track*,QString)),
                pipeline, SLOT(addGainRequest(const Track*,QString)));
    }

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startTrackGainThread(const WorkerRequest &req)
{
    Gain *worker = new Gain(req, settings->outFormat());
    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(trackDone(const Track*,QString)));

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startAlbumGainThread(QList<WorkerRequest> &reqs)
{
    Gain *worker = new Gain(reqs, settings->outFormat());
    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(trackDone(const Track*,QString)));

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addEncoderRequest(const Track *track, const QString &inputFile)
{
    trackProgress(track, Track::Queued, 0);
    QFileInfo trackFile(track->resultFilePath());
    QString outFile = trackFile.dir().filePath(
                QFileInfo(inputFile).baseName() +
                ".encoded." +
                trackFile.suffix());

    mData->encoderRequests << WorkerRequest(track, inputFile, outFile);
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addGainRequest(const Track *track, const QString &fileName)
{
    if (settings->outFormat()->gainType() == GainType::Album)
        trackProgress(track, Track::WaitGain, 0);
    else
        trackProgress(track, Track::Queued, 0);

    mData->gainRequests << WorkerRequest(track, fileName, fileName);
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::trackDone(const Track *track, const QString &outFileName)
{
    // Track is ready, rename the file to the final name.
    QFile::remove(track->resultFilePath());
    QFile(outFileName).rename(track->resultFilePath());


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
 *
 ************************************************/
int DiskPipeline::runningThreadCount() const
{
    int res = 0;
    foreach (WorkerThread *thread, mThreads)
    {
        if (thread->isRunning())
            ++res;
    }
    return res;
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
