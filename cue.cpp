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
#include "tags.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QDebug>

/************************************************
*
************************************************/
static void splitTitle(Track *track, char separator)
{
    QByteArray b   = track->tagData(TagId::Title);
    int        pos = b.indexOf(separator);
    track->setTag(TagId::Artist, b.left(pos).trimmed());
    track->setTag(TagId::Title, b.right(b.length() - pos - 1).trimmed());
}

/************************************************
 *
 ************************************************/
CueDisc::CueDisc() :
    Tracks()
{
}

/************************************************
 *
 ************************************************/
CueDisc::CueDisc(const QString &fileName)
{
    CueData data(fileName);
    if (data.isEmpty()) {
        throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. The CUE sheet has no FILE tag.").arg(fileName));
    }

    QFileInfo cueFileInfo(fileName);
    QString   fullPath = cueFileInfo.absoluteFilePath();
    //CueDisc res;
    mFileName  = fullPath;
    mDiscCount = 1;
    mDiscNum   = 1;
    setUri(fullPath);
    setTitle(cueFileInfo.fileName());

    QByteArray           albumPerformer = getAlbumPerformer(data);
    const CueData::Tags &global         = data.globalTags();

    for (const CueData::Tags &t : data.tracks()) {
        Track track;
        track.setTag(TagId::File, t.value(CueData::FILE_TAG));
        track.setTag(TagId::Album, global.value(CueData::TITLE_TAG));

        track.setCueIndex(0, CueIndex(t.value("INDEX 00")));
        track.setCueIndex(1, CueIndex(t.value("INDEX 01")));

        track.setTag(TagId::Catalog, global.value("CATALOG"));
        track.setTag(TagId::CDTextfile, global.value("CDTEXTFILE"));
        track.setTag(TagId::Comment, global.value("COMMENT"));

        track.setTag(TagId::Flags, t.value("FLAGS"));
        track.setTag(TagId::Genre, global.value("GENRE"));
        track.setTag(TagId::ISRC, t.value("ISRC"));
        track.setTag(TagId::Title, t.value("TITLE"));
        track.setTag(TagId::Artist, t.value("PERFORMER", global.value("PERFORMER")));
        track.setTag(TagId::SongWriter, t.value("SONGWRITER", global.value("SONGWRITER")));
        track.setTag(TagId::DiscId, global.value("DISCID"));

        track.setTag(TagId::Date, t.value("DATE", global.value("DATE")));
        track.setTag(TagId::AlbumArtist, albumPerformer);

        track.setTrackNum(TrackNum(t.value(CueData::TRACK_TAG).toUInt()));
        track.setTrackCount(TrackNum(data.tracks().count()));
        track.setTag(TagId::DiscNum, global.value("DISCNUMBER", "1"));
        track.setTag(TagId::DiscCount, global.value("TOTALDISCS", "1"));
        track.setCueFileName(fullPath);

        this->append(track);
    }

    splitTitleTag(data);
    setCodecName(data);
}

/************************************************
 *
 ************************************************/
bool CueDisc::isMutiplyAudio() const
{
    if (isEmpty())
        return false;

    QByteArray file = last().tagData(TagId::File);
    for (const Track &track : *this) {
        if (track.tagData(TagId::File) != file)
            return true;
    }

    return false;
}

/************************************************
*
************************************************/
QByteArray CueDisc::getAlbumPerformer(const CueData &data)
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
void CueDisc::splitTitleTag(const CueData &data)
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
    for (Track &track : *this) {
        if (splitByBackSlash)  splitTitle(&track, '\\');
        else if (splitBySlash) splitTitle(&track, '/');
        else if (splitByDash)  splitTitle(&track, '-');
    }
    // clang-format off
}

/************************************************
* Auto detect codepage
************************************************/
void CueDisc::setCodecName(const CueData &data)
{
    QString codecName = data.codecName();

    if (codecName.isEmpty()) {
        UcharDet charDet;
        foreach (const Track &track, *this) {
            charDet << track;
        }

        codecName = charDet.textCodecName();
    }

    for (Track &track : *this) {
        track.setCodecName(codecName);
    }
}

/************************************************
 *
 ************************************************/
CueReader::CueReader()
{
}



QVector<CueDisc> CueReader::load(const QString &fileName)
{
    return QVector<CueDisc>();
}

