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

#include <QDir>
#include <QUuid>
#include <QDebug>


/************************************************
 *
 ************************************************/
Splitter::Splitter(const Disk *disk, const QString &workDir, PreGapType preGapType, QObject *parent):
    Worker(parent),
    mDisk(disk),
    mWorkDir(workDir),
    mPreGapType(preGapType),
    mCurrentTrack(NULL)
{
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    mExtractPregapTrack = (mPreGapType == PreGapType::ExtractToFile && mDisk->track(0)->cueIndex(1).milliseconds() > 0);
}


/************************************************
 *
 ************************************************/
void Splitter::run()
{
    mCurrentTrack = 0;
    Decoder decoder;

    if (!decoder.open(mDisk->audioFileName()))
    {
        error(mDisk->track(0),
              tr("I can't read <b>%1</b>:<br>%2",
                 "Splitter error. %1 is a file name, %2 is a system error text.")
              .arg(mDisk->audioFileName())
              .arg(decoder.errorString()));
        return;
    }


    // Extract pregap to separate file ....................
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    if (mExtractPregapTrack)
    {
        mCurrentTrack = mDisk->preGapTrack();
        CueIndex start = mDisk->track(0)->cueIndex(0);
        CueIndex end   = mDisk->track(0)->cueIndex(1);
        QString outFileName = tmpFileName(mWorkDir, 0);

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

    for (int i=0; i<mDisk->count(); ++i)
    {
        mCurrentTrack = mDisk->track(i);
        QString outFileName = tmpFileName(mWorkDir, i + 1);

        CueIndex start, end;
        if (i==0 && mPreGapType == PreGapType::AddToFirstTrack)
            start = CueTime("00:00:00");
        else
            start = mDisk->track(i)->cueIndex(1);

        if (i<mDisk->count()-1)
            end = mDisk->track(i+1)->cueIndex(01);

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
const QList<const Track *> Splitter::tracks() const
{
    QList<const Track *> res;
    if (mExtractPregapTrack)
        res << mDisk->preGapTrack();

    for (int i=0; i<mDisk->count(); ++i)
        res << mDisk->track(i);

    return res;
}


/************************************************
 *
 ************************************************/
void Splitter::decoderProgress(int percent)
{
    emit trackProgress(mCurrentTrack, Track::Splitting, percent);
}


/************************************************
 *
 ************************************************/
QString Splitter::tmpFileName(const QString &dir, int trackNum)
{
    return QDir::toNativeSeparators(QString("%1/flacon_%2_[%3].wav")
                                    .arg(dir)
                                    .arg(trackNum, 2, 10, QChar('0'))
                                    .arg(QUuid::createUuid().toString().mid(1, 36)));
}




