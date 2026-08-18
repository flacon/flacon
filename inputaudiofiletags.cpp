/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2026
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

#include "inputaudiofiletags.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <QDebug>

static QByteArray getString(const TagLib::PropertyMap properties, const char *key)
{
    if (properties.contains(key) && !properties[key].isEmpty()) {
        TagLib::ByteVector bv = properties[key].front().data(TagLib::String::Latin1);
        return QByteArray(bv.data(), static_cast<int>(bv.size()));
    }
    return QByteArray();
}

static QByteArray getString(const TagLib::PropertyMap properties, const std::vector<const char *> keys)
{
    QByteArray res;
    for (const char *key : keys) {
        res = getString(properties, key);
        if (!res.isEmpty()) {
            return res;
        }
    }
    return {};
}

static int bytesToInt(const QByteArray &data, int defaultValue)
{
    bool ok;
    int  res = data.toInt(&ok);
    return ok ? res : defaultValue;
}

#if 0
namespace {
void dumpProperties(const TagLib::PropertyMap &properties)
{
    qDebug() << "=== TAGLIB PROPERTY MAP ===";

    for (const auto &item : properties) {
        QString key = QString::fromUtf8(item.first.toCString(true));

        QStringList values;
        for (const auto &val : item.second) {
            values.append(QString::fromUtf8(val.toCString(true)));
        }

        qDebug().noquote() << key << ":" << values;
    }
}
}
#endif

void InputAudioFileTags::load(const QString &filePath)
{
#ifdef Q_OS_WIN
    TagLib::FileRef f(filePath.toStdWString().c_str());
#else
    TagLib::FileRef f(filePath.toLocal8Bit().constData());
#endif
    if (f.isNull() || !f.file() || !f.file()->isValid()) {
        return;
    }

    TagLib::PropertyMap properties = f.file()->properties();
    // dumpProperties(properties);

    mAlbumTags[AlbumTags::TagId::Album]          = getString(properties, "ALBUM");
    mAlbumTags[AlbumTags::TagId::Catalog]        = getString(properties, { "CATALOGNUMBER", "CATALOG" });
    mAlbumTags[AlbumTags::TagId::DiscId]         = getString(properties, { "DISCID", "MUSICBRAINZ_DISCID" });
    mAlbumTags[AlbumTags::TagId::AlbumPerformer] = getString(properties, { "ALBUMARTIST", "ALBUM ARTIST" });

    {
        // Disk number (can be stored as "1" or "1/2")
        QList<QByteArray> val = getString(properties, "DISCNUMBER").split('/');
        if (val.count() > 1) {
            mDiscNum   = bytesToInt(val[0], 1);
            mDiscCount = bytesToInt(val[1], 1);
        }
        else {
            mDiscNum = bytesToInt(val[0], 1);
        }
    }

    {
        QByteArray val = getString(properties, { "TOTALTRACKS", "TRACKTOTAL" });
        mTrackCount    = bytesToInt(val, 1);
    }

    mTrackTags[TrackTags::TagId::Comment]    = getString(properties, "COMMENT");
    mTrackTags[TrackTags::TagId::Date]       = getString(properties, { "DATE", "YEAR" });
    mTrackTags[TrackTags::TagId::Genre]      = getString(properties, "GENRE");
    mTrackTags[TrackTags::TagId::Isrc]       = getString(properties, "ISRC");
    mTrackTags[TrackTags::TagId::Title]      = getString(properties, "TITLE");
    mTrackTags[TrackTags::TagId::Performer]  = getString(properties, "ARTIST");
    mTrackTags[TrackTags::TagId::SongWriter] = getString(properties, "COMPOSER");

    {
        QByteArray val = getString(properties, "TRACKNUMBER");
        mTrackNum      = bytesToInt(val, 1);
    }
}

AlbumTags InputAudioFileTags::albumTags(const TextCodec &textCodec) const
{
    AlbumTags res;
    res.setDiscNum(mDiscNum);
    res.setDiscCount(mDiscCount);
    res.setTrackCount(mTrackCount);

    for (AlbumTags::TagId tagId : mAlbumTags.keys()) {
        res.setTag(tagId, textCodec.decode(mAlbumTags.value(tagId)));
    }
    return res;
}

TrackTags InputAudioFileTags::trackTags(const TextCodec &textCodec) const
{
    TrackTags res;
    res.setTrackNum(mTrackNum);

    for (TrackTags::TagId tagId : mTrackTags.keys()) {
        res.setTag(tagId, textCodec.decode(mTrackTags.value(tagId)));
    }
    return res;
}
