/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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

#include "in_flac.h"
#include <QDebug>
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>

REGISTER_INPUT_FORMAT(Format_Flac)

/************************************************
 *
 ************************************************/
QStringList Format_Flac::decoderArgs(const QString &fileName) const
{
    QStringList args;
    args << "-c";
    args << "-d";
    args << "-s";
    args << fileName;
    args << "-";

    return args;
}

/************************************************
 *
 ************************************************/
QByteArray Format_Flac::readEmbeddedCue(const QString &fileName) const
{
    TagLib::FLAC::File file(fileName.toLocal8Bit().data());
    if (!file.isOpen()) {
        return QByteArray();
    }

    TagLib::Ogg::XiphComment *comment = file.xiphComment(false);
    if (!comment) {
        return QByteArray();
    }

    const TagLib::Ogg::FieldListMap &tags = comment->fieldListMap();

    static constexpr auto CUE_SHEET_TAGS = { "CUESHEET", "cuesheet" };

    for (auto key : CUE_SHEET_TAGS) {
        if (tags.contains(key)) {
            return QByteArray(tags[key].front().toCString(true));
        }
    }

    return QByteArray();
}
