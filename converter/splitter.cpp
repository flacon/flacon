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

#include <QDebug>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "Splitter")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
Splitter::Job::Job(const Disc *disk, Conv::ConvTrack track, bool addPregap, bool addTrack, bool addPostgap) :
    QList<Splitter::Chunk>(),
    mDisk(disk),
    mTrack(track)
{
    const Track cur  = *(disk->track(track.index()));
    const Track next = (cur.index() + 1 < disk->count()) ? *(disk->track(cur.index() + 1)) : Track();

    if (addPregap) {
        (*this) << getPart(cur.cueIndex(0), cur.cueIndex(1));
    }

    if (addTrack) {
        (*this) << getPart(cur.cueIndex(1), next.cueIndex(0));
    }

    if (addPostgap) {
        (*this) << getPart(next.cueIndex(0), next.cueIndex(1));
    }

    merge();
}

/************************************************
 *
 ************************************************/
void Splitter::Job::merge()
{
    for (int i = size() - 2; i >= 0; --i) {
        if (this->at(i).file == this->at(i + 1).file) {
            (*this)[i].end = this->at(i + 1).end;
            this->removeAt(i + 1);
        }
    }
}

/************************************************
 *
 ************************************************/
QList<Splitter::Chunk> Splitter::Job::getPart(const CueIndex &from, const CueIndex &to) const
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
    first.file = getInputAudioFile(to.file());
    first.end  = to;

    return { first, second };
}

/************************************************
 *
 ************************************************/
InputAudioFile Splitter::Job::getInputAudioFile(const QByteArray &fileTag) const
{
    QByteArray prevTag;

    int n = 0;
    for (int i = 0; i < mDisk->count(); ++i) {
        const Track *t = mDisk->track(i);

        for (int j = 0; j <= 1; ++j) {
            if (t->cueIndex(j).file() == fileTag) {
                return mDisk->audioFiles().at(n);
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

    QString uid = QString("%1").arg(globalUid.fetchAndAddRelaxed(1), 4, 10, QLatin1Char('0'));

    // Pregap ...................................
    if (mTracks.first().index() == 0 && mPregapType == PreGapType::ExtractToFile) {
        Job job(mDisc, mTracks.first(), true, false, false);
        //, QString("%1/pregap-%2.wav").arg(mOutDir, uid)
        //for (const Chunk &chunk : job) {
    }

    QString outFileName;
    for (const ConvTrack &track : mTracks) {
        try {
            bool addPregap = (track.index() == 0 && mPregapType == PreGapType::AddToFirstTrack);
            Job  job(mDisc, track, addPregap, true, true);
            outFileName = QString("%1/track-%2_%3.wav").arg(mOutDir, uid).arg(track.trackNum(), 2, 10, QLatin1Char('0'));

            processJob(job, outFileName);

            qCDebug(LOG) << "Splitter trackReady:" << track << outFileName;
            emit trackReady(track, outFileName);
        }
        catch (FlaconError &err) {
            qCCritical(LOG) << "Splitter error for track " << track.trackNum() << ": " << err.what();
            emit error(track, err.what());
            deleteFile(outFileName);
            return;
        }
    }
}

void Splitter::processJob(const Job &job, const QString &outFileName)
{
    QObject keeper;

    for (const Chunk &chunk : job) {
        qDebug() << "@@@ SPLIT" << chunk.file.filePath() << " [" << chunk.start.toString() << ":" << chunk.end.toString() << "] OUT:" << outFileName;

        if (mDecoder.inputFile() != chunk.file.filePath()) {
            mDecoder.close();
            try {
                mDecoder.open(chunk.file.filePath());
            }
            catch (FlaconError &err) {
                throw FlaconError(tr("I can't read <b>%1</b>:<br>%2", "Splitter error. %1 is a file name, %2 is a system error text.").arg(mDecoder.inputFile(), err.what()));
            }
        }

        mDecoder.extract(chunk.start, chunk.end, outFileName);
    }
}

#if 0
    // First track ..............................
    if (mTracks.first().index() == 0) {
        jobs << Job(mDisc, mTracks.first(), mPregapType == PreGapType::AddToFirstTrack, true, true, QString("%1/track-%2_%3.wav").arg(mOutDir, uid).arg(0, 2, 10, QLatin1Char('0')));
    }

    // Other tracks .............................
    for (int i = 1; i < mTracks.size(); ++i) {
        jobs << Job(mDisc, mTracks.at(i), false, true, true, QString("%1/track-%2_%3.wav").arg(mOutDir, uid).arg(i, 2, 10, QLatin1Char('0')));
    }

    // Split ....................................
    Decoder decoder;

    for (const Job &job : jobs) {
        QString outFileName = "";
        for (const Chunk &chunk : job) {
            if (decoder.inputFile() != chunk.file.filePath()) {
                decoder.close();
                try {
                    decoder.open(chunk.file.filePath());
                }
                catch (FlaconError &err) {
                    emit error(mTracks.at(0),
                               tr("I can't read <b>%1</b>:<br>%2",
                                  "Splitter error. %1 is a file name, %2 is a system error text.")
                                       .arg(decoder.inputFile(), err.what()));
                    return;
                }
            }

            try {
                QObject keeper;
                connect(&decoder, &Decoder::progress, &keeper, [this, &job](int percent) {
                    //                    Conv::ConvTrack track = job.track();
                    //                    emit trackProgress(job.track(), TrackState::Splitting, percent);
                });

                decoder.extract(chunk.start, chunk.end, outFileName);

                qCDebug(LOG) << "Splitter trackReady:" << track << outFileName;
                emit trackReady(track, outFileName);
            }
            catch (FlaconError &err) {
                if (job.track.isPregap()) {
                    qCWarning(LOG) << "Splitter error for pregap track : " << err.what();
                }
                else {
                    qCWarning(LOG) << "Splitter error for track " << track.trackNum() << ": " << err.what();
                }
                deleteFile(outFileName);
                emit error(track, err.what());
                return;
            }
        }
    }
#endif
/*
        Decoder decoder;

        try {
            decoder.open(mInFile);
        }
        catch (FlaconError &err) {
            emit error(mTracks.at(0),
                       tr("I can't read <b>%1</b>:<br>%2",
                          "Splitter error. %1 is a file name, %2 is a system error text.")
                               .arg(mInFile, err.what()));
            return;
        }

        for (const ConvTrack &track : mTracks) {

            // clang-format off
            QString outFileName =
                    track.isPregap() ?
                        QString("%1/pregap-%2.wav").arg(mOutDir, uid) :
                        QString("%1/track-%2_%3.wav").arg(mOutDir, uid).arg(track.trackNum(), 2, 10, QLatin1Char('0'));
            // clang-format on

            try {
                QObject keeper;
                connect(&decoder, &Decoder::progress, &keeper, [this, track](int percent) {
                    emit trackProgress(track, TrackState::Splitting, percent);
                });

                decoder.extract(track.start(), track.end(), outFileName);

                qCDebug(LOG) << "Splitter trackReady:" << track << outFileName;
                emit trackReady(track, outFileName);
            }
            catch (FlaconError &err) {
                if (track.isPregap()) {
                    qCWarning(LOG) << "Splitter error for pregap track : " << err.what();
                }
                else {
                    qCWarning(LOG) << "Splitter error for track " << track.trackNum() << ": " << err.what();
                }
                deleteFile(outFileName);
                emit error(track, err.what());
                return;
            }
        }
    */
//}
