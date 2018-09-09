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
#include "types.h"
#include "tags.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(5, 4, 0))
typedef QList<QByteArray> QByteArrayList;
#endif


enum CueTagId
{
    CTAG_UNKNOWN,
    CTAG_FILE,
    CTAG_TRACK,
    CTAG_INDEX,
    CTAG_DISCID,
    CTAG_CATALOG,
    CTAG_CDTEXTFILE,
    CTAG_TITLE,
    CTAG_COMMENT,
    CTAG_DATE,
    CTAG_FLAGS,
    CTAG_GENRE,
    CTAG_ISRC,
    CTAG_PERFORMER,
    CTAG_SONGWRITER
};


/************************************************
 *
 ************************************************/
CueDisk::CueDisk():
    Tracks(),
    mDiskCount(0),
    mDiskNum(0)
{

}


/************************************************

 ************************************************/
QString cueIndexTagKey(int indexNum)
{
    return QString("CUE_INDEX:%1").arg(indexNum);
}


/************************************************
 *
 ************************************************/
QByteArray unQuote(const QByteArray &line)
{
    if (line.length() > 2 &&
       (line.at(0) == '"' || line.at(0) == '\'') &&
        line.at(0) == line.at(line.length()-1))
    {
        return line.mid(1, line.length() - 2);
    }
    return line;
}


/************************************************
 *
 ************************************************/
CueReader::CueReader()
{

}


/************************************************
 *
 ************************************************/
struct ParserTrack
{
    ParserTrack(DiskNum diskNum, Track tags):
        diskNum(diskNum),
        tags(tags)
    {
    }

    DiskNum   diskNum;
    Track     tags;
};



/************************************************

 ************************************************/
void parseLine(const QByteArray &line, CueTagId &tag, QByteArray &value)
{
    QByteArray l = line.trimmed();

    if (l.isEmpty())
        tag = CTAG_UNKNOWN;


    QByteArray tagStr = leftPart(l, ' ').toUpper();
    value = rightPart(l, ' ').trimmed();


    if (tagStr == "REM")
    {
        tagStr = leftPart(value, ' ').toUpper();
        value = rightPart(value, ' ').trimmed();
    }

    value = unQuote(value);

    //=============================
    if      (tagStr == "FILE")       tag = CTAG_FILE;
    else if (tagStr == "TRACK")      tag = CTAG_TRACK;
    else if (tagStr == "INDEX")      tag = CTAG_INDEX;
    else if (tagStr == "DISCID")     tag = CTAG_DISCID;
    else if (tagStr == "CATALOG")    tag = CTAG_CATALOG;
    else if (tagStr == "CDTEXTFILE") tag = CTAG_CDTEXTFILE;
    else if (tagStr == "TITLE")      tag = CTAG_TITLE;
    else if (tagStr == "COMMENT")    tag = CTAG_COMMENT;
    else if (tagStr == "DATE")       tag = CTAG_DATE;
    else if (tagStr == "FLAGS")      tag = CTAG_FLAGS;
    else if (tagStr == "GENRE")      tag = CTAG_GENRE;
    else if (tagStr == "ISRC")       tag = CTAG_ISRC;
    else if (tagStr == "PERFORMER")  tag = CTAG_PERFORMER;
    else if (tagStr == "SONGWRITER") tag = CTAG_SONGWRITER;
    else                             tag = CTAG_UNKNOWN;
}


/************************************************

 ************************************************/
QByteArray extractFileFromFileTag(const QByteArray &value)
{
    int n = value.lastIndexOf(' ');
    if (n>-1)
        return unQuote(value.left(n));

    return unQuote(value);
}



/************************************************
 *
 ************************************************/
void readData(const QString &fileName, QByteArrayList *data,  QString *codecName)
{
    QFileInfo fi(fileName);
    if (!fi.exists())
        throw CueReaderError(QObject::tr("File <b>\"%1\"</b> does not exist").arg(fileName));

    QFile file(fi.canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly))
        throw CueReaderError(file.errorString());

    // Detect codepage and skip BOM .............
    QByteArray magic = file.read(3);
    int seek = 0;
    if (magic.startsWith("\xEF\xBB\xBF"))
    {
        *codecName = "UTF-8";
        seek = 3;
    }

    else if (magic.startsWith("\xFE\xFF"))
    {
        *codecName = "UTF-16BE";
        seek = 2;
    }

    else if (magic.startsWith("\xFF\xFE"))
    {
        *codecName = "UTF-16LE";
        seek = 2;
    }

    file.seek(seek);

    // Detect codepage and skip BOM .............
    while (!file.atEnd())
    {
        QByteArray line = file.readLine().trimmed();
        if (line.isEmpty())
            continue;

        *data << line;
    }
    file.close();
}


/************************************************
*
************************************************/
static void splitTitle(QList<ParserTrack*> *tracks, char separator)
{
    foreach (ParserTrack *track, *tracks)
    {
        QByteArray b = track->tags.tagData(TagId::Title);
        int pos = b.indexOf(separator);
        track->tags.setTag(TagId::Artist, b.left(pos).trimmed());
        track->tags.setTag(TagId::Title,  b.right(b.length() - pos - 1).trimmed());
    }
}


/************************************************
 *
 ************************************************/
static void setDiskArtist(QList<ParserTrack*> *tracks)
{
    QByteArray artist = tracks->first()->tags.tagData(TagId::Artist);

    foreach (ParserTrack *track, *tracks)
    {
        if (track->tags.tagData(TagId::Artist) != artist)
            return;
    }

    foreach (ParserTrack *track, *tracks)
        track->tags.setTag(TagId::AlbumArtist, artist);
}


/************************************************
 Complete cue sheet syntax documentation
 https://github.com/flacon/flacon/blob/master/cuesheet_syntax.md
 ************************************************/
QVector<CueDisk> CueReader::load(const QString &fileName)
{
    QByteArrayList data;
    QString codecName;
    QString fullPath = QFileInfo(fileName).absoluteFilePath();
    readData(fullPath, &data, &codecName);

    Track globalTags;

    enum class Mode {
        Global,
        Track
    };
    Mode mode = Mode::Global;

    int diskNum = -1;
    QList<ParserTrack*> tracks;
    ParserTrack *track = nullptr;

    bool splitByDash      = true;
    bool splitBySlash     = true;
    bool splitByBackSlash = true;
    int lineNum = 0;
    auto i = data.begin();
    for (; i != data.end(); ++i)
    {
        lineNum++;
        CueTagId tag;
        QByteArray value;
        parseLine((*i), tag, value);


        if (tag == CTAG_FILE)
        {
            globalTags.setTag(TagId::File, extractFileFromFileTag(value));
            ++diskNum;
            mode = Mode::Global;
            continue;
        }

        if (tag == CTAG_TRACK)
        {
            track = new ParserTrack(diskNum, globalTags);
            tracks << track;
            mode = Mode::Track;
            continue;
        }

        if (mode == Mode::Global) // Per disk tags ........
        {
            switch(tag)
            {
            case CTAG_DISCID:      globalTags.setTag(TagId::DiscId,     value); break;
            case CTAG_CATALOG:     globalTags.setTag(TagId::Catalog,    value); break;
            case CTAG_CDTEXTFILE:  globalTags.setTag(TagId::CDTextfile, value); break;
            case CTAG_TITLE:       globalTags.setTag(TagId::Album,      value); break;
            case CTAG_COMMENT:     globalTags.setTag(TagId::Comment,    value); break;
            case CTAG_DATE:        globalTags.setTag(TagId::Date,       value); break;
            case CTAG_GENRE:       globalTags.setTag(TagId::Genre,      value); break;
            case CTAG_PERFORMER:
            {
                globalTags.setTag(TagId::Artist,      value);
                globalTags.setTag(TagId::AlbumArtist, value);
                break;
            }
            case CTAG_SONGWRITER:  globalTags.setTag(TagId::SongWriter, value); break;
            default: break;
            }
            continue;
        }
        else // Per track tags ............................
        {
            if (diskNum < 0)
                throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. The cue sheet has no FILE tag.").arg(fileName));

            if (!track)
                throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. Incorrect track index on line %2.", "Cue parser error.").arg(fileName).arg(lineNum));

            switch (tag)
            {
            case CTAG_INDEX:
            {
                bool ok;
                int num = leftPart(value, ' ').toInt(&ok);
                if (!ok)
                    throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. Incorrect track index on line %2.", "Cue parser error.").arg(fileName).arg(lineNum));

                if (num < 0 || num > 99)
                    throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. Incorrect track index on line %2.", "Cue parser error.").arg(fileName).arg(lineNum));

                track->tags.setCueIndex(num, CueIndex(rightPart(value, ' ')));

                break;
            }



            case CTAG_DISCID:      track->tags.setTag(TagId::DiscId,     value); break;
            case CTAG_CATALOG:     track->tags.setTag(TagId::Catalog,    value); break;
            case CTAG_CDTEXTFILE:  track->tags.setTag(TagId::CDTextfile, value); break;

            case CTAG_TITLE:
            {
                track->tags.setTag(TagId::Title,      value);
                splitByDash      = splitByDash      && value.contains('-');
                splitBySlash     = splitBySlash     && value.contains('/');
                splitByBackSlash = splitByBackSlash && value.contains('\\');
                break;
            }
            case CTAG_COMMENT:     track->tags.setTag(TagId::Comment,    value); break;
            case CTAG_DATE:        track->tags.setTag(TagId::Date,       value); break;
            case CTAG_GENRE:       track->tags.setTag(TagId::Genre,      value); break;
            case CTAG_PERFORMER:
            {
                track->tags.setTag(TagId::Artist,  value);
                splitByDash      = false;
                splitBySlash     = false;
                splitByBackSlash = false;
                break;
            }
            case CTAG_SONGWRITER:  track->tags.setTag(TagId::SongWriter, value); break;
            case CTAG_ISRC:        track->tags.setTag(TagId::ISRC,       value); break;
            case CTAG_FLAGS:       track->tags.setTag(TagId::Flags,      value); break;

            case CTAG_FILE:
            case CTAG_TRACK:
            case CTAG_UNKNOWN:
                break;
            }
        }
    }

    if (tracks.isEmpty())
        throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. The cue sheet has no FILE tag.").arg(fileName));

    // Auto detect codepage :::::::::::::::::::::::::::::::
    if (codecName.isEmpty())
    {
        UcharDet charDet;
        foreach (ParserTrack *track, tracks)
            charDet << track->tags;

        codecName = charDet.textCodecName();
    }

    // Set global tags ::::::::::::::::::::::::::::::::::::
    if (splitByBackSlash)   splitTitle(&tracks, '\\');
    else if (splitBySlash)  splitTitle(&tracks, '/');
    else if (splitByDash)   splitTitle(&tracks, '-');

    if (globalTags.tagData(TagId::AlbumArtist).isEmpty())
        setDiskArtist(&tracks);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::


    QVector<CueDisk> res;
    TrackNum n =0;
    QFileInfo cueFileInfo(fileName);

    foreach (ParserTrack *track, tracks)
    {
        ++n;
        if (track->diskNum >= res.count())
        {
            CueDisk disk = CueDisk();
            disk.setTitle(cueFileInfo.fileName());
            disk.mDiskNum   = res.count() + 1;
            disk.mDiskCount = diskNum + 1;
            disk.mFileName  = fullPath;
            disk.setUri(fullPath + QString(" [%1]").arg(n));
            res << disk;
        }

        track->tags.setCueFileName(fullPath);
        track->tags.setTrackNum(n);
        track->tags.setTrackCount(diskNum);
        track->tags.setCodecName(codecName);
        res.last() << Track(track->tags);
    }

    qDeleteAll(tracks);


    for (int i=0; i<res.count(); ++i)
    {
        if (res.at(i).count() == 0)
        {
            throw CueReaderError(QObject::tr("<b>%1</b> is not a valid cue file. Disk %2 has no tags.").arg(fileName).arg(i));
        }
    }

    return res;
}
