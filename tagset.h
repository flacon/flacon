/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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


#ifndef TAGSET_H
#define TAGSET_H

#include <QtCore/QSharedDataPointer>
#include <QString>
#include <QList>
#include <QHash>
#include <QAction>

class TagSetData;
class QTextCodec;

#define TAG_ALBUM       "ALBUM"
#define TAG_CATALOG     "CATALOG"
#define TAG_CDTEXTFILE  "CDTEXTFILE"
#define TAG_COMMENT     "COMMENT"
#define TAG_DATE        "DATE"
#define TAG_FLAGS       "FLAGS"
#define TAG_GENRE       "GENRE"
#define TAG_ISRC        "ISRC"
#define TAG_PERFORMER   "PERFORMER"
#define TAG_SONGWRITER  "SONGWRITER"
#define TAG_TITLE       "TITLE"
#define TAG_DISCID      "DISCID"
#define TAG_FILE        "FILE"
#define TAG_DISKNUM     "DISKNUM"
#define TAG_CUE_FILE    "CUE_FILE"
#define START_TRACK_NUM "START_TRACK_NUM"

#define CODEC_AUTODETECT "AUTODETECT"

class TagSet
{
public:
    explicit TagSet(const QString &uri);
    TagSet(const TagSet &other);
    ~TagSet();

    TagSet &operator=(const TagSet &other);

    QString uri() const;
    QString title() const;
    void setTitle(const QString &title);
    void setTitle(const QByteArray &title, bool encoded);

    int tracksCount() const;

    QString textCodecName() const;
    void setTextCodecName(const QString codecName);


    QString trackTag(int track, const QString &tagName) const;
    void setTrackTag(int track, const QString &tagName, const QString &value);
    void setTrackTag(int track, const QString &tagName, const QByteArray &value, bool encoded);


    QString diskTag(const QString &tagName) const;
    void setDiskTag(const QString &tagName, const QString &value);
    void setDiskTag(const QString &tagName, const QByteArray &value, bool encoded);

    int distance(const TagSet &other);
    int distance(const TagSet *other);

private:
    QSharedDataPointer<TagSetData> d;
};

class Disk;

class TagSetAction: public QAction
{
    Q_OBJECT
public:
    TagSetAction(QObject* parent, Disk *disk, TagSet *tagSet);
    TagSetAction(const QString &text, QObject* parent, Disk *disk, TagSet *tagSet);
    TagSetAction(const QIcon &icon, const QString &text, QObject* parent, Disk *disk, TagSet *tagSet);

    TagSet *tagSet() const { return mTagSet; }
    Disk *disk() const { return mDisk; }
private:
    Disk *mDisk;
    TagSet *mTagSet;
};


QDebug operator<<(QDebug dbg, const TagSet &ts);

#endif // TAGSET_H
