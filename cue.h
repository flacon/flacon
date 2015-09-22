/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2015
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


#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QList>
#include "tagset.h"

class QFile;

class CueIndex
{
public:
    CueIndex(const QString &str = "");

    bool isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueIndex operator-(const CueIndex &other) const;
    bool operator==(const CueIndex &other) const;
    bool operator!=(const CueIndex &other) const;

private:
    bool mNull;
    int mCdValue;
    int mHiValue;

    bool parse(const QString &str);
};


class CueTagSet: public TagSet
{
public:
    explicit CueTagSet(const QString &uri);
    CueTagSet(const CueTagSet &other);

    QString cueFileName() const;
    QString fileTag() const;
    CueIndex index(int track, int indexNum) const;
    bool isMultiFileCue() const;
    int diskNumInCue() const;
};


class CueReader
{
public:
    explicit CueReader(const QString &fileName);

    QString fileName() const { return mFileName; }
    CueTagSet disk(int index) const { return mDisks.at(index); }
    int diskCount() const { return mDisks.count(); }
    bool isMultiFileCue() const { return mDisks.count() > 1; }
    bool isValid() const { return mValid; }
    QString errorString() const { return mErrorString; }
private:
    QString mFileName;
    QList<CueTagSet> mDisks;

    QString mCodecName;
    QByteArray mPerformer;
    QByteArray mAlbum;
    QByteArray mGenre;
    QByteArray mDate;
    QByteArray mComment;
    QByteArray mSongwriter;
    QByteArray mDiskId;
    QByteArray mCatalog;
    QByteArray mCdTextFile;

    bool parse(QFile &file);
    bool parseOneDiskTags(QFile &file, CueTagSet *tags);
    bool mValid;
    QString mErrorString;
};

#endif // CUE_H
