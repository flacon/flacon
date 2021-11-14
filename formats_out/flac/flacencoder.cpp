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

#include "flacencoder.h"

QStringList FlacEncoder::programArgs() const
{
    QStringList args;
    args << programPath();

    args << "--force";  // Force overwriting of output files.
    args << "--silent"; // Suppress progress indicator

    // Settings .................................................
    // Compression parametr really looks like --compression-level-N
    args << QString("--compression-level-%1").arg(profile().value("Compression").toString());

    // Tags .....................................................
    if (!track().artist().isEmpty())
        args << "--tag" << QString("artist=%1").arg(track().artist());

    if (!track().album().isEmpty())
        args << "--tag" << QString("album=%1").arg(track().album());

    if (!track().genre().isEmpty())
        args << "--tag" << QString("genre=%1").arg(track().genre());

    if (!track().date().isEmpty())
        args << "--tag" << QString("date=%1").arg(track().date());

    if (!track().title().isEmpty())
        args << "--tag" << QString("title=%1").arg(track().title());

    if (!track().tag(TagId::AlbumArtist).isEmpty())
        args << "--tag" << QString("albumartist=%1").arg(track().tag(TagId::AlbumArtist));

    if (!track().comment().isEmpty())
        args << "--tag" << QString("comment=%1").arg(track().comment());

    if (!track().discId().isEmpty())
        args << "--tag" << QString("discId=%1").arg(track().discId());

    args << "--tag" << QString("tracknumber=%1").arg(track().trackNum());
    args << "--tag" << QString("totaltracks=%1").arg(track().trackCount());
    args << "--tag" << QString("tracktotal=%1").arg(track().trackCount());

    args << "--tag" << QString("disc=%1").arg(track().discNum());
    args << "--tag" << QString("discnumber=%1").arg(track().discNum());
    args << "--tag" << QString("disctotal=%1").arg(track().discCount());

    if (!coverFile().isEmpty()) {
        args << QString("--picture=%1").arg(coverFile());
    }

    if (profile().isEmbedCue()) {
        args << "--tag" << QString("cuesheet=%1").arg(embeddedCue());
    }

    args << "-";
    args << "-o" << outFile();
    return args;
}
