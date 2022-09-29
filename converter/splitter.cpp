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

#include "splitter.h"
#include "disc.h"
#include "decoder.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QFile>

namespace {
Q_LOGGING_CATEGORY(LOG, "Splitter")
}

using namespace Conv;

struct Splitter::Job
{
    struct Chunk
    {
        InputAudioFile file;
        CueTime        start;
        CueTime        end;
        Decoder *      decoder = nullptr;
    };

    const Disc *    disk = nullptr;
    Conv::ConvTrack track;
    QList<Chunk>    chunks;
    QString         outFileName;
    bool            isPregap = false;

    Job(const Disc *disk, Conv::ConvTrack track, bool addPregap, bool addTrack, bool addPostgap);
    QList<Chunk>   getPart(const CueIndex &from, const CueIndex &to) const;
    void           merge();
    InputAudioFile getInputAudioFile(const QByteArray &fileTag) const;
};

/************************************************
 *
 ************************************************/
Splitter::Job::Job(const Disc *disk, Conv::ConvTrack track, bool addPregap, bool addTrack, bool addPostgap) :
    disk(disk),
    track(track)
{
    const Track cur  = *(disk->track(track.index()));
    const Track next = (cur.index() + 1 < disk->count()) ? *(disk->track(cur.index() + 1)) : Track();

    if (addPregap) {
        chunks << getPart(cur.cueIndex(0), cur.cueIndex(1));
    }

    if (addTrack) {
        chunks << getPart(cur.cueIndex(1), next.cueIndex(0));
    }

    if (addPostgap) {
        chunks << getPart(next.cueIndex(0), next.cueIndex(1));
    }

    merge();
}

/************************************************
 *
 ************************************************/
void Splitter::Job::merge()
{
    for (int i = chunks.size() - 2; i >= 0; --i) {
        if (chunks.at(i).file == chunks.at(i + 1).file) {
            chunks[i].end = chunks.at(i + 1).end;
            chunks.removeAt(i + 1);
        }
    }
}

/************************************************
 *
 ************************************************/
QList<Splitter::Job::Chunk> Splitter::Job::getPart(const CueIndex &from, const CueIndex &to) const
{
    //qDebug() << "***************************";
    //qDebug() << "* FROM: " << from.file() << from.toString(true);
    //qDebug() << "* TO:   " << to.file() << to.toString(true);
    //qDebug() << "***************************";

    // The 00 and 01 indexes are in the same file, or is it the last track (01 index is empty)
    if (from.file() == to.file() || to.file().isEmpty()) {
        if (from == to) {
            return {};
        }

        Chunk res;
        res.file  = getInputAudioFile(from.file());
        res.start = from;
        res.end   = to;
        return { res };
    }

    Chunk first;
    first.file  = getInputAudioFile(from.file());
    first.start = from;

    Chunk second;
    second.file = getInputAudioFile(to.file());
    second.end  = to;

    return { first, second };
}

/************************************************
 *
 ************************************************/
InputAudioFile Splitter::Job::getInputAudioFile(const QByteArray &fileTag) const
{
    QByteArray prevTag;

    int n = 0;
    for (int i = 0; i < disk->count(); ++i) {
        const Track *t = disk->track(i);

        for (int j = 0; j <= 1; ++j) {
            if (t->cueIndex(j).file() == fileTag) {
                return disk->audioFiles().at(n);
            }

            if (t->cueIndex(j).file() != prevTag) {
                n++;
                prevTag = t->cueIndex(j).file();
            }
        }
    }

    throw FlaconError(QString("Incorrect file tag %1").arg(fileTag.data()));
}

/************************************************
 *
 ************************************************/
Splitter::Splitter(Disc *disk, const ConvTracks &tracks, const QString &outDir, QObject *parent) :
    Worker(parent),
    mDisc(disk),
    mTracks(tracks),
    mOutDir(outDir)
{
}

/************************************************
 *
 ************************************************/
void Splitter::setPregapType(const PreGapType &pregapType)
{
    mPregapType = pregapType;
}

/************************************************
 *
 ************************************************/
void Splitter::run()
{
    static QAtomicInteger<quint32> globalUid(1);
    QString                        uid = QString("%1").arg(globalUid.fetchAndAddRelaxed(1), 4, 10, QLatin1Char('0'));

    // ******************************************
    // Create jobs
    QList<Job> jobs;
    for (const ConvTrack &track : mTracks) {
        if (track.isPregap()) {
            Job job(mDisc, mTracks.first(), true, false, false);
            job.outFileName = QString("%1/pregap-%2.wav").arg(mOutDir, uid);
            job.isPregap    = true;
            jobs << job;
            continue;
        }

        bool addPregap = (track.index() == 0 && mPregapType == PreGapType::AddToFirstTrack);
        Job  job(mDisc, track, addPregap, true, true);
        job.outFileName = QString("%1/track-%2_%3.wav").arg(mOutDir, uid).arg(track.trackNum(), 2, 10, QLatin1Char('0'));
        jobs << job;
    }

    for (const Job &job : jobs) {
        qCDebug(LOG) << "Spliter job _________________________";
        qCDebug(LOG) << "  track index: " << job.track.index();
        qCDebug(LOG) << "  outFileName: " << job.outFileName;
        qCDebug(LOG) << "  isPregap:    " << job.isPregap;
        qCDebug(LOG) << "  chunks:    ";
        for (const Job::Chunk &chunk : job.chunks) {
            qCDebug(LOG) << "  * " << chunk.start.toString() << ":" << chunk.end.toString() << " file" << chunk.file.filePath();
        }
    }

    // ******************************************
    // Validate jobs
    try {
        if (jobs.isEmpty()) {
            throw FlaconError("Jobs is empty");
        }

        for (const Job &job : jobs) {
            if (job.chunks.isEmpty()) {
                throw FlaconError("Job chunks is empty");
            }
        }
    }
    catch (FlaconError &err) {
        qCWarning(LOG) << "Incorrect splitter jobs : " << err.what();
        emit error(mTracks.first(), err.what());
        return;
    }

    // ******************************************
    // Create and open decoders
    QObject                  keeper;
    QMap<QString, Decoder *> decoders;
    for (const Job &job : jobs) {
        for (const Job::Chunk &chunk : job.chunks) {
            decoders.insert(chunk.file.filePath(), nullptr);
        }
    }

    for (const QString &file : decoders.keys()) {
        try {
            Decoder *decoder = new Decoder(&keeper);
            decoder->open(file);
            decoders[file] = decoder;
        }
        catch (FlaconError &err) {
            emit error(mTracks.first(), tr("I can't read <b>%1</b>:<br>%2", "Splitter error. %1 is a file name, %2 is a system error text.").arg(file, err.what()));
            return;
        }
    }

    for (Job &job : jobs) {
        for (Job::Chunk &chunk : job.chunks) {
            chunk.decoder = decoders.value(chunk.file.filePath());
        }
    }

    // ******************************************
    // Decode data
    for (const Job &job : jobs) {
        try {
            processTrack(job);
            qCDebug(LOG) << "Splitter trackReady:" << job.track << job.outFileName;
            emit trackReady(job.track, job.outFileName);
        }
        catch (FlaconError &err) {
            if (job.isPregap) {
                qCWarning(LOG) << "Splitter error for pregap track : " << err.what();
            }
            else {
                qCWarning(LOG) << "Splitter error for track " << job.track.trackNum() << ": " << err.what();
            }

            emit error(job.track, err.what());
            deleteFile(job.outFileName);
            return;
        }
    }
}

struct ProgressCalc
{
    uint64_t totalSize = 0;
    uint64_t chunkSize = 0;
    uint64_t chunkDone = 0;
    uint64_t done      = 0;
};

/************************************************
 *
 ************************************************/
void Splitter::processTrack(const Job &job)
{

    emit trackProgress(job.track, TrackState::Splitting, 0);

    QFile outFile(job.outFileName);
    if (!outFile.open(QFile::WriteOnly)) {
        throw outFile.errorString();
    }

    uint32_t bytes = 0;
    for (const Job::Chunk &chunk : job.chunks) {
        bytes += chunk.decoder->bytesCount(chunk.start, chunk.end);
    }

    WavHeader hdr = job.chunks.first().decoder->wavHeader();
    hdr.resizeData(bytes);
    outFile.write(hdr.toLegacyWav());

    ProgressCalc progress;
    progress.totalSize = bytes;

    for (const Job::Chunk &chunk : job.chunks) {
        progress.chunkSize = chunk.decoder->bytesCount(chunk.start, chunk.end);

        // Extract chunk .............................
        QObject keeper;
        connect(chunk.decoder, &Decoder::progress, &keeper, [this, job, progress](int percents) {
            double chunkDone = double(percents) / 100 * progress.chunkSize;
            emit   trackProgress(job.track, TrackState::Splitting, (progress.done + chunkDone) / progress.totalSize * 100);
        });
        progress.done += progress.chunkSize;

        qCDebug(LOG) << "extract: " << chunk.file.filePath() << " [" << chunk.start.toString() << ":" << chunk.end.toString() << "] OUT:" << job.outFileName;
        chunk.decoder->extract(chunk.start, chunk.end, &outFile, false);
    }

    outFile.close();
    emit trackProgress(job.track, TrackState::Splitting, 100);
}
