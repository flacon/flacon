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


#ifndef DISKPIPLINE_H
#define DISKPIPLINE_H

#include <QObject>
#include <QTemporaryDir>
#include "track.h"
#include "converter.h"

class Disk;
class Project;
class WorkerThread;

class DiskPipeline : public QObject
{
    Q_OBJECT
public:
    explicit DiskPipeline(const Converter::Job &job, const Profile &profile, QObject *parent = nullptr);
    virtual ~DiskPipeline();

    bool init();
    void startWorker(int *splitterCount, int *count);
    void stop();
    bool isRunning() const;
    int runningThreadCount() const;

    CoverMode coverMode() const;
    void setCoverMode(CoverMode value);

    int coverImageSize() const;
    void setCoverImageSize(int value);

    QString tmpDir() const;
    void setTmpDir(QString value);

signals:
    void readyStart();
    void threadFinished();
    void finished();
    void threadQuit();
    void trackProgressChanged(const Track &track, TrackState status, Percent percent);


private slots:
    void trackProgress(const Track *track, TrackState state, int percent);
    void trackError(const Track *track, const QString &message);

    void addEncoderRequest(const Track *track, const QString &inputFile);
    void addGainRequest(const Track *track, const QString &fileName);
    void trackDone(const Track *track, const QString &outFileName);

protected:
   static int calcQuality(int input, int preferences, int formatMax);

private:
    class Data;
    Data *mData;
    QTemporaryDir *mTmpDir = nullptr;
    QVector<WorkerThread*> mThreads;
};

#endif // DISKPIPLINE_H
