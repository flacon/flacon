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

#ifndef COPYCOVER_H
#define COPYCOVER_H

#include <QString>
class Disc;

class CopyCover
{
public:
    CopyCover(const Disc *disc, const QString &outDir, const QString &outBaseName, int newSize);
    bool run();

    QString errorString() const { return mErrorString; }

private:
    const Disc *  mDisc;
    const int     mSize;
    const QString mDir;
    const QString mBaseName;
    QString       mErrorString;

    bool copyImage(const QString &outFileName);
    bool resizeImage(const QString &outFileName);
};

#endif // COPYCOVER_H
