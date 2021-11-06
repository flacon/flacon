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

#include "disc.h"
#include "converter.h"
#include "project.h"
#include "settings.h"
#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "discpipline.h"
#include "sox.h"
#include "cuecreator.h"

#include <iostream>
#include <math.h>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "Converter")
}

using namespace Conv;

class Converter::Data
{
public:
    TrackId trackId     = 1;
    int     threadCount = 0;

    QVector<DiscPipeline *>        discPiplines;
    QMap<TrackId, const ConvTrack> tracks;

    QString workDir(const Track *track) const;
};

/************************************************
 *
 ************************************************/
QString Converter::Data::workDir(const Track *track) const
{
    QString dir = Settings::i()->tmpDir();
    if (dir.isEmpty()) {
        dir = QFileInfo(track->resultFilePath()).dir().absolutePath();
    }
    return dir + "/tmp";
}

/************************************************

 ************************************************/
Converter::Converter(QObject *parent) :
    QObject(parent),
    mData(new Data())
{
    qRegisterMetaType<Conv::ConvTrack>();
}

/************************************************
 *
 ************************************************/
Converter::~Converter()
{
    delete mData;
}

/************************************************

 ************************************************/
void Converter::start(const Profile &profile)
{
    Jobs jobs;
    for (int d = 0; d < project->count(); ++d) {
        Job job;
        job.disc = project->disc(d);

        for (int t = 0; t < job.disc->count(); ++t)
            job.tracks << job.disc->track(t);

        jobs << job;
    }

    start(jobs, profile);
}

/************************************************
 *
 ************************************************/
void Converter::start(const Converter::Jobs &jobs, const Profile &profile)
{
    qCDebug(LOG) << "Start converter:" << jobs.length() << "\n"
                 << profile;
    qCDebug(LOG) << "Temp dir =" << Settings::i()->tmpDir();

    if (jobs.isEmpty()) {
        emit finished();
        return;
    }

    if (!check(profile)) {
        emit finished();
        return;
    }

    bool ok;
    mData->threadCount = Settings::i()->value(Settings::Encoder_ThreadCount).toInt(&ok);
    if (!ok || mData->threadCount < 1) {
        mData->threadCount = qMax(6, QThread::idealThreadCount());
    }

    qCDebug(LOG) << "Threads count" << mData->threadCount;

    try {
        for (const Job &converterJob : jobs) {

            if (converterJob.tracks.isEmpty() || converterJob.disc->isEmpty()) {
                continue;
            }

            if (!converterJob.disc->canConvert()) {
                continue;
            }

            mData->discPiplines << createDiscPipeline(profile, converterJob);
        }
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "Can't start " << err.what();
        emit error(err.what());
        qDeleteAll(mData->discPiplines);
        mData->discPiplines.clear();
        emit finished();
    }

    startThread();
    emit started();
}

/************************************************
 *
 ************************************************/
DiscPipeline *Converter::createDiscPipeline(const Profile &profile, const Converter::Job &converterJob)
{
    // Tracks ..............................
    ConvTracks resTracks;

    PreGapType preGapType = profile.isCreateCue() ? profile.preGapType() : PreGapType::Skip;

    for (const TrackPtrList &tracks : converterJob.disc->tracksByFileTag()) {

        // Pregap track ....................
        bool hasPregap =
                converterJob.tracks.contains(tracks.first()) && // We extract first track in Audio
                tracks.first()->cueIndex(1).milliseconds() > 0; // The first track don't start from zero second

        if (hasPregap && preGapType == PreGapType::ExtractToFile) {
            Track *firstTrack = tracks.first();

            Track pregapTrack = *firstTrack; // copy tags and all settings
            pregapTrack.setCueFileName(firstTrack->cueFileName());
            pregapTrack.setTag(TagId::TrackNum, QByteArray("0"));
            pregapTrack.setTitle("(HTOA)");

            ConvTrack track(pregapTrack);
            track.setId((mData->trackId)++);
            track.setPregap(true);
            track.setEnabled(true);

            track.setStart(firstTrack->cueIndex(0));
            track.setEnd(firstTrack->cueIndex(1));

            resTracks << track;
        }

        // Tracks ..........................
        for (int i = 0; i < tracks.count(); ++i) {
            const Track *t = tracks.at(i);

            ConvTrack track(*t);
            track.setId((mData->trackId)++);
            track.setPregap(false);
            track.setEnabled(converterJob.tracks.contains(t));

            if (i == 0 && hasPregap && preGapType == PreGapType::AddToFirstTrack) {
                track.setStart(CueIndex("00:00:00"));
            }
            else {
                track.setStart(t->cueIndex(1));
            }

            if (i < tracks.count() - 1) {
                track.setEnd(tracks.at(i + 1)->cueIndex(01));
            }

            resTracks << track;
        }
    }

    QString wrkDir = mData->workDir(converterJob.tracks.first());

    DiscPipeline *pipeline = new DiscPipeline(profile, converterJob.disc, resTracks, wrkDir, this);

    connect(pipeline, &DiscPipeline::readyStart, this, &Converter::startThread);
    connect(pipeline, &DiscPipeline::threadFinished, this, &Converter::startThread);
    connect(pipeline, &DiscPipeline::trackProgressChanged, this, &Converter::trackProgress);

    return pipeline;
}

/************************************************

 ************************************************/
bool Converter::isRunning()
{
    foreach (DiscPipeline *pipe, mData->discPiplines) {
        if (pipe->isRunning())
            return true;
    }

    return false;
}

/************************************************

 ************************************************/
bool Converter::canConvert()
{
    if (!Settings::i()->currentProfile().isValid()) {
        return false;
    }

    for (int i = 0; i < project->count(); ++i) {
        if (project->disc(i)->canConvert())
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

    foreach (DiscPipeline *pipe, mData->discPiplines) {
        pipe->stop();
    }
}

/************************************************

 ************************************************/
void Converter::startThread()
{
    int count         = mData->threadCount;
    int splitterCount = qMax(1.0, ceil(count / 2.0));

    foreach (DiscPipeline *pipe, mData->discPiplines) {
        count -= pipe->runningThreadCount();
    }

    foreach (DiscPipeline *pipe, mData->discPiplines) {
        pipe->startWorker(&splitterCount, &count);
        if (count <= 0) {
            break;
        }
    }

    foreach (DiscPipeline *pipe, mData->discPiplines) {
        if (pipe->isRunning()) {
            return;
        }
    }

    emit finished();
}

/************************************************

 ************************************************/
bool Converter::check(const Profile &profile) const
{
    QStringList errors;
    if (!profile.isValid()) {
        errors << "Incorrect output profile";
        return false;
    }

    bool ok = profile.check(&errors);

    if (profile.bitsPerSample() != BitsPerSample::AsSourcee || profile.sampleRate() != SampleRate::AsSource) {
        if (!Settings::i()->checkProgram(Sox::programName())) {
            errors << QObject::tr("I can't find program <b>%1</b>.").arg(Sox::programName());
            ok = false;
        }
    }

    if (!ok) {
        QString s;
        foreach (QString e, errors) {
            s += QString("<li style='margin-top: 4px;'> %1</li>").arg(e);
        }

        Messages::error(QString("<html>%1<ul>%2</ul></html>")
                                .arg(tr("Conversion is not possible:"), s));
    }

    return ok;
}
