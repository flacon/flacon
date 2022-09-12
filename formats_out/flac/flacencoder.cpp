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
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <QFile>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "FlacEncoder")
}

QString writeMetadataFile(const QString &path, const QString &fieldName, const QString &fieldValue)
{
    QString metadataFile = path + "." + fieldName;
    QFile   file(metadataFile);
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

    // Settings .................................................
    // Compression parametr really looks like --compression-level-N
    args << QString("--compression-level-%1").arg(profile().value("Compression").toString());

    args << "-";
    args << "-o" << outFile();
    return args;
}

void FlacEncoder::writeMetadata(const QString &filePath) const
{
    TagLib::FLAC::File file(filePath.toLocal8Bit(), false);

    if (!file.isValid()) {
        qCWarning(LOG) << Q_FUNC_INFO << "file is invalid";
        throw FlaconError("Can't open file");
    }

    TagLib::Ogg::XiphComment *comments = file.xiphComment(true);

    auto writeStrTag = [comments](const QString &tagName, const QString &value) {
        if (!value.isEmpty()) {
            comments->addField(tagName.toStdString(), TagLib::String(value.toStdString(), TagLib::String::UTF8), true);
        }
    };

    auto writeIntTag = [comments](const QString &tagName, int value) {
        comments->addField(tagName.toStdString(), QString::number(value).toStdString(), true);
    };

    writeStrTag("ARTIST", track().artist());
    writeStrTag("ALBUM", track().album());
    writeStrTag("GENRE", track().genre());
    writeStrTag("DATE", track().date());
    writeStrTag("TITLE", track().title());
    writeStrTag("ALBUMARTIST", track().tag(TagId::AlbumArtist));
    writeStrTag("COMMENT", track().comment());
    writeStrTag("DISCID", track().discId());

    writeIntTag("TRACKNUMBER", track().trackNum());
    writeIntTag("TOTALTRACKS", track().trackCount());
    writeIntTag("TRACKTOTAL", track().trackCount());

    writeIntTag("DISC", track().discNum());
    writeIntTag("DISCNUMBER", track().discNum());
    writeIntTag("DISCTOTAL", track().discCount());

    if (profile().isEmbedCue()) {
        writeStrTag("CUESHEET", embeddedCue());
    }

    if (!coverImage().isEmpty()) {
        const CoverImage  &img = coverImage();
        TagLib::ByteVector dt(img.data().data(), img.data().size());

        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setData(dt);
        pic->setMimeType(img.mimeType().toStdString());
        pic->setWidth(img.size().width());
        pic->setHeight(img.size().height());
        pic->setColorDepth(img.depth());

        file.addPicture(pic);
    }

    if (!file.save()) {
        throw FlaconError("Can't save file");
    }
}
