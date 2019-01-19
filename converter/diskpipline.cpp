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

#include <QUuid>

#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "cuecreator.h"
#include "copycover.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"
#include "inputaudiofile.h"

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
        needStartSplitter(true),
        interrupted(false),
        preGapType(PreGapType::Skip),
        extractPregap(false)
    {
    }

    DiskPipeline *pipeline;
    Converter::Job job;
    bool needStartSplitter;
    QHash<const Track*, TrackState> trackStates;
    QList<EncoderRequest> encoderRequests;
    QList<WorkerRequest> gainRequests;
    bool interrupted;
    QString workDir;
    QString tmpFilePrefix;
    PreGapType preGapType;
    bool extractPregap;
    int trackCount;

    void interrupt(TrackState state);
    void startSplitterThread();
    void startEncoderThread(const EncoderRequest &req);
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
        Messages::error(QObject::tr("I can't create directory \"%1\".").arg(dir.path()));
        return false;
    }

    if (!QFileInfo(dir.path()).isWritable())
    {
        Messages::error(QObject::tr("I can't write to directory \"%1\".").arg(dir.path()));
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
DiskPipeline::DiskPipeline(const Converter::Job &job, QObject *parent) :
    QObject(parent),
    mData(new Data()),
    mTmpDir(nullptr)
{
    mData->pipeline = this;
    mData->job = job;
    mData->preGapType =  settings->createCue() ? settings->preGapType() : PreGapType::Skip;

    // If the first track starts with zero second, doesn't make sense to create pregap track.
    mData->extractPregap = (mData->preGapType == PreGapType::ExtractToFile &&
                            job.disk->track(0)->cueIndex(1).milliseconds() > 0);

    mData->trackCount = mData->job.tracks.count();
    if (mData->extractPregap)
        mData->trackCount += 1;
}


/************************************************

 ************************************************/
DiskPipeline::~DiskPipeline()
{
    // Remove temporary files
    QDir dir(QFileInfo(mData->tmpFilePrefix).dir());
    QStringList filters;
    filters << QFileInfo(mData->tmpFilePrefix).fileName() + "*";

    foreach (const QString &file, dir.entryList(filters, QDir::Files))
        dir.remove(file);

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
        mData->workDir = QFileInfo(mData->job.tracks.first()->resultFilePath()).dir().absolutePath();

    mData->tmpFilePrefix = QDir::toNativeSeparators(QString("%1/flacon_%2-")
                                                .arg(mData->workDir)
                                                .arg(QUuid::createUuid().toString().mid(1, 36)));

    foreach (const Track *track, mData->job.tracks)
    {
        mData->trackStates.insert(track, TrackState::NotRunning);
    }

    if (!mData->createDir(mData->workDir))
        return false;

    foreach (const Track *track, mData->job.tracks)
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
        if (*count > 0 && mData->gainRequests.count() == mData->trackCount)
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

    CueCreator cue(job.disk, preGapType);
    if (!cue.write())
    {
        pipeline->trackError(job.tracks.first(), cue.errorString());
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

    QString dir = QFileInfo(job.tracks.first()->resultFilePath()).dir().absolutePath();

    CopyCover copyCover(job.disk, dir, "cover", size);
    bool res = copyCover.run();

    if (!res)
        Messages::error(copyCover.errorString());

    return res;
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startSplitterThread()
{
    Splitter *worker = new Splitter(job, tmpFilePrefix, extractPregap, preGapType);

    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,TrackState,int)),
            pipeline, SLOT(trackProgress(const Track*,TrackState,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(addEncoderRequest(const Track*,QString)));

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();

    needStartSplitter = false;
    trackStates.insert(job.tracks.first(), TrackState::Splitting);

    createCue();
    copyCoverImage();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::Data::startEncoderThread(const EncoderRequest &req)
{
    Encoder *worker = new Encoder(req);
    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,TrackState,int)),
            pipeline, SLOT(trackProgress(const Track*,TrackState,int)));

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

    connect(worker,   SIGNAL(trackProgress(const Track*,TrackState,int)),
            pipeline, SLOT(trackProgress(const Track*,TrackState,int)));

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

    connect(worker,   SIGNAL(trackProgress(const Track*,TrackState,int)),
            pipeline, SLOT(trackProgress(const Track*,TrackState,int)));

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
int DiskPipeline::calcQuality(int input, int preferences, int formatMax)
{

    int min = qMin(qMin(input, formatMax),
                   (preferences ? preferences : std::numeric_limits<int>::max()));

    if (min < input)
        return min;
    else
        return 0;
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addEncoderRequest(const Track *track, const QString &inputFile)
{
    trackProgress(track, TrackState::Queued, 0);
    QFileInfo trackFile(track->resultFilePath());
    QString outFile = trackFile.dir().filePath(
                QFileInfo(inputFile).baseName() +
                ".encoded." +
                trackFile.suffix());

    EncoderRequest req;
    req.track      = track;
    req.inputFile  = inputFile;
    req.outFile    = outFile;
    req.format     = settings->outFormat();

    // If the original quality is worse than requested, leave it as is.
    req.bitsPerSample = calcQuality(mData->job.disk->audioFile()->bitsPerSample(),
                                    settings->value(Settings::Resample_BitsPerSample).toInt(),
                                    int(req.format->maxBitPerSample()));

    req.sampleRate = calcQuality(mData->job.disk->audioFile()->sampleRate(),
                                 settings->value(Settings::Resample_SampleRate).toInt(),
                                 int(req.format->maxSampleRate()));

    mData->encoderRequests << req;
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiskPipeline::addGainRequest(const Track *track, const QString &fileName)
{
    if (settings->outFormat()->gainType() == GainType::Album)
        trackProgress(track, TrackState::WaitGain, 0);
    else
        trackProgress(track, TrackState::Queued, 0);

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


    mData->trackStates.insert(track, TrackState::OK);
    emit trackProgressChanged(*track, TrackState::OK, 0);
    emit threadFinished();

    if (!isRunning())
        emit finished();
}


/************************************************

 ************************************************/
void DiskPipeline::Data::interrupt(TrackState state)
{
    interrupted = true;
    encoderRequests.clear();

    QHash<const Track*, TrackState>::iterator it;
    for (it = trackStates.begin(); it != trackStates.end(); ++it)
    {
        switch (it.value())
        {
        case TrackState::Splitting:
        case TrackState::Encoding:
        case TrackState::Queued:
        case TrackState::WaitGain:
        case TrackState::CalcGain:
        case TrackState::WriteGain:
        case TrackState::NotRunning:
            it.value() = state;
            emit pipeline->trackProgressChanged(*(it.key()), state, 0);
            break;


        case TrackState::Canceled:
        case TrackState::Error:
        case TrackState::Aborted:
        case TrackState::OK:
            break;
        }
    }
}


/************************************************

 ************************************************/
void DiskPipeline::stop()
{
    mData->interrupt(TrackState::Aborted);
    emit threadQuit();
    emit threadFinished();

    emit finished();
}


/************************************************

 ************************************************/
void DiskPipeline::trackError(const Track *track, const QString &message)
{
    mData->trackStates.insert(track, TrackState::Error);
    emit trackProgressChanged(*track, TrackState::Error, 0);
    mData->interrupt(TrackState::Aborted);
    emit threadQuit();
    emit threadFinished();

    emit finished();
    Messages::error(message);
}


/************************************************

 ************************************************/
bool DiskPipeline::isRunning() const
{
    QHash<const Track*, TrackState>::const_iterator it;
    for (it = mData->trackStates.begin(); it != mData->trackStates.end(); ++it)
    {
        switch (it.value())
        {
        case TrackState::Splitting:
        case TrackState::Encoding:
        case TrackState::Queued:
        case TrackState::WaitGain:
        case TrackState::CalcGain:
        case TrackState::WriteGain:
            return true;

        case TrackState::NotRunning:
        case TrackState::Canceled:
        case TrackState::Error:
        case TrackState::Aborted:
        case TrackState::OK:
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
void DiskPipeline::trackProgress(const Track *track, TrackState state, int percent)
{
    if (mData->interrupted)
        return;

    if (!track)
        return;

    mData->trackStates.insert(track, state);
    emit trackProgressChanged(*track, state, percent);
}
