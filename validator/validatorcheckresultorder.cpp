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

#include "validatorcheckresultorder.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "track.h"
#include "disc.h"
#include "profiles.h"
#include <QApplication>

ValidatorCheckResultOrder::ValidatorCheckResultOrder(const QList<const Disk *> disks, const Profile *profile) :
    mDisks(disks),
    mProfile(profile)
{
}

void ValidatorCheckResultOrder::clear()
{
    mWarnings.clear();
    mErrors.clear();
}

bool ValidatorCheckResultOrder::validate(const Disk *disk)
{
    ValidatorResultFiles files(mDisks, mProfile);
    files.sortByPath();

    QMap<QString, ValidatorResultFiles> byDir = files.splitByDirectory();

    bool res = true;
    for (auto it = byDir.begin(); it != byDir.end(); ++it) {
        res = validateDir(it.key(), disk, it.value()) && res;
    }

    return res;
}

bool ValidatorCheckResultOrder::validateDir(const QString &, const Disk *disk, const ValidatorResultFiles &files)
{
    QStringList ourFiles;
    for (auto f : files) {
        if (f.track->disc() == disk) {
            ourFiles << f.file.fileName();
        }
    }

    ValidatorResultFiles::UnaryPred sameDisk = [disk](const ValidatorResultFile &f) -> bool { return f.track->disc() == disk; };

    int firstIndex = files.indexOf(sameDisk);
    if (firstIndex < 0) {
        return true;
    }

    int lastIndex = files.lastIndexOf(sameDisk);

    bool res = true;
    for (int i = firstIndex; i <= lastIndex; ++i) {
        const Track *track = files.at(i).track;
        if (track->disc() != disk) {

            if (ourFiles.contains(files.at(i).file.fileName())) {
                continue; // The situation when files overwrite each other is handled in another check
            }

            res = false;
            mErrors << tr("The output files of the disc are mixed with the files of disc %1.\n"
                          "You could change the \"Start num\" for one of them.",
                          "Error message, %1 is the disk description, artist and album for the disc, respectively")
                               .arg(diskString(track->disc()));
        }
    }

    return res;
}

QString ValidatorCheckResultOrder::diskString(const Disk *disk)
{
    return QStringLiteral("%1 \"%2 - %3\"").arg(mDisks.indexOf(disk) + 1).arg(disk->discTag(TagId::Artist), disk->discTag(TagId::Album));
}
