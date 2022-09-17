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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include "types.h"
#include "tags.h"
#include "inputaudiofile.h"
#include <QDebug>

class Disc;

class Track
{
    friend class Disc;

public:
    Track() = default;
    Track(Disc *disc, int index);
    Track(const Track &other)            = default;
    Track &operator=(const Track &other) = default;
    Track(Disc *disc, int index, const TrackTags &tags);

    ~Track() = default;

    const InputAudioFile &audioFile() const { return mAudiofile; }
    void                  setAudioFile(const InputAudioFile &file);
    QString               audioFileName() const { return mAudiofile.fileName(); }

    const TrackTags &tags() const { return mTags; }
    void             setTags(const TrackTags &tags);

    QString    tag(const TagId &tagId) const;
    QByteArray tagData(const TagId &tagId) const;
    TagValue   tagValue(TagId tagId) const;
    void       setTag(const TagId &tagId, const QString &value);
    void       setTag(const TagId &tagId, const QByteArray &value);
    void       setTag(TagId tagId, const TagValue &value);

    QString           codecName() const;
    void              setCodecName(const QString &value);
    const QTextCodec *codec() const { return mTags.codec(); }

    bool operator==(const Track &other) const;

    QString artist() const { return tag(TagId::Artist); }
    void    setArtist(const QString &value) { setTag(TagId::Artist, value); }

    QString album() const { return tag(TagId::Album); }
    void    setAlbum(const QString &value) { setTag(TagId::Album, value); }

    QString albumArtist() const { return tag(TagId::AlbumArtist); }
    void    setAlbumArtist(const QString &value) { setTag(TagId::AlbumArtist, value); }

    QString comment() const { return tag(TagId::Comment); }
    void    setComment(const QString &value) { setTag(TagId::Comment, value); }

    QString title() const { return tag(TagId::Title); }
    void    setTitle(const QString &value) { setTag(TagId::Title, value); }

    QString genre() const { return tag(TagId::Genre); }
    void    setGenre(const QString &value) { setTag(TagId::Genre, value); }

    QString date() const { return tag(TagId::Date); }
    void    setDate(const QString &value) { setTag(TagId::Date, value); }

    QString discId() const { return tag(TagId::DiscId); }

    TrackNum trackNum() const;
    void     setTrackNum(TrackNum value);

    TrackNum trackCount() const;
    void     setTrackCount(TrackNum value);

    DiscNum discNum() const;
    void    setDiscNum(DiscNum value);

    DiscNum discCount() const;
    void    setDiscCount(DiscNum value);

    QString resultFileName() const;
    QString resultFilePath() const;

    Duration duration() const;

    CueIndex cueIndex(int indexNum) const;

    Disc *disc() const { return mDisc; }

    int index() const { return mIndex; }

    bool preEmphased() const;

private:
    Disc          *mDisc = nullptr;
    TrackTags      mTags;
    int            mIndex = -1;
    InputAudioFile mAudiofile;

    QString calcResultFilePath() const;
    QString safeFilePathLen(const QString &path) const;
};

class Tracks : public QVector<Track>
{
public:
    Tracks();
    explicit Tracks(int size);
    explicit Tracks(const QList<Track *> &other);
    Tracks(const Tracks &other);
    Tracks &operator=(const Tracks &other);
    virtual ~Tracks();

    QString uri() const { return mUri; }
    void    setUri(const QString &value) { mUri = value; }

    QString title() const;
    void    setTitle(const QByteArray &value);
    void    setTitle(const QString &value);

private:
    QString  mUri;
    TagValue mTitle;
};

using TrackPtrList = QList<Track *>;

class UcharDet
{

public:
    UcharDet();
    UcharDet(const UcharDet &)            = delete;
    UcharDet &operator=(const UcharDet &) = delete;
    ~UcharDet();

    void      add(const Track &track);
    UcharDet &operator<<(const Track &track);
    UcharDet &operator<<(const TrackTags &track);

    QString     textCodecName() const;
    QTextCodec *textCodec() const;

private:
    struct Data;
    Data *mData;
};

QTextCodec *determineTextCodec(const QVector<Track *> &tracks);

QDebug operator<<(QDebug debug, const Track &track);

#endif // TRACK_H
