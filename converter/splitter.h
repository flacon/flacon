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


#ifndef SPLITTER_H
#define SPLITTER_H

#include "worker.h"
#include "types.h"

class Disk;
class Track;
class Project;


class Splitter: public Worker
{
    Q_OBJECT
public:
    Splitter(const Disk *disk, const QString &tmpFilePrefix, PreGapType preGapType, QObject *parent = NULL);

public slots:
    void run() override;

public:
    const QList<const Track*> tracks() const;

private slots:
    void decoderProgress(int percent);

private:
    const Disk *mDisk;
    const QString mTmpFilePrefix;
    const PreGapType mPreGapType;
    const Track *mCurrentTrack;
    bool mExtractPregapTrack;
};



#endif // SPLITTER_H
