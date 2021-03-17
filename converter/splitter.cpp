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

namespace {
Q_LOGGING_CATEGORY(LOG, "Converter")
}

using namespace Conv;

/************************************************
 *
 ************************************************/
Splitter::Splitter(const SplitterJob &job, QObject *parent) :
    Worker(parent),
    mJob(job)
{
}

/************************************************
 *
 ************************************************/
void Splitter::run()
{
    static QAtomicInteger<quint32> globalUid(1);
    QString                        uid = QString("%1").arg(globalUid.fetchAndAddRelaxed(1), 4, 10, QLatin1Char('0'));

    ConvTrack currentTrack;
    Decoder   decoder;

    try {
        decoder.open(mJob.audio.filePath());
    }
    catch (FlaconError &err) {
        emit error(mJob.track(0),
                   tr("I can't read <b>%1</b>:<br>%2",
                      "Splitter error. %1 is a file name, %2 is a system error text.")
                           .arg(mJob.audio.fileName())
                           .arg(err.what()));
        return;
    }

    // Extract pregap to separate file ....................
    if (mJob.preGapType == PreGapType::ExtractToFile) {
        currentTrack        = mJob.pregapTrack;
        QString outFileName = QString("%1/pregap-%2.wav").arg(mJob.outDir).arg(uid);

        try {
            decoder.extract(currentTrack.start(), currentTrack.end(), outFileName);
        }
        catch (FlaconError &err) {
            qWarning() << "Splitter error for pregap track : " << err.what();
            deleteFile(outFileName);
            emit error(currentTrack, err.what());
            return;
        }

        emit trackReady(currentTrack, outFileName);
    }
    // Extract pregap to separate file ....................

    // We havn't pregap in the GUI and pregap is short so we suppress progress for pregap track
    connect(&decoder, &Decoder::progress, [this, &currentTrack](int percent) {
        emit trackProgress(currentTrack, TrackState::Splitting, percent);
    });

    qCDebug(LOG) << "Start splitting" << mJob.tracks.count() << "tracks from" << mJob.audio.fileName();
    for (int i = 0; i < mJob.tracks.count(); ++i) {
        currentTrack        = mJob.track(i);
        QString outFileName = QString("%1/track-%2_%3.wav").arg(mJob.outDir).arg(uid).arg(i + 1, 2, 10, QLatin1Char('0'));

        try {
            decoder.extract(currentTrack.start(), currentTrack.end(), outFileName);
        }
        catch (FlaconError &err) {
            qWarning() << "Splitter error for track " << currentTrack.trackNum() << ": " << err.what();
            deleteFile(outFileName);
            emit error(currentTrack, err.what());
        }

        qCDebug(LOG) << "Splitter trackReady:" << currentTrack << outFileName;
        emit trackReady(currentTrack, outFileName);
    }
}
