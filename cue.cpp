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

#include "cue.h"
#include "cuedata.h"
#include "types.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QDebug>
#include "uchardetect.h"
#include "profiles.h"
#include "inputaudiofile.h"
#include "formats_in/informat.h"
#include <QBuffer>

/**************************************
 * Tags::Track
 **************************************/
Tags::Track Cue::Track::decode(const TextCodec &textCodec) const
{
    Tags::Track res;
    res.setTrackNum(mTrackNumTag);

    for (TrackTags::TagId tagId : mTags.keys()) {
        res.setTag(tagId, textCodec.decode(mTags.value(tagId)));
    }

    return res;
}

/************************************************
 *
 ************************************************/
Cue::Cue(const QString &fileName) noexcept(false)
{
    CueData data(fileName);
    if (data.isEmpty()) {
        throw CueError(QObject::tr("<b>%1</b> is not a valid CUE file. The CUE sheet has no FILE tag.").arg(fileName));
    }

    QFileInfo fileInfo(fileName);
    mFilePath     = fileInfo.absoluteFilePath();
    mTagsId.uri   = fileInfo.absoluteFilePath();
    mTagsId.title = fileInfo.fileName();

    read(data);
}

/**************************************
 * If all tracks have the same tag value, it is
 * more convenient for the user to edit the tag in the album area.
 **************************************/
static bool updateGlobalTagsFromTracks(const CueData &data, const QByteArray &tag, QByteArray *variable)
{
    bool same = data.allTracksHaveSameTag(tag);

    if (!same) {
        return false;
    }

    QByteArray trackValue = data.tracks().first().value(tag);

    if (*variable == trackValue) {
        return true;
    }

    if (variable->isEmpty()) {
        *variable = trackValue;
        return true;
    }

    return false;
}

/************************************************
 *
 ************************************************/
void Cue::read(const CueData &data)
{
    const CueData::Tags &global = data.globalTags();

    mTags[AlbumTags::TagId::Album]          = global.value(CueData::TITLE_TAG);
    mTags[AlbumTags::TagId::Catalog]        = global.value("CATALOG");
    mTags[AlbumTags::TagId::CdTextfile]     = global.value("CDTEXTFILE");
    mTags[AlbumTags::TagId::DiscId]         = global.value("DISCID");
    mDiscNumTag                             = global.value("DISCNUMBER", "1").toInt();
    mDiscCountTag                           = global.value("TOTALDISCS", "1").toInt();
    mTags[AlbumTags::TagId::AlbumPerformer] = getAlbumPerformer(data);

    QByteArray prevFileTag;

    for (const CueData::Tags &t : data.tracks()) {
        Track track;
        track.mFileTag = t.value(CueData::FILE_TAG);

        QByteArray index00     = t.value("INDEX 00");
        QByteArray index00File = t.value("INDEX 00 FILE");

        QByteArray index01     = t.value("INDEX 01");
        QByteArray index01File = t.value("INDEX 01 FILE");

        if (index00.isEmpty() && !index01.isEmpty()) {
            if (index01File == prevFileTag) {
                index00     = index01;
                index00File = index01File;
            }
            else {
                index00     = "00:00:00";
                index00File = index01File;
            }
        }

        if (index01.isEmpty() && !index00.isEmpty()) {
            index01     = index00;
            index01File = index00File;
        }
        prevFileTag = index01File;

        track.mCueIndex00 = CueIndex(index00, index00File);
        track.mCueIndex01 = CueIndex(index01, index01File);

        track.mTags[TrackTags::TagId::Flags] = t.value(CueData::FLAGS_TAG);
        track.mTags[TrackTags::TagId::Isrc]  = t.value("ISRC");
        track.mTags[TrackTags::TagId::Title] = t.value("TITLE");

        track.mTags[TrackTags::TagId::Comment]    = t.value("COMMENT", global.value("COMMENT"));
        track.mTags[TrackTags::TagId::Genre]      = t.value("GENRE", global.value("GENRE"));
        track.mTags[TrackTags::TagId::Performer]  = t.value("PERFORMER", global.value("PERFORMER"));
        track.mTags[TrackTags::TagId::SongWriter] = t.value("SONGWRITER", global.value("SONGWRITER"));
        track.mTags[TrackTags::TagId::Date]       = t.value("DATE", global.value("DATE"));

        track.mTrackNumTag = TrackNum(t.value(CueData::TRACK_TAG).toUInt());

        mTracks.append(track);
    }

    if (Profile::isSplitTrackTitle()) {
        splitTitleTag(data);
    }

    {
        mFileTags.clear();
        mMutiplyAudio = false;

        QString prev;
        for (const Track &track : qAsConst(mTracks)) {
            QString s = QString::fromUtf8(track.fileTag());

            if (s != prev) {
                mMutiplyAudio = !mFileTags.isEmpty();
                mFileTags << s;
                prev = s;
            }
        }
    }

    mBom = data.bomCodec();
    validate();
}

/************************************************
 *
 ************************************************/
void Cue::validate()
{
    bool hasFileTag = false;
    for (const Track &t : qAsConst(mTracks)) {
        hasFileTag = hasFileTag || !t.fileTag().isEmpty();
    }

    if (!hasFileTag) {
        throw CueError(QObject::tr("<b>%1</b> is not a valid CUE file. The CUE sheet has no FILE tag.").arg(mFilePath));
    }
}

/************************************************
 *
 ************************************************/
void Cue::splitTitle(Track *track, char separator)
{
    QByteArray b                              = track->titleTag();
    int        pos                            = b.indexOf(separator);
    track->mTags[TrackTags::TagId::Performer] = b.left(pos).trimmed();
    track->mTags[TrackTags::TagId::Title]     = b.right(b.length() - pos - 1).trimmed();
}

/************************************************
 *
 ************************************************/
TextCodec Cue::detectTextCodec() const
{
    UcharDet uchardet;

    uchardet << albumTag();

    for (const Track &track : mTracks) {
        uchardet << track.performerTag();
        uchardet << track.titleTag();
    }

    return uchardet.detect();
}

/**************************************
 *
 **************************************/
Tags Cue::decode(const TextCodec &textCodec) const
{
    Tags res;
    // clang-format off
    if (mDiscCountTag)  res.setDiscCount(mDiscCountTag);
    if (mDiscNumTag)    res.setDiscNum(mDiscNumTag);
    // clang-format on

    for (AlbumTags::TagId tagId : mTags.keys()) {
        res.setTag(tagId, textCodec.decode(mTags.value(tagId)));
    }

    for (const Track &t : mTracks) {
        res.tracks().append(t.decode(textCodec));
    }

    return res;
}

/************************************************
 *
 ************************************************/
QByteArray Cue::getAlbumPerformer(const CueData &data)
{
    QByteArray res = data.globalTags().value(CueData::PERFORMER_TAG);

    if (!res.isEmpty())
        return res;

    res = data.tracks().first().value(CueData::PERFORMER_TAG);

    for (const CueData::Tags &t : data.tracks()) {
        if (t.value(CueData::PERFORMER_TAG) != res)
            return "";
    }
    return res;
}

/************************************************
 *
 ************************************************/
void Cue::splitTitleTag(const CueData &data)
{
    bool splitByDash      = true;
    bool splitBySlash     = true;
    bool splitByBackSlash = true;

    for (const CueData::Tags &track : data.tracks()) {
        if (track.contains("PERFORMER")) {
            return;
        }

        QByteArray value = track.value("TITLE");
        splitByDash      = splitByDash && value.contains('-');
        splitBySlash     = splitBySlash && value.contains('/');
        splitByBackSlash = splitByBackSlash && value.contains('\\');
    }

    // clang-format off
    for (Track &track : mTracks) {
        if (splitByBackSlash)  splitTitle(&track, '\\');
        else if (splitBySlash) splitTitle(&track, '/');
        else if (splitByDash)  splitTitle(&track, '-');
    }
    // clang-format on
}

/************************************************
 *
 ************************************************/
EmbeddedCue::EmbeddedCue(const InputAudioFile &audioFile) noexcept(false) :
    Cue()
{
    QFileInfo fileInfo(audioFile.filePath());
    mFilePath     = EMBEDED_PREFIX + fileInfo.absoluteFilePath();
    mTagsId.uri   = mFilePath;
    mTagsId.title = QObject::tr("Embedded on %1", "The title for the CUE embedded in the audio file. %1 - is an audio-file name.").arg(fileInfo.fileName());

    if (!audioFile.isValid() || !audioFile.format()) {
        return;
    }

    QByteArray bytes = audioFile.format()->readEmbeddedCue(audioFile.filePath());
    if (bytes.isEmpty()) {
        return;
    }

    QBuffer buf(&bytes);
    buf.open(QBuffer::ReadOnly);

    CueData data(&buf);
    if (data.isEmpty()) {
        return;
    }

    read(data);
}

/**************************************
 *
 **************************************/
CueError::~CueError()
{
}
