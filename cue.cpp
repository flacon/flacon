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


enum CueTagId
{
    CTAG_EMPTY,
    CTAG_UNKNOWN,
    CTAG_FILE,
    CTAG_TRACK,
    CTAG_INDEX00,
    CTAG_INDEX01,
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
 *
 ************************************************/
static QByteArray unQuote(const QByteArray &line)
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

 ************************************************/
static QByteArray extractFileFromFileTag(const QByteArray &value)
{
    int n = value.lastIndexOf(' ');
    if (n>-1)
        return unQuote(value.left(n));

    return unQuote(value);
}


/************************************************
*
************************************************/
static void splitTitle(Track *track, char separator)
{
    QByteArray b = track->tagData(TagId::Title);
    int pos = b.indexOf(separator);
    track->setTag(TagId::Artist, b.left(pos).trimmed());
    track->setTag(TagId::Title,  b.right(b.length() - pos - 1).trimmed());
}


/************************************************
 *
 ************************************************/
typedef QHash<CueTagId, QByteArray> CueTags;

inline CueTags &operator<<(CueTags &dest, const CueTags &src)
{
    CueTags::const_iterator i;
    for (i = src.constBegin(); i != src.constEnd(); ++i)
        dest.insert(i.key(), i.value());

    return dest;
}


/************************************************
 *
 ************************************************/
struct CueData
{
    explicit CueData(const QString &fileName);


    CueTags globalTags;
    QVector<CueTags> tracks;
    QString codecName;


private:
    QString mFileName;
    int mLineNum;
    void parseLine(const QByteArray &line, CueTagId &tag, QByteArray &value) const;
};


/************************************************
 *
 ************************************************/
CueData::CueData(const QString &fileName):
    mFileName(fileName),
    mLineNum(0)
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
        codecName = "UTF-8";
        seek = 3;
    }

    else if (magic.startsWith("\xFE\xFF"))
    {
        codecName = "UTF-16BE";
        seek = 2;
    }

    else if (magic.startsWith("\xFF\xFE"))
    {
        codecName = "UTF-16LE";
        seek = 2;
    }

    file.seek(seek);


    // Read global tags ..............................
    QByteArray value;
    QByteArray audioFile;
    while (!file.atEnd())
    {
        mLineNum++;
        QByteArray line = file.readLine().trimmed();
        if (line.isEmpty())
            continue;

        CueTagId tag;
        parseLine(line, tag, value);

        if (tag == CTAG_EMPTY)
            continue;

        if (tag == CTAG_TRACK)
        {
            break;
        }


        if (tag == CTAG_FILE)
            audioFile = extractFileFromFileTag(value);
        else
            globalTags.insert(tag, value);
    }



    while (!file.atEnd())
    {
        bool ok;
        int num = leftPart(value, ' ').toInt(&ok);
        if (!ok)
            throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track number on line %2.", "Cue parser error.").arg(mFileName).arg(mLineNum));

        CueTags track;
        track.insert(CTAG_TRACK, QString("%1").arg(num).toLatin1());
        track.insert(CTAG_FILE, audioFile);

        while (!file.atEnd())
        {
            mLineNum++;
            QByteArray line = file.readLine().trimmed();
            if (line.isEmpty())
                continue;

            CueTagId tag;
            parseLine(line, tag, value);
            if (value.isEmpty())
                continue;

            if (tag == CTAG_TRACK)
                break;

            if (tag == CTAG_FILE)
            {
                audioFile = extractFileFromFileTag(value);
                continue;
            }

            track.insert(tag, value);
        }
        tracks << track;
    }

    file.close();
}


/************************************************
 *
 ************************************************/
void CueData::parseLine(const QByteArray &line, CueTagId &tag, QByteArray &value) const
{
    QByteArray l = line.trimmed();

    if (l.isEmpty())
    {
        tag = CTAG_EMPTY;
        return;
    }

    tag   = CueTagId::CTAG_UNKNOWN;
    QByteArray tagStr = leftPart(l, ' ').toUpper();
    value = rightPart(l, ' ').trimmed();


    if (tagStr == "REM")
    {
        tagStr = leftPart(value, ' ').toUpper();
        value = rightPart(value, ' ').trimmed();
    }

    value = unQuote(value);

    //=============================
    if (tagStr == "INDEX")
    {
        bool ok;
        int num = leftPart(value, ' ').toInt(&ok);
        if (!ok)
            throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track index on line %2.", "Cue parser error.").arg(mFileName).arg(mLineNum));

        if (num < 0 || num > 99)
            throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track index on line %2.", "Cue parser error.").arg(mFileName).arg(mLineNum));

        switch (num)
        {
        case 0:
            tag   = CTAG_INDEX00;
            value = rightPart(value, ' ').trimmed();
            break;
        case 1:
            tag   = CTAG_INDEX01;
            value = rightPart(value, ' ').trimmed();
            break;
        }
    }
    else if (tagStr == "FILE")       tag = CTAG_FILE;
    else if (tagStr == "TRACK")      tag = CTAG_TRACK;

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
 Complete CUE sheet syntax documentation
 https://github.com/flacon/flacon/blob/master/cuesheet_syntax.md
 ************************************************/
QVector<CueDisk> CueReader::load(const QString &fileName)
{
    CueData data(fileName);
    if (data.tracks.isEmpty())
        throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. The CUE sheet has no FILE tag.").arg(fileName));

    bool splitByDash      = true;
    bool splitBySlash     = true;
    bool splitByBackSlash = true;
    int diskCount         = 0;
    QByteArray albumArtist = data.tracks.first().value(CTAG_PERFORMER);
    {
        QByteArray audioFile;
        for (const CueTags &tags: data.tracks)
        {
            if (tags.contains(CTAG_PERFORMER))
            {
                splitByDash      = false;
                splitBySlash     = false;
                splitByBackSlash = false;
            }
            else
            {
                QByteArray value = tags.value(CTAG_TITLE);
                splitByDash      = splitByDash      && value.contains('-');
                splitBySlash     = splitBySlash     && value.contains('/');
                splitByBackSlash = splitByBackSlash && value.contains('\\');
            }

            if (tags.value(CTAG_PERFORMER) != albumArtist)
                albumArtist.clear();

            if (audioFile != tags.value(CTAG_FILE))
            {
                diskCount++;
                audioFile = tags.value(CTAG_FILE);
            }
        }
    }
    if (!data.globalTags.value(CTAG_PERFORMER).isEmpty())
        albumArtist = data.globalTags.value(CTAG_PERFORMER);


    QFileInfo cueFileInfo(fileName);
    QString fullPath = QFileInfo(fileName).absoluteFilePath();

    QVector<CueDisk> res;
    QByteArray audioFile;
    for (int i=0; i<data.tracks.count(); ++i)
    {
        CueTags tags = data.globalTags;
        tags.remove(CTAG_TITLE);
        tags << data.tracks[i];

        if (audioFile != tags.value(CTAG_FILE))
        {
            audioFile = tags.value(CTAG_FILE);
            CueDisk disk;
            disk.setTitle(cueFileInfo.fileName());
            disk.mFileName  = fullPath;
            disk.mDiskCount = diskCount;
            disk.mDiskNum   = res.count() + 1;
            disk.setUri(fullPath + QString(" [%1]").arg(res.count() + 1));
            res << disk;
        }

        CueDisk &disk = res.last();
        disk << Track();
        Track &track = disk.last();

        track.setTag(TagId::File,          audioFile);
        track.setTag(TagId::Album,         data.globalTags.value(CTAG_TITLE));
        track.setTrackNum(tags.value(CTAG_TRACK).toInt());
        track.setTrackCount(data.tracks.count());
        track.setTag(TagId::DiskNum,    QString("1"));
        track.setTag(TagId::DiskCount,  QString("1"));

        track.setCueIndex(0, CueIndex(tags.value(CTAG_INDEX00)));
        track.setCueIndex(1, CueIndex(tags.value(CTAG_INDEX01)));

        track.setTag(TagId::Catalog,       tags.value(CTAG_CATALOG));
        track.setTag(TagId::CDTextfile,    tags.value(CTAG_CDTEXTFILE));
        track.setTag(TagId::Comment,       tags.value(CTAG_COMMENT));
        track.setTag(TagId::Date,          tags.value(CTAG_DATE));
        track.setTag(TagId::Flags,         tags.value(CTAG_FLAGS));
        track.setTag(TagId::Genre,         tags.value(CTAG_GENRE));
        track.setTag(TagId::ISRC,          tags.value(CTAG_ISRC));
        track.setTag(TagId::Title,         tags.value(CTAG_TITLE));
        track.setTag(TagId::Artist,        tags.value(CTAG_PERFORMER));
        track.setTag(TagId::SongWriter,    tags.value(CTAG_SONGWRITER));
        track.setTag(TagId::DiscId,        tags.value(CTAG_DISCID));
        track.setTag(TagId::AlbumArtist,   albumArtist);

        if (splitByBackSlash)   splitTitle(&track, '\\');
        else if (splitBySlash)  splitTitle(&track, '/');
        else if (splitByDash)   splitTitle(&track, '-');

        track.setCueFileName(fullPath);
    }

    // Auto detect codepage :::::::::::::::::::::::::::::::
    QString codecName = data.codecName;
    if (codecName.isEmpty())
    {
        UcharDet charDet;
        foreach (const CueDisk &disk, res)
        {
            foreach (const Track &track, disk)
                charDet << track;
        }

        codecName = charDet.textCodecName();
    }


    for (int d=0; d<res.count(); ++d)
      {
          CueDisk &disk = res[d];

          if (disk.count() == 0)
          {
              throw CueReaderError(QObject::tr("<b>%1</b> is not a valid CUE file. Disk %2 has no tags.").arg(fileName).arg(d));
          }

          for (int t=0; t<disk.count(); ++t)
          {
              Track &track = disk[t];
              track.setCodecName(codecName);
          }
      }

    return res;
}
