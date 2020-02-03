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


#ifndef CUECREATOR_H
#define CUECREATOR_H

#include "types.h"
#include <QFile>
#include <QString>
#include "track.h"


class Disc;
class Track;

class CueCreator
{
public:
    explicit CueCreator(const Disc *disk, PreGapType preGapType, const QString &fileTemplate);
    bool write();

    QString errorString() const { return mErrorString; }

    QTextCodec *textCodec() const { return mTextCodec; }
    void setTextCodecName(const QString &codecName);
    void setTextCodecMib(int mib);

private:
    const Disc *mDisk;
    const PreGapType mPreGapType;
    QFile mFile;
    QString mErrorString;
    QTextCodec *mTextCodec;
    Track mGlobalTags;

    void initGlobalTags();
    void writeLine(const QString &text);
    void writeGlobalTag(const QString &format, TagId tagId);
    void writeTrackTag(const Track *track, const QString &prefix, TagId tagId);

};

#endif // CUECREATOR_H
