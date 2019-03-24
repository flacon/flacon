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


#include "types.h"
#include <QVector>
#include <QTextStream>

/************************************************
 *
 ************************************************/
QString preGapTypeToString(PreGapType type)
{
    switch(type)
    {
    case PreGapType::ExtractToFile:   return "Extract";
    case PreGapType::AddToFirstTrack: return "AddToFirst";
    default:                          return "Disable";
    }
}


/************************************************

 ************************************************/
PreGapType strToPreGapType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "EXTRACT")     return PreGapType::ExtractToFile;
    if (s == "ADDTOFIRST")  return PreGapType::AddToFirstTrack;

    return PreGapType::AddToFirstTrack;
}



/************************************************

 ************************************************/
QString gainTypeToString(GainType type)
{
    switch(type)
    {
    case GainType::Disable: return "Disable";
    case GainType::Track:   return "Track";
    case GainType::Album:   return "Album";
    }

    return "Disable";
}


/************************************************

 ************************************************/
GainType strToGainType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "TRACK")   return GainType::Track;
    if (s == "ALBUM")   return GainType::Album;

    return GainType::Disable;
}


/************************************************

 ************************************************/
QString coverModeToString(CoverMode mode)
{
    switch(mode)
    {
    case CoverMode::Disable:  return "Disable";
    case CoverMode::OrigSize: return "OrigSize";
    case CoverMode::Scale:    return "Scale";
    }

    return "Disable";

}


/************************************************

 ************************************************/
CoverMode strToCoverMode(const QString &str)
{
    QString s = str.toUpper();

    if (s == "ORIGSIZE") return CoverMode::OrigSize;
    if (s == "SCALE")    return CoverMode::Scale;

    return CoverMode::Disable;
}


/************************************************

 ************************************************/
unsigned int levenshteinDistance(const QString &s1, const QString & s2)
{
    const unsigned int len1 = s1.size(), len2 = s2.size();
    QVector<unsigned int> col(len2+1), prevCol(len2+1);

    for (int i = 0; i < prevCol.size(); i++)
        prevCol[i] = i;

    for (unsigned int i = 0; i < len1; i++)
    {
        col[0] = i+1;
        for (unsigned int j = 0; j < len2; j++)
            col[j+1] = qMin( qMin( 1 + col[j], 1 + prevCol[1 + j]),
                            prevCol[j] + (s1[i]==s2[j] ? 0 : 1) );
        col.swap(prevCol);
    }
    return prevCol[len2];
}


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
 *
 ************************************************/
void initTypes()
{
    qRegisterMetaType<TrackState>("TrackState");
}


Messages::Handler *Messages::mHandler = nullptr;

/************************************************
 *
 ************************************************/
void Messages::error(const QString &message)
{
    QString msg(message);
    msg.replace("<br>", " ", Qt::CaseInsensitive);
    msg.remove(QRegExp("<[^>]*>"));
    msg.replace("\\n", "\n");
    QTextStream(stderr) << msg.toLocal8Bit() << endl;

    if (mHandler)
        mHandler->showErrorMessage(message);
}


/************************************************
 *
 ************************************************/
void Messages::setHandler(Messages::Handler *handler)
{
    mHandler = handler;
}


/************************************************

 ************************************************/
QString safeString(const QString &str)
{
    QString res = str;
    res.replace('|', "-");
    res.replace('/', "-");
    res.replace('\\', "-");
    res.replace(':', "-");
    res.replace('*', "-");
    res.replace('?', "-");
    return res;
}



/************************************************

 ************************************************/
static QString doExpandPattern(const QString &pattern, const QHash<QChar, QString> *tokens, bool optional)
{
    QString res;
    bool perc = false;
    bool hasVars = false;
    bool isValid = true;


    for(int i=0; i<pattern.length(); ++i)
    {
        QChar c = pattern.at(i);


        // Sub pattern .................................
        if (c == '{')
        {
            int level = 0;
            int start = i + 1;
            //int j = i;
            QString s = "{";

            for (int j=i; j<pattern.length(); ++j)
            {
                c = pattern.at(j);
                if (c == '{')
                    level++;
                else if (c == '}')
                    level--;

                if (level == 0)
                {
                    s = doExpandPattern(pattern.mid(start, j - start), tokens, true);
                    i = j;
                    break;
                }
            }
            res += s;
        }
        // Sub pattern .................................

        else
        {
            if (perc)
            {
                perc = false;
                if (tokens->contains(c))
                {
                    QString s = tokens->value(c);
                    hasVars = true;
                    isValid = !s.isEmpty();
                    res += s;
                }
                else
                {
                    if (c == '%')
                        res += "%";
                    else
                        res += QString("%") + c;
                }
            }
            else
            {
                if (c == '%')
                    perc = true;
                else
                    res += c;
            }
        }
    }

    if (perc)
        res += "%";

    if (optional)
    {
        if  (hasVars)
        {
            if (!isValid)
                return "";
        }
        else
        {
            return "{" + res + "}";
        }
    }

    return res;
}


/************************************************
  %N  Number of tracks       %n  Track number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title
 ************************************************/
QString expandPattern(const QString &pattern,
                     int trackCount,
                     int trackNum,
                     const QString &album,
                     const QString &title,
                     const QString &artist,
                     const QString &genre,
                     const QString &date)
{
    QHash<QChar, QString> tokens;
    tokens.insert(QChar('N'),   QString("%1").arg(trackCount, 2, 10, QChar('0')));
    tokens.insert(QChar('n'),   QString("%1").arg(trackNum, 2, 10, QChar('0')));
    tokens.insert(QChar('A'),   safeString(album));
    tokens.insert(QChar('t'),   safeString(title));
    tokens.insert(QChar('a'),   safeString(artist));
    tokens.insert(QChar('g'),   safeString(genre));
    tokens.insert(QChar('y'),   safeString(date));

    return doExpandPattern(pattern, &tokens, false);
}


/************************************************
 *
 ************************************************/
QString patternExample(const QString &pattern)
{
    return expandPattern(pattern, 14, 13, "Help", "Yesterday", "The Beatles", "Pop", "1965");
}
