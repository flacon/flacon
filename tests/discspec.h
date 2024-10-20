/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#ifndef DISCSPEC_H
#define DISCSPEC_H
#include <QSettings>

class Disc;
class Track;

namespace Tests {

class DiscSpec
{
public:
    DiscSpec(const QString &fileName);

    QString fileName() const { return mFileName; }

    static void write(const Disc &disk, const QString &fileName);

    void verify(const Disc &disk) const;

private:
    QString mFileName;

    void verifyDiskTags(const Disc &disk, const QJsonObject &json) const;
    void verifyTrackTags(const Track *track, const QJsonObject &json) const;
    int  strToDuration(const QString &str) const;
};

}

#endif // DISCSPEC_H
