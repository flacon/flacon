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

#ifndef DISCPIPLINE_H
#define DISCPIPLINE_H

#include <QObject>
#include <QTemporaryDir>
#include "track.h"
#include "converter.h"
#include "convertertypes.h"
#include "profiles.h"
#include "coverimage.h"
#include "replaygain.h"

class Project;

namespace Conv {

class WorkerThread;

class DiscPipeline : public QObject
{
    Q_OBJECT
public:
    explicit DiscPipeline(const Profile &profile, Disc *disc, ConvTracks tracks, const QString &workDir, QObject *parent = nullptr) noexcept(false);
    virtual ~DiscPipeline();

    QList<ConvTrack> tracks() const { return mTracks; }

    void startWorker(int *splitterCount, int *count);
    void stop();
    bool isRunning() const;
    int  runningThreadCount() const;

signals:
    void readyStart();
    void threadFinished();
    void finished();
    void stopAllThreads();
    void trackProgressChanged(const Conv::ConvTrack &track, TrackState status, Percent percent);

private slots:
    void trackProgress(const Conv::ConvTrack &track, TrackState state, int percent);
    void trackError(const Conv::ConvTrack &track, const QString &message);

    void trackDone(const Conv::ConvTrack &track, const QString &outFileName);

private:
    Profile               mProfile;
    Disc                 *mDisc = nullptr;
    QString               mWorkDir;
    QList<ConvTrack>      mTracks;
    QMap<int, TrackState> mTrackStates;
    QTemporaryDir        *mTmpDir = nullptr;
    CoverImage            mCoverImage;
    QString               mEmbeddedCue;
    ReplayGain::AlbumGain mAlbumGain;

    struct SplitterRequest
    {
        ConvTracks tracks;
        QString    outDir;
        PreGapType pregapType;
    };

    struct Request
    {
        ConvTrack track;
        QString   inputFile;
    };

    QVector<WorkerThread *> mThreads;
    bool                    mInterrupted = false;
    QList<SplitterRequest>  mSplitterRequests;
    QList<Request>          mEncoderRequests;
    QList<Request>          mAlbumGainRequests;

    void addSpliterRequest();
    void startSplitter(const SplitterRequest &request);

    void addEncoderRequest(const Conv::ConvTrack &track, const QString &inputFile);
    void startEncoder(const ConvTrack &track, const QString &inputFile);

    void writeGain(const Conv::ConvTrack &track, const QString &fileName, const ReplayGain::Result &trackGain);

    void interrupt(TrackState state);

    void createDir(const QString &dirName) const;

    void copyCoverImage() const;
    void createEmbedImage();

    void writeOutCueFile();
    void loadEmbeddedCue();

    bool hasPregap() const;
    void updateDiskState();
};

} // Namespace

#endif // DISCPIPLINE_H
