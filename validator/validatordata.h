#ifndef VALIDATORDATA_H
#define VALIDATORDATA_H

/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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

#include <QList>
#include <disc.h>

class Track;
class Profile;

struct ValidatorTrack
{
    ValidatorTrack(const Track *track, const Profile *profile);

    bool operator==(const ValidatorTrack &other) const { return id == other.id; }

    uint64_t id     = 0;
    uint64_t diskId = 0;

    QString resultFilePath;
    int     resultFilePathIndex = -1;
};

struct ValidatorDisk
{
    ValidatorDisk(const Disc *disk, const Profile *profile);

    bool operator==(const ValidatorDisk &other) const { return id == other.id; }

    uint64_t id = 0;
    QString  artistTag;
    QString  albumTag;

    QList<ValidatorTrack> tracks;
};

struct ValidatorData
{

public:
    void clear();
    void fill(QList<::Disc *> disks, const Profile *profile);

    QList<ValidatorDisk> disks;

private:
    void fillResultFilePathIndex();
};

#endif // VALIDATORDATA_H
