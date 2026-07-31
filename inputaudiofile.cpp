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

#include "inputaudiofile.h"
#include "decoder.h"
#include "formats_in/informat.h"
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace {
Q_LOGGING_CATEGORY(LOG, "InputAudioFile")
}

InputAudioFile::Data::Data(const InputAudioFile::Data &other) :
    QSharedData(other),
    mFilePath(other.mFilePath),
    mFileName(other.mFileName),
    mErrorString(other.mErrorString),
    mFormat(other.mFormat),
    mSampleRate(other.mSampleRate),
    mBitsPerSample(other.mBitsPerSample),
    mDuration(other.mDuration),
    mValid(other.mValid),
    mCdQuality(other.mCdQuality),
    mChannelsCount(other.mChannelsCount)
{
}

void InputAudioFile::Data::load(const QString &filePath)
{
    mFilePath = filePath;
    qCDebug(LOG) << "load" << filePath;

    if (mFilePath.isEmpty()) {
        mErrorString = QObject::tr("The audio file name is not set.");
        qCDebug(LOG) << mErrorString;
        mValid = false;
        mFileName.clear();
        return;
    }

    QFileInfo fi(filePath);
    mFileName     = fi.fileName();
    mTagsId.uri   = mFilePath;
    mTagsId.title = QObject::tr("Tags from %1", "The title for the tags from the audio file. %1 - is an audio-file name.").arg(mFileName);

    if (!fi.exists()) {
        mErrorString = QObject::tr("The audio file does not exist.");
        qCDebug(LOG) << mErrorString;
        mValid = false;
        return;
    }

    try {
        Conv::Decoder dec;
        dec.open(mFilePath);
        mFormat = dec.audioFormat();

        mSampleRate    = dec.wavHeader().sampleRate();
        mBitsPerSample = dec.wavHeader().bitsPerSample();
        mCdQuality     = dec.wavHeader().isCdQuality();
        mDuration      = dec.duration();
        mChannelsCount = dec.wavHeader().numChannels();

        mValid = true;

        // clang-format off
        qCDebug(LOG) << "Audio is loaded: "
                        "format ="         << mFormat->name() <<
                        "mDuration ="      << mDuration <<
                        "mCdQuality ="     << mCdQuality <<
                        "mSampleRate ="    << mSampleRate <<
                        "mBitsPerSample =" << mBitsPerSample;
        // clang-format on
    }
    catch (FlaconError &err) {
        mErrorString = err.what();
        qCDebug(LOG) << mErrorString;
        mValid = false;
    }

    loadTags(mFilePath);
    // mTags.resize(1);
    // mTags.tracks()[0].setArtist("@@@INXS");
    // mTags.tracks()[0].setTitle("@@@Guns In The Sky");
    // mTags.tracks()[0].setDate("@@@1987");
    // mTags.tracks()[0].setGenre("@@@Rock");
    // mTags.setDiscId("8D095A0C");
    // mTags.tracks()[0].setTrackNum(2);
    // mTags.setAlbum("@@@Kick");
}

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

void InputAudioFile::Data::loadTags(const QString &filePath)
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

    // auto getStr = [&](const char *key[]) -> QByteArray {
    //     if (properties.contains(key) && !properties[key].isEmpty()) {
    //         TagLib::ByteVector bv = properties[key].front().data(TagLib::String::Latin1);
    //         return QByteArray(bv.data(), static_cast<int>(bv.size()));
    //     }
    //     return QByteArray();
    // };

    mAlbumTags[AlbumTags::TagId::Album]          = getString(properties, "ALBUM");
    mAlbumTags[AlbumTags::TagId::Catalog]        = getString(properties, { "CATALOGNUMBER", "CATALOG" });
    mAlbumTags[AlbumTags::TagId::DiscId]         = getString(properties, { "DISCID", "MUSICBRAINZ_DISCID" });
    mAlbumTags[AlbumTags::TagId::AlbumPerformer] = getString(properties, { "ALBUMARTIST", "ALBUM ARTIST" });

    // QByteArray discNum = getString(properties, "DISCNUMBER");
    // Disk number (can be stored as "1" or "1/2")
    // mAlbumTags[AlbumTags::TagId::DiscId] = getString(properties, "DISCNUMBER");

    mTrackTags[TrackTags::TagId::Comment]    = getString(properties, "COMMENT");
    mTrackTags[TrackTags::TagId::Date]       = getString(properties, { "DATE", "YEAR" });
    mTrackTags[TrackTags::TagId::Genre]      = getString(properties, "GENRE");
    mTrackTags[TrackTags::TagId::Isrc]       = getString(properties, "ISRC");
    mTrackTags[TrackTags::TagId::Title]      = getString(properties, "TITLE");
    mTrackTags[TrackTags::TagId::Performer]  = getString(properties, "ARTIST");
    mTrackTags[TrackTags::TagId::SongWriter] = getString(properties, "COMPOSER");
}

bool InputAudioFile::operator==(const InputAudioFile &other) const
{
    return mData->mFilePath == other.mData->mFilePath;
}

InputAudioFile::InputAudioFile() :
    mData(new Data())
{
}

InputAudioFile::InputAudioFile(const QString &fileName) :
    mData(new Data())
{
    mData->load(fileName);
}

InputAudioFile::InputAudioFile(const InputAudioFile &other) :
    mData(other.mData)
{
}

InputAudioFile &InputAudioFile::operator=(const InputAudioFile &other)
{
    mData = other.mData;
    return *this;
}

Tags InputAudioFile::tags(const TextCodec &textCodec) const
{
    Tags aTags;
    for (AlbumTags::TagId tagId : mData->mAlbumTags.keys()) {
        aTags.setTag(tagId, textCodec.decode(mData->mAlbumTags.value(tagId)));
    }

    TrackTags tTags;
    for (TrackTags::TagId tagId : mData->mTrackTags.keys()) {
        tTags.setTag(tagId, textCodec.decode(mData->mTrackTags.value(tagId)));
    }
    aTags.tracks().append(tTags);

    return aTags;
}
