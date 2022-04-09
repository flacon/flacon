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

#ifndef COVERIMAGE_H
#define COVERIMAGE_H

#include <QString>
#include <QSize>

class CoverImage
{
public:
    CoverImage()                        = default;
    CoverImage(const CoverImage &other) = default;
    CoverImage &operator=(const CoverImage &other) = default;

    explicit CoverImage(const QString &origFilePath, uint size = 0);

    QString mimeType() const { return mMimeType; }
    QSize   size() const { return mSize; }
    int     depth() const { return mDepth; }

    const QByteArray &data() const { return mData; }

    void saveAs(const QString &filePath) const;

    bool isEmpty() const { return mData.isEmpty(); }

private:
    QString    mMimeType;
    QByteArray mData;
    QSize      mSize;
    int        mDepth = 0;
};

#endif // COVERIMAGE_H
