/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include "alacencoder.h"

QStringList AlacEncoder::programArgs() const
{
    QStringList args;
    args << programPath();

    args << "--quiet"; // Produce no output to stderr

    // Settings .................................................
    if (profile().value("Compression").toInt() == 0) {
        args << QString("--fast");
    }

    // Tags .....................................................
    if (!track().artist().isEmpty())
        args << QString("--artist=%1").arg(track().artist());

    if (!track().album().isEmpty())
        args << QString("--album=%1").arg(track().album());

    if (!track().genre().isEmpty())
        args << QString("--genre=%1").arg(track().genre());

    if (!track().date().isEmpty())
        args << QString("--year=%1").arg(track().date());

    if (!track().title().isEmpty())
        args << QString("--title=%1").arg(track().title());

    if (!track().tag(TagId::AlbumArtist).isEmpty())
        args << QString("--albumArtist=%1").arg(track().tag(TagId::AlbumArtist));

    if (!track().comment().isEmpty())
        args << QString("--comment=%1").arg(track().comment());

    args << QString("--track=%1/%2").arg(track().trackNum()).arg(track().trackCount());
    args << QString("--disc=%1/%2").arg(track().discNum()).arg(track().discCount());

    if (!coverFile().isEmpty()) {
        args << QString("--cover=%1").arg(coverFile());
    }

    args << "-";
    args << outFile();
    return args;
}
