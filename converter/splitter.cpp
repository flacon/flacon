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


/************************************************
 *
 ************************************************/
Splitter::Splitter(const Disc *disc, const QString &workDir, bool extractPregap, PreGapType preGapType, QObject *parent):
    Worker(parent),
    mDisc(disc),
    mWorkDir(workDir),
    mExtractPregap(extractPregap),
    mPreGapType(preGapType)
{
}


/************************************************
 *
 ************************************************/
void Splitter::run()
{
    static QAtomicInteger<quint32> globalUid(1);
    QString uid = QString("%1").arg(globalUid.fetchAndAddRelaxed(1), 4, 10, QLatin1Char('0'));

    mCurrentTrack = nullptr;
    Decoder decoder;

    try
    {
        decoder.open(mDisc->audioFileName());
    }
    catch (FlaconError &err)
    {
        emit error(mTracks.first(),
              tr("I can't read <b>%1</b>:<br>%2",
                 "Splitter error. %1 is a file name, %2 is a system error text.")
              .arg(mDisc->audioFileName())
              .arg(err.what()));
        return;
    }


    // Extract pregap to separate file ....................
    if (mExtractPregap)
    {
        mCurrentTrack  = mDisc->preGapTrack();
        CueIndex start = mDisc->track(0)->cueIndex(0);
        CueIndex end   = mDisc->track(0)->cueIndex(1);
        QString outFileName = QString("%1/pregap-%2.wav").arg(mWorkDir).arg(uid);

        try
        {
            decoder.extract(start, end, outFileName);
        }
        catch (FlaconError &err)
        {
            qWarning() << "Splitter error for pregap track : " <<  err.what();
            deleteFile(outFileName);
            emit error(mCurrentTrack, err.what());
            return;
        }

        emit trackReady(mCurrentTrack, outFileName);
    }
    // Extract pregap to separate file ....................



    // We havn't pregap in the GUI and pregap is short so we suppress progress for pregap track
    connect(&decoder, SIGNAL(progress(int)),
            this, SLOT(decoderProgress(int)));

    qDebug() << "Start splitting" << mDisc->count() << "tracks from" << mDisc->audioFileName();
    for (int i=0; i<mDisc->count(); ++i)
    {
        mCurrentTrack = mDisc->track(i);
        if (!mTracks.contains(mCurrentTrack))
            continue;

        QString outFileName = QString("%1/track-%2_%3.wav").arg(mWorkDir).arg(uid).arg(i+1, 2, 10, QLatin1Char('0'));

        CueIndex start, end;
        if (i==0 && mPreGapType == PreGapType::AddToFirstTrack)
            start = CueTime("00:00:00");
        else
            start = mDisc->track(i)->cueIndex(1);

        if (i<mDisc->count()-1)
            end = mDisc->track(i+1)->cueIndex(01);

        try
        {
            decoder.extract(start, end, outFileName);
        }
        catch (FlaconError &err)
        {
            qWarning() << "Splitter error for track " << mCurrentTrack->trackNum() << ": " <<  err.what();
            deleteFile(outFileName);
            emit error(mCurrentTrack, err.what());

        }

        qDebug() << "Splitter trackReady:" << *mCurrentTrack << outFileName;
        emit trackReady(mCurrentTrack, outFileName);
    }
}


/************************************************
 *
 ************************************************/
void Splitter::decoderProgress(int percent)
{
    emit trackProgress(mCurrentTrack, TrackState::Splitting, percent);
}
