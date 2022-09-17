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

#ifndef CONVERTERTYPES_H
#define CONVERTERTYPES_H

#include <QtGlobal>
#include "../types.h"
#include "../track.h"
#include "../inputaudiofile.h"

class Track;
class OutFormat;
class Profile;

namespace Conv {

class ConvTrack : public Track
{
public:
    ConvTrack()                       = default;
    ConvTrack(const ConvTrack &other) = default;
    explicit ConvTrack(const Track &other);

    ConvTrack &operator=(const ConvTrack &other) = default;

    bool isPregap() const { return mPregap; }
    void setPregap(bool value) { mPregap = value; }

private:
    bool mPregap = false;
};

using ConvTracks = QList<ConvTrack>;

} // namespace

Q_DECLARE_METATYPE(Conv::ConvTrack)

#endif // CONVERTERTYPES_H
