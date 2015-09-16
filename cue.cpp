#include "cue.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QDebug>


enum CueTags
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

 ************************************************/
CueIndex::CueIndex(const QString &str):
    mNull(true),
    mCdValue(0),
    mHiValue(0)
{
    if (!str.isEmpty())
        mNull = !parse(str);
}


/************************************************
 *
 ************************************************/
quint64 cueIndexKey(int diskNum, int trackNum, int indexNum)
{
    // disk    track   index
    // FFFFFFF FFFFFFF FF
    return (diskNum << 9) + (trackNum << 7) + indexNum;
}


/************************************************

 ************************************************/
QString CueIndex::toString(bool cdQuality) const
{
    if (cdQuality)
    {
        int min =  mCdValue / (60 * 75);
        int sec = (mCdValue - min * 60 * 75) / 75;
        int frm =  mCdValue - (min * 60 + sec) * 75;

        return QString("%1:%2:%3")
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(frm, 2, 10, QChar('0'));
    }
    else
    {
        int min = mHiValue / (60 * 1000);
        int sec = (mHiValue - min * 60 * 1000) / 1000;
        int msec = mHiValue - (min * 60 + sec) * 1000;

        return QString("%1:%2.%3")
                .arg(min,  2, 10, QChar('0'))
                .arg(sec,  2, 10, QChar('0'))
                .arg(msec, 3, 10, QChar('0'));
    }

}


/************************************************

 ************************************************/
CueIndex CueIndex::operator -(const CueIndex &other) const
{
    CueIndex res;
    res.mCdValue = this->mCdValue - other.mCdValue;
    res.mHiValue = this->mHiValue - other.mHiValue;
    res.mNull = false;
    return res;
}


/************************************************

 ************************************************/
bool CueIndex::operator ==(const CueIndex &other) const
{
    return this->mHiValue == other.mHiValue;
}


/************************************************

 ************************************************/
bool CueIndex::operator !=(const CueIndex &other) const
{
    return this->mHiValue != other.mHiValue;
}


/************************************************

 ************************************************/
bool CueIndex::parse(const QString &str)
{
    QStringList sl = str.split(QRegExp("\\D"), QString::KeepEmptyParts);

    if (sl.length()<3)
        return false;

    bool ok;
    int min = sl[0].toInt(&ok);
    if (!ok)
        return false;

    int sec = sl[1].toInt(&ok);
    if (!ok)
        return false;

    int frm = sl[2].leftJustified(2, '0').toInt(&ok);
    if (!ok)
        return false;

    int msec = sl[2].leftJustified(3, '0').toInt(&ok);
    if (!ok)
        return false;

    mCdValue = (min * 60 + sec) * 75 + frm;
    mHiValue = (min * 60 + sec) * 1000 + msec;
    return true;
}

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

 ************************************************/
CueReader::CueReader(const QString &fileName):
    mFileName(fileName)
{
}


/************************************************

 ************************************************/
void CueReader::load()
{
    QFileInfo fi(mFileName);
    if (!fi.exists())
        throw QObject::tr("File \"%1\" not exists").arg(mFileName);

    QFile file(fi.canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly))
        throw file.errorString();

    // Detect codepage and skip BOM .............
    QString codecName;
    QByteArray magic = file.read(3);

    if (magic.startsWith("\xEF\xBB\xBF"))
    {
        codecName = "UTF-8";
        file.seek(3);
    }

    else if (magic.startsWith("\xFE\xFF"))
    {
        codecName = "UTF-16BE";
        file.seek(2);
    }

    else if (magic.startsWith("\xFF\xFE"))
    {
        codecName = "UTF-16LE";
        file.seek(2);
    }
    // Detect codepage and skip BOM .............


    try
    {
        parse(file);
    }
    catch (QString e)
    {
        file.close();
        throw e;
    }

    file.close();

    if (mTagSetList.isEmpty())
        throw QObject::tr("The <b>%1</b> is not a valid CUE file.").arg(mFileName);

    QString title = QFileInfo(mFileName).fileName();
    for (int i=0; i<mTagSetList.count(); ++i)
    {
        TagSet &tags = mTagSetList[i];
        tags.setTextCodecName("UTF-8");
        tags.setTitle(title + (mTagSetList.count() == 1 ? "" : QObject::tr(" [disk %1]").arg(i + 1)));
    }
}


/************************************************

 ************************************************/
CueIndex CueReader::cueIndex(int diskNum, int trackNum, int indexNum) const
{
    return mIndexes.value(cueIndexKey(diskNum, trackNum, indexNum));
}


/************************************************

 ************************************************/
QByteArray leftPart(const QByteArray &line, const QChar separator)
{
    int n = line.indexOf(separator);
    if (n > -1)
        return line.left(n);
    else
        return line;
}


/************************************************

 ************************************************/
QByteArray rightPart(const QByteArray &line, const QChar separator)
{
    int n = line.indexOf(separator);
    if (n > -1)
        return line.right(line.length() - n - 1);
    else
        return QByteArray();
}


/************************************************

 ************************************************/
void parseLine(const QByteArray &line, CueTags &tag, QByteArray &value)
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
 Complete cue sheet syntax documentation
 http://digitalx.org/cue-sheet/syntax/
 ************************************************/
void CueReader::parse(QFile &file)
{
    CueTags tag;
    QByteArray value;

    int diskNum = -1;
    while (!file.atEnd())
    {
        parseLine(file.readLine(), tag, value);

        switch (tag)
        {
        case CTAG_FILE:
            diskNum++;
            mTagSetList << parseOneDiskTags(file, value, diskNum);
            break;

        case CTAG_DISCID:      mDiskId = value;     break;
        case CTAG_CATALOG:     mCatalog = value;    break;
        case CTAG_CDTEXTFILE:  mCdTextFile = value; break;
        case CTAG_TITLE:       mAlbum = value;      break;
        case CTAG_COMMENT:     mComment = value;    break;
        case CTAG_DATE:        mDate = value;       break;
        case CTAG_GENRE:       mGenre = value;      break;
        case CTAG_PERFORMER:   mPerformer = value;  break;
        case CTAG_SONGWRITER:  mSongwriter = value; break;

        case CTAG_FLAGS:
        case CTAG_ISRC:
        case CTAG_UNKNOWN:
        case CTAG_TRACK:
        case CTAG_INDEX:
            break;
        }

    }
}

QByteArray extractFileFromFileTag(const QByteArray &value)
{
    int n = value.lastIndexOf(' ');
    if (n>-1)
        return unQuote(value.left(n));

    return unQuote(value);
}

/************************************************
 Complete cue sheet syntax documentation
 http://digitalx.org/cue-sheet/syntax/
 ************************************************/
TagSet CueReader::parseOneDiskTags(QFile &file, QByteArray fileTag, int diskNum)
{
    TagSet tagSet(QFileInfo(mFileName).canonicalFilePath() + QString(" [%1]").arg(mTagSetList.count()));
    tagSet.setDiskTag(TAG_FILE, extractFileFromFileTag(fileTag), false);
    tagSet.setDiskTag(TAG_DISCID,     mDiskId,     true);
    tagSet.setDiskTag(TAG_CATALOG,    mCatalog,    false);
    tagSet.setDiskTag(TAG_CDTEXTFILE, mCdTextFile, false);
    tagSet.setDiskTag(TAG_DISKNUM,    QString("%1").arg(diskNum + 1).toLatin1(), true);


    int trackIdx = -1;
    CueTags tag;
    QByteArray value;
    qint64 pos;

    while (!file.atEnd())
    {
        pos = file.pos();
        QByteArray line = file.readLine();
        parseLine(line, tag, value);
        //qDebug() << ":::" << tag << value;


        // Common tags ..........................
        switch (tag)
        {
        case CTAG_FILE:
            file.seek(pos);
            return tagSet;

        case CTAG_TRACK:
            trackIdx++;
            tagSet.setTrackTag(trackIdx, TAG_PERFORMER,  mPerformer,  false);
            tagSet.setTrackTag(trackIdx, TAG_ALBUM,      mAlbum,      false);
            tagSet.setTrackTag(trackIdx, TAG_GENRE,      mGenre,      false);
            tagSet.setTrackTag(trackIdx, TAG_DATE,       mDate,       false);
            tagSet.setTrackTag(trackIdx, TAG_COMMENT,    mComment,    false);
            tagSet.setTrackTag(trackIdx, TAG_SONGWRITER, mSongwriter, false);
            continue;
            break;

        default:
            break;
        }


        // Per track tags .......................
        if (trackIdx < 0)
        {
            qWarning() << "Unexpected tag in line" << line << ", file: " << mFileName << " pos: " << pos;
            continue;
        }

        switch (tag)
        {

        case CTAG_INDEX:
        {
            bool ok;
            int num = leftPart(value, ' ').toInt(&ok);
            if (!ok)
                throw QObject::tr("The <b>%1</b> is not a valid CUE file.").arg(mFileName);

            QString time = rightPart(value, ' ');
            mIndexes.insert(cueIndexKey(mTagSetList.count(), trackIdx, num), CueIndex(time));
        }
            break;

        case CTAG_DISCID:      tagSet.setDiskTag(TAG_DISCID,     value, true); break;
        case CTAG_CATALOG:     tagSet.setDiskTag(TAG_CATALOG,    value, true); break;
        case CTAG_CDTEXTFILE:  tagSet.setDiskTag(TAG_CDTEXTFILE, value, true); break;

        case CTAG_TITLE:       tagSet.setTrackTag(trackIdx, TAG_TITLE,      value, false); break;
        case CTAG_COMMENT:     tagSet.setTrackTag(trackIdx, TAG_COMMENT,    value, false); break;
        case CTAG_DATE:        tagSet.setTrackTag(trackIdx, TAG_DATE,       value, false); break;
        case CTAG_GENRE:       tagSet.setTrackTag(trackIdx, TAG_GENRE,      value, false); break;
        case CTAG_PERFORMER:   tagSet.setTrackTag(trackIdx, TAG_PERFORMER,  value, false); break;
        case CTAG_SONGWRITER:  tagSet.setTrackTag(trackIdx, TAG_SONGWRITER, value, false); break;
        case CTAG_ISRC:        tagSet.setTrackTag(trackIdx, TAG_ISRC,       value, true);  break;
        case CTAG_FLAGS:       tagSet.setTrackTag(trackIdx, TAG_FLAGS,      value, true);  break;

        case CTAG_FILE:
        case CTAG_TRACK:
        case CTAG_UNKNOWN:
            break;
        }
    }

    return tagSet;
}

