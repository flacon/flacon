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
#include "settings.h"

#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <errno.h>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "DiscPipeline")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
class Conv::WorkerThread : public QThread
{
public:
    explicit WorkerThread(Worker *worker, QObject *parent = nullptr);
    virtual ~WorkerThread() override;

    void run() override;

private:
    Worker *mWorker;
};

/************************************************
 *
 ************************************************/
WorkerThread::WorkerThread(Worker *worker, QObject *parent) :
    QThread(parent),
    mWorker(worker)
{
    worker->moveToThread(this);
}

/************************************************
 *
 ************************************************/
WorkerThread::~WorkerThread()
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

/************************************************
 *
 ************************************************/
void WorkerThread::run()
{
    mWorker->run();
}

/************************************************
 *
 ************************************************/
DiscPipeline::DiscPipeline(const Profile &profile, Disc *disc, ConvTracks tracks, const QString &workDir, QObject *parent) noexcept(false) :
    QObject(parent),
    mProfile(profile),
    mDisc(disc),
    mWorkDir(workDir)
{

    QString dir = QFileInfo(mWorkDir).dir().absolutePath();
    qCDebug(LOG) << "Create tmp dir" << dir;

    createDir(dir);

    mTmpDir = new QTemporaryDir(QString("%1/tmp").arg(dir));
    mTmpDir->setAutoRemove(true);

    for (const ConvTrack &track : qAsConst(tracks)) {
        if (!track.isEnabled()) {
            continue;
        }

        mTracks[track.id()] = track;

        qCDebug(LOG) << "Create directory for output files" << dir;
        createDir(QFileInfo(track.resultFilePath()).absoluteDir().path());
    }

    // A disk can contain several audio files,
    // so we create several splitter requests.
    QString prev;
    for (const ConvTrack &t : qAsConst(tracks)) {
        if (!t.isEnabled()) {
            continue;
        }

        if (t.audioFile().filePath() != prev) {
            addSpliterRequest(t.audioFile());
            prev = t.audioFile().filePath();
        }
    }
}

/************************************************

 ************************************************/
DiscPipeline::~DiscPipeline()
{
    delete mTmpDir;
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
    if (mInterrupted) {
        return;
    }

    if (*count <= 0) {
        return;
    }

    if (*splitterCount > 0 && !mSplitterRequests.isEmpty()) {
        SplitterRequest req = mSplitterRequests.takeFirst();
        startSplitter(req.tracks, req.inFile, req.outDir);
        --(*splitterCount);
        --(*count);
        return;
    }

    if (mProfile.gainType() == GainType::Track) {
        while (*count > 0 && !mGainRequests.isEmpty()) {
            startGain(mGainRequests.takeFirst());
            --(*count);
        }
    }
    else if (mProfile.gainType() == GainType::Album) {
        if (*count > 0 && mGainRequests.count() == mTracks.count()) {
            startGain(mGainRequests);
            mGainRequests.clear();
            --(*count);
        }
    }

    while (*count > 0 && !mEncoderRequests.isEmpty()) {
        const Request req = mEncoderRequests.takeFirst();
        startEncoder(req.track, req.inputFile);
        --(*count);
    }
}

/************************************************
 *
 ************************************************/
void DiscPipeline::addSpliterRequest(const InputAudioFile &audio)
{
    QString    inFile = audio.filePath();
    QString    outDir = mTmpDir->path();
    ConvTracks tracks;

    for (const ConvTrack &t : qAsConst(mTracks)) {
        if (t.isEnabled() && t.audioFile().filePath() == audio.filePath()) {
            tracks << t;
        }
    }

    mSplitterRequests << SplitterRequest { tracks, inFile, outDir };
}

/************************************************
 *
 ************************************************/
void DiscPipeline::startSplitter(const ConvTracks &tracks, const QString &inFile, const QString &outDir)
{
    Splitter     *splitter = new Splitter(mProfile, tracks, inFile, outDir);
    WorkerThread *thread   = new WorkerThread(splitter, this);

    connect(this, &DiscPipeline::stopAllThreads, thread, &Conv::WorkerThread::deleteLater);
    connect(splitter, &Splitter::trackProgress, this, &DiscPipeline::trackProgress);
    connect(splitter, &Worker::error, this, &DiscPipeline::trackError);
    connect(splitter, &Splitter::trackReady, this, &DiscPipeline::addEncoderRequest);
    connect(thread, &Conv::WorkerThread::finished, this, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();

    mTracks[tracks.first().id()].setState(TrackState::Splitting);

    // *********************************************************
    // Short tasks, we do not allocate separate threads for them.
    try {
        copyCoverImage();
        createEmbedImage();
        createOutCue();
    }
    catch (const FlaconError &err) {
        trackError(tracks.first(), err.what());
    }
}

/************************************************
 *
 ************************************************/
void DiscPipeline::addEncoderRequest(const ConvTrack &track, const QString &inputFile)
{
    mEncoderRequests << Request { track, inputFile };
    trackProgress(track, TrackState::Queued, 0);
    emit readyStart();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::startEncoder(const ConvTrack &track, const QString &inputFile)
{
    QFileInfo trackFile(track.resultFilePath());
    QString   outFile = QDir(mTmpDir->path()).filePath(QFileInfo(inputFile).baseName() + ".encoded." + trackFile.suffix());

    Encoder *encoder = mProfile.outFormat()->createEncoder();
    encoder->setInputFile(inputFile);
    encoder->setOutFile(outFile);
    encoder->setTrack(track);
    encoder->setProfile(mProfile);
    encoder->setCoverFile(mEmbedCoverFile);

    WorkerThread *thread = new WorkerThread(encoder, this);

    connect(this, &DiscPipeline::stopAllThreads, thread, &Conv::WorkerThread::deleteLater);
    connect(encoder, &Encoder::trackProgress, this, &DiscPipeline::trackProgress);
    connect(encoder, &Encoder::error, this, &DiscPipeline::trackError);

    if (mProfile.gainType() == GainType::Disable) {
        connect(encoder, &Encoder::trackReady, this, &DiscPipeline::trackDone);
    }
    else {
        connect(encoder, &Encoder::trackReady, this, &DiscPipeline::addGainRequest);
    }

    connect(thread, &Conv::WorkerThread::finished, this, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::addGainRequest(const ConvTrack &track, const QString &fileName)
{
    if (mProfile.gainType() == GainType::Album) {
        trackProgress(track, TrackState::WaitGain, 0);
    }
    else {
        trackProgress(track, TrackState::Queued, 0);
    }

    mGainRequests << Request { track, fileName };
    emit readyStart();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::startGain(const DiscPipeline::Request &request)
{
    startGain(QList<Request>() << request);
}

/************************************************
 *
 ************************************************/
void DiscPipeline::startGain(const QList<Request> &requests)
{
    Gain *worker = mProfile.outFormat()->createGain(mProfile);
    for (const Request &req : requests) {
        worker->addTrack(req.track, req.inputFile);
    }

    WorkerThread *thread = new WorkerThread(worker, this);
    connect(this, &DiscPipeline::stopAllThreads, thread, &Conv::WorkerThread::deleteLater);
    connect(worker, &Gain::trackProgress, this, &DiscPipeline::trackProgress);
    connect(worker, &Gain::error, this, &DiscPipeline::trackError);
    connect(worker, &Gain::trackReady, this, &DiscPipeline::trackDone);
    connect(thread, &Conv::WorkerThread::finished, this, &DiscPipeline::threadFinished);

    mThreads << thread;
    thread->start();
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
        trackError(track, tr("I can't rename file:\n%1 to %2\n%3").arg(outFileName, track.resultFilePath(), file.errorString()));
    }

    mTracks[track.id()].setState(TrackState::OK);
    emit trackProgressChanged(track, TrackState::OK, 0);
    emit threadFinished();

    if (!isRunning()) {
        qCDebug(LOG) << "pipline finished";
        emit finished();
    }
}

/************************************************
 *
 ************************************************/
void DiscPipeline::createDir(const QString &dirName) const
{
    QDir dir(dirName);

    if (!dir.mkpath(".")) {
        QString msg = QObject::tr("I can't create directory \"%1\".").arg(dir.path());
        QString err = strerror(errno);
        throw FlaconError(msg + "<br><br>" + err);
    }

    if (!QFileInfo(dir.path()).isWritable()) {
        QString msg = QObject::tr("I can't write to directory \"%1\".").arg(dir.path());
        QString err = strerror(errno);
        throw FlaconError(msg + "<br><br>" + err);
    }
}

/************************************************

 ************************************************/
void DiscPipeline::interrupt(TrackState state)
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
                emit trackProgressChanged(track, state, 0);
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
    interrupt(TrackState::Aborted);
    emit stopAllThreads();
    emit threadFinished();

    emit finished();
}

/************************************************

 ************************************************/
void DiscPipeline::trackError(const ConvTrack &track, const QString &message)
{
    mTracks[track.id()].setState(TrackState::Error);
    emit trackProgressChanged(track, TrackState::Error, 0);
    interrupt(TrackState::Aborted);
    emit stopAllThreads();
    emit threadFinished();

    emit finished();
    Messages::error(message);
}

/************************************************

 ************************************************/
bool DiscPipeline::isRunning() const
{
    for (const ConvTrack &track : qAsConst(mTracks)) {
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
    foreach (WorkerThread *thread, mThreads) {
        if (thread->isRunning())
            ++res;
    }
    return res;
}

/************************************************

 ************************************************/
void DiscPipeline::trackProgress(const ConvTrack &track, TrackState state, int percent)
{
    if (mInterrupted)
        return;

    mTracks[track.id()].setState(state);
    emit trackProgressChanged(track, state, percent);
}

/************************************************
 *
 ************************************************/
void DiscPipeline::copyCoverImage() const
{
    QString file = Settings::i()->coverMode() != CoverMode::Disable ? mDisc->coverImageFile() : "";
    int     size = Settings::i()->coverMode() == CoverMode::Scale ? Settings::i()->coverImageSize() : 0;

    if (file.isEmpty()) {
        return;
    }

    QString dir = QFileInfo(mTracks.first().resultFilePath()).dir().absolutePath();

    CopyCover copyCover(file, size, dir, "cover");
    if (!copyCover.run()) {
        throw FlaconError(copyCover.errorString());
    }
}

/************************************************
 *
 ************************************************/
void DiscPipeline::createEmbedImage()
{
    QString file = Settings::i()->embededCoverMode() != CoverMode::Disable ? mDisc->coverImageFile() : "";
    int     size = Settings::i()->embededCoverMode() == CoverMode::Scale ? Settings::i()->embededCoverImageSize() : 0;

    if (file.isEmpty()) {
        return;
    }

    CopyCover copyCover(file, size, mTmpDir->path(), "cover");
    if (!copyCover.run()) {
        throw FlaconError(copyCover.errorString());
    }
    mEmbedCoverFile = copyCover.fileName();
}

/************************************************
 *
 ************************************************/
void DiscPipeline::createOutCue() const
{
    if (!mProfile.isCreateCue()) {
        return;
    }

    CueCreator cue(mDisc, mProfile.preGapType(), mProfile.cueFileName());
    if (!cue.write()) {
        throw FlaconError(cue.errorString());
    }
}
