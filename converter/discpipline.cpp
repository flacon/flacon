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


#include "discpipline.h"

#include <QUuid>

#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "cuecreator.h"
#include "copycover.h"
#include "project.h"
#include "inputaudiofile.h"
#include "profiles.h"

#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <errno.h>


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


struct GainRequest
{
    const Track *track;
    QString inputFile;
};

struct EncoderRequest
{
    const Track *track;
    QString inputFile;
    QString outFile;
};

/************************************************
 *
 ************************************************/
class DiscPipeline::Data
{
public:
    Data(const Converter::Job &job, const Profile &profile, DiscPipeline *pipeline):
        pipeline(pipeline),
        job(job),
        profile(profile)
    {
    }

    ~Data()
    {
        delete workDir;
    }

    DiscPipeline *pipeline;
    Converter::Job job;
    const Profile profile;
    bool needStartSplitter = true;
    QHash<const Track*, TrackState> trackStates;
    QList<EncoderRequest> encoderRequests;
    QList<GainRequest> gainRequests;
    bool interrupted = false;
    PreGapType preGapType = PreGapType::Skip;
    bool extractPregap = false;
    int trackCount = 0;
    CoverMode coverMode = CoverMode::Disable;
    int coverImageSize = 0;
    QString tmpDirName;
    QTemporaryDir *workDir = nullptr;
    QVector<WorkerThread*> threads;

    bool init();
    void interrupt(TrackState state);
    void startSplitterThread();
    void startEncoderThread(const EncoderRequest &req);
    void startTrackGainThread(const GainRequest &req);
    void startAlbumGainThread(QList<GainRequest> &reqs);
    bool createDir(const QString &dirName) const;
    bool createCue() const;
    bool copyCoverImage() const;
};



/************************************************
 *
 ************************************************/
bool DiscPipeline::Data::createDir(const QString &dirName) const
{
    QDir dir(dirName);

    if (! dir.mkpath("."))
    {
        QString msg = QObject::tr("I can't create directory \"%1\".").arg(dir.path());
        QString err = strerror(errno);
        Messages::error(msg + "<br><br>" + err);
        return false;
    }

    if (!QFileInfo(dir.path()).isWritable())
    {
        QString msg = QObject::tr("I can't write to directory \"%1\".").arg(dir.path());
        QString err = strerror(errno);
        Messages::error(msg + "<br><br>" + err);
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
DiscPipeline::DiscPipeline(const Converter::Job &job, const Profile &profile, QObject *parent) :
    QObject(parent),
    mData(new Data(job, profile, this))
{
    mData->preGapType = mData->profile.isCreateCue() ? mData->profile.preGapType() : PreGapType::Skip;

    // If the first track starts with zero second, doesn't make sense to create pregap track.
    mData->extractPregap = (mData->preGapType == PreGapType::ExtractToFile &&
                            job.disc->track(0)->cueIndex(1).milliseconds() > 0);

    mData->trackCount = mData->job.tracks.count();
    if (mData->extractPregap)
        mData->trackCount += 1;
}


/************************************************

 ************************************************/
DiscPipeline::~DiscPipeline()
{
    delete mData;
}


/************************************************
 *
 ************************************************/
bool DiscPipeline::init()
{
    return mData->init();
}


/************************************************
 *
 ************************************************/
bool DiscPipeline::Data::init()
{
    QString dir = !tmpDirName.isEmpty() ?
                   tmpDirName :
                   QFileInfo(job.tracks.first()->resultFilePath()).dir().absolutePath();

    qDebug() << "Create tmp dir" << dir;

    if (!createDir(dir)) {
        qWarning() << "Can't create tmp dir" << dir;
        return false;
    }

    workDir = new QTemporaryDir(QString("%1/flacon").arg(dir));
    workDir->setAutoRemove(true);

    foreach (const Track *track, job.tracks) {
        trackStates.insert(track, TrackState::NotRunning);

        if (!createDir(QFileInfo(track->resultFilePath()).absoluteDir().path())) {
            qWarning() << "Can't create out dir" << QFileInfo(track->resultFilePath()).absoluteDir().path();
            return false;
        }
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
void DiscPipeline::startWorker(int *splitterCount, int *count)
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

    if (mData->profile.gainType() == GainType::Track)
    {
        while (*count > 0 && !mData->gainRequests.isEmpty())
        {
            mData->startTrackGainThread(mData->gainRequests.takeFirst());
            --(*count);
        }
    }
    else if (mData->profile.gainType() == GainType::Album)
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
bool DiscPipeline::Data::createCue() const
{
    if (!profile.isCreateCue())
        return true;

    CueCreator cue(job.disc, preGapType, profile.cueFileName());
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
bool DiscPipeline::Data::copyCoverImage() const
{
    if (coverMode == CoverMode::Disable)
        return true;

    int size = 0;
    if (coverMode == CoverMode::Scale)
        size = coverImageSize;

    QString dir = QFileInfo(job.tracks.first()->resultFilePath()).dir().absolutePath();

    CopyCover copyCover(job.disc, dir, "cover", size);
    bool res = copyCover.run();

    if (!res)
        Messages::error(copyCover.errorString());

    return res;
}


/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startSplitterThread()
{
    Splitter *worker = new Splitter(job.disc, workDir->path(), extractPregap, preGapType);
    for (const Track *t: job.tracks)
        worker->addTrack(t);

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

    threads << thread;
    thread->start();

    needStartSplitter = false;
    trackStates.insert(job.tracks.first(), TrackState::Splitting);

    createCue();
    copyCoverImage();
}


/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startEncoderThread(const EncoderRequest &req)
{
    Encoder *worker = new Encoder(req.track, req.inputFile, req.outFile, profile);

    // If the original quality is worse than requested, leave it as is.
    worker->setBitsPerSample(calcQuality(
                                 job.disc->audioFile()->bitsPerSample(),
                                 profile.bitsPerSample(),
                                 int(profile.maxBitPerSample())));

    worker->setSampleRate(calcQuality(
                              job.disc->audioFile()->sampleRate(),
                              profile.sampleRate(),
                              int(profile.maxSampleRate())));


    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,TrackState,int)),
            pipeline, SLOT(trackProgress(const Track*,TrackState,int)));

    connect(worker, SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    if (profile.gainType() == GainType::Disable)
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

    threads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startTrackGainThread(const GainRequest &req)
{
    Gain *worker = new Gain(profile);
    worker->addTrack(req.track, req.inputFile);

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

    threads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startAlbumGainThread(QList<GainRequest> &reqs)
{
    Gain *worker = new Gain(profile);
    for (const GainRequest &req: reqs) {
        worker->addTrack(req.track, req.inputFile);
    }

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

    threads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
int DiscPipeline::calcQuality(int input, int preferences, int formatMax)
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
void DiscPipeline::addEncoderRequest(const Track *track, const QString &inputFile)
{
    trackProgress(track, TrackState::Queued, 0);
    QFileInfo trackFile(track->resultFilePath());
    QString outFile = mData->workDir->filePath(
                QFileInfo(inputFile).baseName() +
                ".encoded." +
                trackFile.suffix());

    mData->encoderRequests.append({track, inputFile, outFile});
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiscPipeline::addGainRequest(const Track *track, const QString &fileName)
{
    if (mData->profile.gainType() == GainType::Album)
        trackProgress(track, TrackState::WaitGain, 0);
    else
        trackProgress(track, TrackState::Queued, 0);

    mData->gainRequests.append({track, fileName});
    emit readyStart();
}


/************************************************
 *
 ************************************************/
void DiscPipeline::trackDone(const Track *track, const QString &outFileName)
{
    qDebug() << "Track done: "
             << *track
             << "outFileName:" << outFileName;

    // Track is ready, rename the file to the final name.
    // Remove old already existing file.
    QFile::remove(track->resultFilePath());

    QFile file(outFileName);
    if (! file.rename(track->resultFilePath())) {
        trackError(track, tr("I can't rename file:\n%1 to %2\n%3")
                   .arg(outFileName)
                   .arg(track->resultFilePath())
                   .arg(file.errorString()));
    }

    mData->trackStates.insert(track, TrackState::OK);
    emit trackProgressChanged(*track, TrackState::OK, 0);
    emit threadFinished();

    qDebug() << "finished:" << (!isRunning());

    if (!isRunning())
        emit finished();
}


/************************************************

 ************************************************/
void DiscPipeline::Data::interrupt(TrackState state)
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
void DiscPipeline::stop()
{
    mData->interrupt(TrackState::Aborted);
    emit threadQuit();
    emit threadFinished();

    emit finished();
}


/************************************************

 ************************************************/
void DiscPipeline::trackError(const Track *track, const QString &message)
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
bool DiscPipeline::isRunning() const
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
int DiscPipeline::runningThreadCount() const
{
    int res = 0;
    foreach (WorkerThread *thread, mData->threads)
    {
        if (thread->isRunning())
            ++res;
    }
    return res;
}


/************************************************
 *
 ************************************************/
CoverMode DiscPipeline::coverMode() const
{
    return mData->coverMode;
}


/************************************************
 *
 ************************************************/
void DiscPipeline::setCoverMode(CoverMode value)
{
    mData->coverMode = value;
}


/************************************************
 *
 ************************************************/
int DiscPipeline::coverImageSize() const
{
    return mData->coverImageSize;
}


/************************************************
 *
 ************************************************/
void DiscPipeline::setCoverImageSize(int value)
{
    mData->coverImageSize = value;
}


/************************************************
 *
 ************************************************/
QString DiscPipeline::tmpDir() const
{
    return mData->tmpDirName;
}


/************************************************
 *
 ************************************************/
void DiscPipeline::setTmpDir(QString value)
{
    mData->tmpDirName = value;
}


/************************************************

 ************************************************/
void DiscPipeline::trackProgress(const Track *track, TrackState state, int percent)
{
    if (mData->interrupted)
        return;

    if (!track)
        return;

    mData->trackStates.insert(track, state);
    emit trackProgressChanged(*track, state, percent);
}
