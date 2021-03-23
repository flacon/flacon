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
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "Converter")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
class Conv::WorkerThread : public QThread
{
public:
    explicit WorkerThread(Worker *worker, QObject *parent = nullptr) :
        QThread(parent),
        mWorker(worker)
    {
        worker->moveToThread(this);
    }

    virtual ~WorkerThread()
    {
        quit();
        if (!wait(3000)) {
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
class DiscPipeline::Data
{
public:
    Data(const DiscPipelineJob &job, DiscPipeline *pipeline) :
        mPipeline(pipeline),
        mJob(job)
    {
    }

    ~Data()
    {
        delete mTmpDir;
    }

    DiscPipeline *           mPipeline;
    DiscPipelineJob          mJob;
    QMap<TrackId, ConvTrack> mTracks;
    QList<SplitterJob>       mSplitterRequests;
    QList<EncoderJob>        mEncoderRequests;
    GainJobs                 mGainRequests;
    bool                     mInterrupted = false;
    QTemporaryDir *          mTmpDir      = nullptr;
    QVector<WorkerThread *>  mThreads;

    void interrupt(TrackState state);

    void startEncoderThread(const EncoderJob &req);
    void startTrackGainThread(const GainJob &req);
    void startAlbumGainThread(const GainJobs &reqs);
    bool createDir(const QString &dirName) const;
    bool copyCoverImage() const;

    void addSpliterRequest(const InputAudioFile &audio);
    void startSplitterThread(const SplitterJob &req);
};

/************************************************
 *
 ************************************************/
bool DiscPipeline::Data::createDir(const QString &dirName) const
{
    QDir dir(dirName);

    if (!dir.mkpath(".")) {
        QString msg = QObject::tr("I can't create directory \"%1\".").arg(dir.path());
        QString err = strerror(errno);
        Messages::error(msg + "<br><br>" + err);
        return false;
    }

    if (!QFileInfo(dir.path()).isWritable()) {
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
void DiscPipeline::Data::addSpliterRequest(const InputAudioFile &audio)
{
    SplitterJob request;
    request.inFile = audio.filePath();
    request.outDir = mTmpDir->path();

    for (const ConvTrack &t : mJob.tracks) {
        if (t.isEnabled() && t.audioFile().filePath() == audio.filePath()) {
            request.tracks << t;
        }
    }

    mSplitterRequests << request;
}

/************************************************
 *
 ************************************************/
DiscPipeline::DiscPipeline(const DiscPipelineJob &job, QObject *parent) :
    QObject(parent),
    mData(new Data(job, this))
{
    QString dir = QFileInfo(mData->mJob.workDir).dir().absolutePath();
    qCDebug(LOG) << "Create tmp dir" << dir;

    if (!mData->createDir(dir)) {
        throw FlaconError(tr("Can't create tmp dir %1").arg(dir));
    }

    mData->mTmpDir = new QTemporaryDir(QString("%1/tmp").arg(dir));
    mData->mTmpDir->setAutoRemove(true);

    for (const ConvTrack &track : mData->mJob.tracks) {
        if (!track.isEnabled()) {
            continue;
        }

        mData->mTracks[track.id()] = track;

        if (!mData->createDir(QFileInfo(track.resultFilePath()).absoluteDir().path())) {
            throw FlaconError(tr("Can't create output dir %1").arg(QFileInfo(track.resultFilePath()).absoluteDir().path()));
        }
    }

    // A disk can contain several audio files,
    // so we create several splitter requests.
    QString prev;
    for (const ConvTrack &t : mData->mJob.tracks) {
        if (!t.isEnabled()) {
            continue;
        }

        if (t.audioFile().filePath() != prev) {
            mData->addSpliterRequest(t.audioFile());
            prev = t.audioFile().filePath();
        }
    }
}

/************************************************

 ************************************************/
DiscPipeline::~DiscPipeline()
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
void DiscPipeline::startWorker(int *splitterCount, int *count)
{
    if (mData->mInterrupted) {
        return;
    }

    if (*count <= 0) {
        return;
    }

    if (*splitterCount > 0 && !mData->mSplitterRequests.isEmpty()) {
        mData->startSplitterThread(mData->mSplitterRequests.takeFirst());
        --(*splitterCount);
        --(*count);
        return;
    }

    if (mData->mJob.gainType == GainType::Track) {
        while (*count > 0 && !mData->mGainRequests.isEmpty()) {
            mData->startTrackGainThread(mData->mGainRequests.takeFirst());
            --(*count);
        }
    }
    else if (mData->mJob.gainType == GainType::Album) {
        if (*count > 0 && mData->mGainRequests.count() == mData->mTracks.count()) {
            mData->startAlbumGainThread(mData->mGainRequests);
            mData->mGainRequests.clear();
            --(*count);
        }
    }

    while (*count > 0 && !mData->mEncoderRequests.isEmpty()) {
        mData->startEncoderThread(mData->mEncoderRequests.takeFirst());
        --(*count);
    }
}

/************************************************
 *
 ************************************************/
bool DiscPipeline::Data::copyCoverImage() const
{
    QString dir = QFileInfo(mJob.tracks.first().resultFilePath()).dir().absolutePath();

    CopyCover copyCover(mJob.coverImage, dir, "cover", mJob.coverImageSize);
    bool      res = copyCover.run();

    if (!res) {
        Messages::error(copyCover.errorString());
    }

    return res;
}

/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startSplitterThread(const SplitterJob &req)
{
    Splitter *    worker = new Splitter(req);
    WorkerThread *thread = new WorkerThread(worker, mPipeline);

    connect(mPipeline, &DiscPipeline::threadQuit, thread, &Conv::WorkerThread::terminate);
    connect(worker, &Splitter::trackProgress, mPipeline, &DiscPipeline::trackProgress);
    connect(worker, &Worker::error, mPipeline, &DiscPipeline::trackError);
    connect(worker, &Splitter::trackReady, mPipeline, &DiscPipeline::addEncoderRequest);
    connect(thread, &Conv::WorkerThread::finished, mPipeline, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();

    mTracks[req.tracks.first().id()].setState(TrackState::Splitting);

    if (mJob.coverImage.isEmpty()) {
        copyCoverImage();
    }
}

/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startEncoderThread(const EncoderJob &req)
{
    Encoder *     worker = new Encoder(req);
    WorkerThread *thread = new WorkerThread(worker, mPipeline);

    connect(mPipeline, &DiscPipeline::threadQuit, thread, &Conv::WorkerThread::terminate);
    connect(worker, &Encoder::trackProgress, mPipeline, &DiscPipeline::trackProgress);
    connect(worker, &Encoder::error, mPipeline, &DiscPipeline::trackError);

    if (mJob.gainType == GainType::Disable) {
        connect(worker, &Encoder::trackReady, mPipeline, &DiscPipeline::trackDone);
    }
    else {
        connect(worker, &Encoder::trackReady, mPipeline, &DiscPipeline::addGainRequest);
    }

    connect(thread, &Conv::WorkerThread::finished, mPipeline, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startTrackGainThread(const GainJob &req)
{
    Gain *        worker = new Gain(req);
    WorkerThread *thread = new WorkerThread(worker, mPipeline);

    connect(mPipeline, &DiscPipeline::threadQuit, thread, &Conv::WorkerThread::terminate);
    connect(worker, &Gain::trackProgress, mPipeline, &DiscPipeline::trackProgress);
    connect(worker, &Gain::error, mPipeline, &DiscPipeline::trackError);
    connect(worker, &Gain::trackReady, mPipeline, &DiscPipeline::trackDone);
    connect(thread, &Conv::WorkerThread::finished, mPipeline, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::Data::startAlbumGainThread(const GainJobs &reqs)
{
    Gain *worker = new Gain(reqs);

    WorkerThread *thread = new WorkerThread(worker, mPipeline);
    connect(mPipeline, &DiscPipeline::threadQuit, thread, &Conv::WorkerThread::terminate);
    connect(worker, &Gain::trackProgress, mPipeline, &DiscPipeline::trackProgress);
    connect(worker, &Gain::error, mPipeline, &DiscPipeline::trackError);
    connect(worker, &Gain::trackReady, mPipeline, &DiscPipeline::trackDone);
    connect(thread, &Conv::WorkerThread::finished, mPipeline, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::addEncoderRequest(const ConvTrack &track, const QString &inputFile)
{
    trackProgress(track, TrackState::Queued, 0);

    EncoderJob job;
    job.track     = track;
    job.inputFile = inputFile;

    QFileInfo trackFile(track.resultFilePath());
    job.outFile = QDir(mData->mTmpDir->path()).filePath(QFileInfo(inputFile).baseName() + ".encoded." + trackFile.suffix());
    job.format  = mData->mJob.format;

    mData->mEncoderRequests << job;
    emit readyStart();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::addGainRequest(const ConvTrack &track, const QString &fileName)
{
    if (mData->mJob.gainType == GainType::Album) {
        trackProgress(track, TrackState::WaitGain, 0);
    }
    else {
        trackProgress(track, TrackState::Queued, 0);
    }

    mData->mGainRequests.append(GainJob(track, fileName, mData->mJob.format));
    emit readyStart();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::trackDone(const ConvTrack &track, const QString &outFileName)
{
    qCDebug(LOG) << "Track done: "
                 << "id=" << track.id()
                 << track
                 << "outFileName:" << outFileName;

    // Track is ready, rename the file to the final name.
    // Remove old already existing file.
    QFile::remove(track.resultFilePath());

    QFile file(outFileName);
    if (!file.rename(track.resultFilePath())) {
        trackError(track, tr("I can't rename file:\n%1 to %2\n%3").arg(outFileName).arg(track.resultFilePath()).arg(file.errorString()));
    }

    mData->mTracks[track.id()].setState(TrackState::OK);
    emit trackProgressChanged(track, TrackState::OK, 0);
    emit threadFinished();

    if (!isRunning()) {
        qCDebug(LOG) << "pipline finished";
        emit finished();
    }
}

/************************************************

 ************************************************/
void DiscPipeline::Data::interrupt(TrackState state)
{
    mInterrupted = true;
    mEncoderRequests.clear();

    for (ConvTrack &track : mTracks) {
        switch (track.state()) {
            case TrackState::Splitting:
            case TrackState::Encoding:
            case TrackState::Queued:
            case TrackState::WaitGain:
            case TrackState::CalcGain:
            case TrackState::WriteGain:
            case TrackState::NotRunning:
                track.setState(state);
                emit mPipeline->trackProgressChanged(track, state, 0);
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
void DiscPipeline::trackError(const ConvTrack &track, const QString &message)
{
    mData->mTracks[track.id()].setState(TrackState::Error);
    emit trackProgressChanged(track, TrackState::Error, 0);
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
    for (const ConvTrack &track : mData->mTracks) {
        switch (track.state()) {
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
    foreach (WorkerThread *thread, mData->mThreads) {
        if (thread->isRunning())
            ++res;
    }
    return res;
}

/************************************************

 ************************************************/
void DiscPipeline::trackProgress(const ConvTrack &track, TrackState state, int percent)
{
    if (mData->mInterrupted)
        return;

    mData->mTracks[track.id()].setState(state);
    emit trackProgressChanged(track, state, percent);
}
