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
#include <QMap>
#include "types.h"
#include "textcodec.h"
#include "tags.h"

class CueData;
class InputAudioFile;

class Cue
{
public:
    class Track
    {
        friend class Cue;

    public:
        CueIndex cueIndex00() const { return mCueIndex00; }
        CueIndex cueIndex01() const { return mCueIndex01; }

        QByteArray tag(TrackTags::TagId tagId) const { return mTags.value(tagId); }

        int        trackNumTag() const { return mTrackNumTag; }
        QByteArray artistTag() const { return performerTag(); }
        QByteArray commentTag() const { return tag(TrackTags::TagId::Comment); }
        QByteArray dateTag() const { return tag(TrackTags::TagId::Date); }
        QByteArray genreTag() const { return tag(TrackTags::TagId::Genre); }
        QByteArray fileTag() const { return mFileTag; }
        QByteArray flagsTag() const { return tag(TrackTags::TagId::Flags); }
        QByteArray isrcTag() const { return tag(TrackTags::TagId::Isrc); }
        QByteArray performerTag() const { return tag(TrackTags::TagId::Performer); }
        QByteArray songWriterTag() const { return tag(TrackTags::TagId::SongWriter); }
        QByteArray titleTag() const { return tag(TrackTags::TagId::Title); }

        Tags::Track decode(const TextCodec &textCodec) const;

    private:
        CueIndex mCueIndex00;
        CueIndex mCueIndex01;

        int mTrackNumTag = 0;

        QMap<TrackTags::TagId, QByteArray> mTags;
        QByteArray                         mFileTag;
    };

    static constexpr const char *const EMBEDED_PREFIX = "embedded://";

public:
    Cue()                 = default;
    Cue(const Cue &other) = default;
    explicit Cue(const QString &fileName) noexcept(false);
    Cue &operator=(const Cue &other) = default;

    QString filePath() const { return mFilePath; }
    TagsId  tagsId() const { return mTagsId; }

    QByteArray tag(AlbumTags::TagId tagId) const { return mTags.value(tagId); }

    DiscNum    discCountTag() const { return mDiscCountTag; }
    DiscNum    discNumTag() const { return mDiscNumTag; }
    QByteArray albumTag() const { return tag(AlbumTags::TagId::Album); }
    QByteArray catalogTag() const { return tag(AlbumTags::TagId::Catalog); }
    QByteArray cdTextfileTag() const { return tag(AlbumTags::TagId::CdTextfile); }
    QByteArray discIdTag() const { return tag(AlbumTags::TagId::DiscId); }
    QByteArray albumPerformer() const { return tag(AlbumTags::TagId::AlbumPerformer); }

    QList<Track> tracks() const { return mTracks; }
    bool         isEmpty() const { return mTracks.isEmpty(); }

    TextCodec detectTextCodec() const;

    TextCodec::BomCodec bom() const { return mBom; }

    QStringList fileTags() const { return mFileTags; }

    bool isMutiplyAudio() const { return mMutiplyAudio; }
    bool isEmbedded() const { return mFilePath.startsWith(EMBEDED_PREFIX); }

    Tags decode(const TextCodec &textCodec) const;

protected:
    QString mFilePath;
    TagsId  mTagsId;

    QList<Track> mTracks;

    QStringList mFileTags;
    bool        mMutiplyAudio = false;

    TextCodec::BomCodec mBom = TextCodec::BomCodec::Unknown;

protected:
    void       read(const CueData &data);
    QByteArray getAlbumPerformer(const CueData &data);
    void       splitTitleTag(const CueData &data);
    void       splitTitle(Track *track, char separator);
    void       validate();

public:
private:
    DiscNum mDiscCountTag = 0;
    DiscNum mDiscNumTag   = 0;

    QMap<AlbumTags::TagId, QByteArray> mTags;
};

class EmbeddedCue : public Cue
{
public:
    EmbeddedCue()                         = default;
    EmbeddedCue(const EmbeddedCue &other) = default;
    EmbeddedCue &operator=(const EmbeddedCue &other) = default;

    explicit EmbeddedCue(const InputAudioFile &audioFile) noexcept(false);
};

class CueError : public FlaconError
{
public:
    explicit CueError(const QString &msg) :
        FlaconError(msg) { }

    virtual ~CueError();
};

#endif // CUE_H
