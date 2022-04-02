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

#include <QFile>

QString writeMetadataFile(const QString &path,const QString &fieldName, const QString &fieldValue)
{
    QString metadataFile = path + "." + fieldName;
    QFile file(metadataFile);
    if (file.open(QFile::WriteOnly)) {
        file.write(fieldValue.toUtf8());
    }
    file.close();
    return metadataFile;
}

QStringList FlacEncoder::programArgs() const
{
    QStringList args;
    args << programPath();

    args << "--force";  // Force overwriting of output files.
    args << "--silent"; // Suppress progress indicator

    /**
     * We ensure metadata for flac is utf-8 by using --tag-from-file option.
     * flac's internal automatical utf8 conversion may cause unwanted result.
     *
     * Using `--tag` should be avoided, use `--tag-from-file` instead.
     *
     * see https://github.com/flacon/flacon/issues/176
     */
    args << "--no-utf8-convert";

    // Settings .................................................
    // Compression parametr really looks like --compression-level-N
    args << QString("--compression-level-%1").arg(profile().value("Compression").toString());

    // Tags .....................................................
    if (!track().artist().isEmpty())
        args << "--tag" << QString("artist=%1").arg(track().artist());

    if (!track().album().isEmpty())
        args << "--tag-from-file" << QString("album=%1").arg(writeMetadataFile(outFile(), QString("album"), track().album()));

    if (!track().genre().isEmpty())
        args << "--tag-from-file" << QString("genre=%1").arg(writeMetadataFile(outFile(), QString("genre"), track().genre()));

    if (!track().date().isEmpty())
        args << "--tag-from-file" << QString("date=%1").arg(writeMetadataFile(outFile(), QString("date"), track().date()));

    if (!track().title().isEmpty())
        args << "--tag-from-file" << QString("title=%1").arg(writeMetadataFile(outFile(), QString("title"), track().title()));

    if (!track().tag(TagId::AlbumArtist).isEmpty())
        args << "--tag-from-file" << QString("albumartist=%1").arg(writeMetadataFile(outFile(), QString("albumartist"), track().tag(TagId::AlbumArtist)));

    if (!track().comment().isEmpty())
        args << "--tag-from-file" << QString("comment=%1").arg(writeMetadataFile(outFile(), QString("comment"), track().comment()));

    if (!track().discId().isEmpty())
        args << "--tag-from-file" << QString("discId=%1").arg(writeMetadataFile(outFile(), QString("discId"), track().discId()));

    args << "--tag-from-file" << QString("tracknumber=%1").arg(writeMetadataFile(outFile(), QString("tracknumber"), QString(track().trackNum())));
    args << "--tag-from-file" << QString("totaltracks=%1").arg(writeMetadataFile(outFile(), QString("totaltracks"), QString(track().trackCount())));
    args << "--tag-from-file" << QString("tracktotal=%1").arg(writeMetadataFile(outFile(), QString("tracktotal"), QString(track().trackCount())));

    args << "--tag-from-file" << QString("disc=%1").arg(writeMetadataFile(outFile(), QString("disc"), QString(track().discNum())));
    args << "--tag-from-file" << QString("discnumber=%1").arg(writeMetadataFile(outFile(), QString("discnumber"), QString(track().discNum())));
    args << "--tag-from-file" << QString("disctotal=%1").arg(writeMetadataFile(outFile(), QString("disctotal"), QString(track().discCount())));

    if (!coverFile().isEmpty()) {
        args << QString("--picture=%1").arg(coverFile());
    }

    if (profile().isEmbedCue()) {
        args << "--tag-from-file" << QString("cuesheet=%1").arg(writeMetadataFile(outFile(), QString("cuesheet"), embeddedCue()));
    }

    args << "-";
    args << "-o" << outFile();
    return args;
}
