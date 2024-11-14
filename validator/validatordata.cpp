#include "validatordata.h"
#include "profiles.h"

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

static uint64_t genId()
{
    static uint64_t id = 0;
    return ++id;
}

ValidatorTrack::ValidatorTrack(const Track *track, const Profile *profile)
{
    id             = genId();
    resultFilePath = profile->resultFilePath(track);
}

ValidatorDisk::ValidatorDisk(const Disc *disk, const Profile *profile)
{
    id        = genId();
    albumTag  = disk->albumTag();
    artistTag = disk->isEmpty() ? "" : disk->tracks().first()->artistTag();

    for (const Track *t : disk->tracks()) {
        ValidatorTrack track = ValidatorTrack(t, profile);
        track.diskId         = id;
        tracks << track;
    }
}

void ValidatorData::clear()
{
    disks.clear();
}

void ValidatorData::fill(QList<Disc *> disks, const Profile *profile)
{
    for (const ::Disc *disk : disks) {
        this->disks << ValidatorDisk(disk, profile);
    }

    fillResultFilePathIndex();
}

void ValidatorData::fillResultFilePathIndex()
{
    QMap<QString, QList<ValidatorTrack *>> paths;

    for (ValidatorDisk &d : this->disks) {
        for (ValidatorTrack &t : d.tracks) {
            paths[t.resultFilePath].append(&t);
        }
    }

    int n = -1;
    for (auto it = paths.cbegin(), end = paths.cend(); it != end; ++it) {
        n++;
        for (ValidatorTrack *t : it.value()) {
            t->resultFilePathIndex = n;
        }
    }
}
