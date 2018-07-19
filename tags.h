/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2018
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


#ifndef TAGS_H
#define TAGS_H

#include <QByteArray>
#include <QString>
#include <QHash>

class QTextCodec;

enum class TagId
{
    Album,
    Catalog,
    CDTextfile,
    Comment,
    Date,
    Flags,
    Genre,
    ISRC,
    Performer,
    SongWriter,
    Title,
    DiscId,
    File,
    Disknum,
    CueFile,
    StartTrackNum
};


class TrackTags
{
public:
    TrackTags();
    TrackTags(const TrackTags &other);
    TrackTags &operator=(const TrackTags &other);
    virtual ~TrackTags();

    QString artist() const            { return tag(TagId::Performer); }
    void setArtist(const QString &value)  { setTag(TagId::Performer, value); }

    QString album() const             { return tag(TagId::Album); }
    void setAlbum(const QString &value)   { setTag(TagId::Album, value); }

    QString comment() const           { return tag(TagId::Comment) ;}
    void setComment(const QString &value)   { setTag(TagId::Comment, value); }

    QString title() const             { return tag(TagId::Title) ;}
    void setTitle(const QString &value)   { setTag(TagId::Title, value); }

    QString genre() const             { return tag(TagId::Genre) ;}
    void setGenre(const QString &value)   { setTag(TagId::Genre, value); }

    QString date() const              { return tag(TagId::Date) ;}
    void setDate(const QString &value)    { setTag(TagId::Date, value); }

    QString diskId() const              { return tag(TagId::DiscId) ;}

    QString tag(const TagId &tagID) const;
    QByteArray tagData(const TagId &tagID) const;
    void setTag(const TagId &tagID, const QString &value);
    void setTag(const TagId &tagID, const QByteArray &value);

    QString codecName() const;
    void setCodecName(const QString &value);

protected:


private:
    struct TagValue
    {
    public:
        TagValue(const QByteArray &val="", bool enc = false):
            value(val),
            encoded(enc)
        {
        }

        QByteArray value;
        bool encoded;
    };

    QHash<int, TagValue> mTags;
    QTextCodec *mTextCodec;
};



struct uchardet;

class UcharDet
{

public:
    UcharDet();
    ~UcharDet();

    void add(const TrackTags &track);
    UcharDet& operator<<(const TrackTags &track);


    QString textCodecName() const;
    QTextCodec* textCodec() const;

private:

    uchardet *mUchcharDet;
};

QTextCodec *determineTextCodec(const QVector<TrackTags*> tracks);

#endif // TAGS_H
