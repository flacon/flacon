/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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

#ifndef CUEDATA_H
#define CUEDATA_H

#include <QVector>
#include <QMap>
#include "textcodec.h"
class QIODevice;

class CueData
{
public:
    CueData(const QString &fileName) noexcept(false);
    CueData(QIODevice *device) noexcept(false);

    QString fileName() const { return mFileName; }
    bool    isEmpty() const { return mTracks.isEmpty(); }

    TextCodec::BomCodec bomCodec() const { return mBomCodec; }

    using Tags = QMap<QByteArray, QByteArray>;

    const Tags         &globalTags() const { return mGlobalTags; }
    const QVector<Tags> tracks() const { return mTracks; }

    static constexpr auto INDEX_TAG     = "INDEX";
    static constexpr auto FILE_TAG      = "FILE";
    static constexpr auto TRACK_TAG     = "TRACK";
    static constexpr auto PERFORMER_TAG = "PERFORMER";
    static constexpr auto TITLE_TAG     = "TITLE";
    static constexpr auto FLAGS_TAG     = "FLAGS";

private:
    QString       mFileName;
    Tags          mGlobalTags;
    QVector<Tags> mTracks;

    TextCodec::BomCodec mBomCodec = TextCodec::BomCodec::Unknown;

    void                read(QIODevice *device);
    TextCodec::BomCodec detectBomCodec(QIODevice *file);
    void                parseLine(const QByteArray &line, QByteArray &tag, QByteArray &value, uint lineNum) const;
};

#endif // CUEDATA_H
