/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#ifndef CUECREATOR_H
#define CUECREATOR_H

#include "types.h"
#include <QFile>
#include <QString>
#include "track.h"
#include "profiles.h"

class Disc;
class Track;

namespace Conv {

class CueCreator
{
public:
    explicit CueCreator(const Profile &profile, const Disc *disk, PreGapType preGapType);

    void    write(QIODevice *out);
    QString writeToFile(const QString &fileTemplate);

private:
    const Disc      *mDisk;
    const Profile    mProfile;
    const PreGapType mPreGapType;
    QString          mCommonGenre;
    QString          mCommonDate;
    QString          mCommonPerformer;
    QString          mCommonSongWriter;

    void writeLine(QIODevice *out, const QString &text) const;
    void writeTag(QIODevice *out, const QString &format, const QString &value) const;
    void writeTrackTags(QIODevice *out, const Track *track) const;

    QString getCommonTag(const Track::Getter &func);
};

} // namepace
#endif // CUECREATOR_H
