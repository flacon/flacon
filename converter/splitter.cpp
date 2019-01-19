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
#include "disk.h"
#include "decoder.h"

#include <QDebug>


/************************************************
 *
 ************************************************/
Splitter::Splitter(const Converter::Job &job, const QString &tmpFilePrefix, bool extractPregap, PreGapType preGapType, QObject *parent):
    Worker(parent),
    mJob(job),
    mTmpFilePrefix(tmpFilePrefix),
    mPreGapType(preGapType),
    mExtractPregap(extractPregap),
    mCurrentTrack(nullptr)
{
}


/************************************************
 *
 ************************************************/
void Splitter::run()
{
    mCurrentTrack = nullptr;
    Decoder decoder;

    if (!decoder.open(mJob.disk->audioFileName()))
    {
        error(mJob.tracks.first(),
              tr("I can't read <b>%1</b>:<br>%2",
                 "Splitter error. %1 is a file name, %2 is a system error text.")
              .arg(mJob.disk->audioFileName())
              .arg(decoder.errorString()));
        return;
    }


    // Extract pregap to separate file ....................
    if (mExtractPregap)
    {
        mCurrentTrack = mJob.disk->preGapTrack();
        CueIndex start = mJob.disk->track(0)->cueIndex(0);
        CueIndex end   = mJob.disk->track(0)->cueIndex(1);
        QString outFileName = QString("%1%2.wav").arg(mTmpFilePrefix).arg(0, 2, 10, QLatin1Char('0'));

        try
        {
            decoder.extract(start, end, outFileName);
        }
        catch (QString &err)
        {
            qWarning() << "Splitter error for pregap track : " <<  decoder.errorString();
            deleteFile(outFileName);
            error(mCurrentTrack, decoder.errorString());
            return;
        }

        emit trackReady(mCurrentTrack, outFileName);
    }
    // Extract pregap to separate file ....................



    // We havn't pregap in the GUI and pregap is short so we suppress progress for pregap track
    connect(&decoder, SIGNAL(progress(int)),
            this, SLOT(decoderProgress(int)));

    for (int i=0; i<mJob.disk->count(); ++i)
    {
        mCurrentTrack = mJob.disk->track(i);
        if (!mJob.tracks.contains(mCurrentTrack))
            continue;

        QString outFileName = QString("%1%2.wav").arg(mTmpFilePrefix).arg(i+1, 2, 10, QLatin1Char('0'));

        CueIndex start, end;
        if (i==0 && mPreGapType == PreGapType::AddToFirstTrack)
            start = CueTime("00:00:00");
        else
            start = mJob.disk->track(i)->cueIndex(1);

        if (i<mJob.disk->count()-1)
            end = mJob.disk->track(i+1)->cueIndex(01);

        bool ret = decoder.extract(start, end, outFileName);


        if (!ret)
        {
            qWarning() << "Splitter error for track " << mCurrentTrack->trackNum() << ": " <<  decoder.errorString();
            deleteFile(outFileName);
            error(mCurrentTrack, decoder.errorString());
            return;
        }

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
