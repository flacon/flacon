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


#include "track.h"

#include "disk.h"
#include "inputaudiofile.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"

#include <QDir>
#include <QDebug>

/************************************************

 ************************************************/
Track::Track(Disk *disk, int index):
    QObject(disk),
    mDisk(disk),
    mIndex(index),
    mCueIndexes(100),
    mStatus(NotRunning),
    mProgress(0)
{
    qRegisterMetaType<Track::Status>("Track::Status");
}


/************************************************

 ************************************************/
Track::~Track()
{
}


/************************************************
 *
 ************************************************/
uint Track::duration() const
{
    uint start = cueIndex(1).milliseconds();
    uint end = 0;
    if (index() < mDisk->count() - 1)
    {
        end = mDisk->track(index()+1)->cueIndex(1).milliseconds();
    }
    else if (mDisk->audioFile())
    {
        end = mDisk->audioFile()->duration();
    }

    if (start > end)
        return 0;

    return end - start;
}


/************************************************

 ************************************************/
QString Track::tag(const QString &tagName) const
{
    return mDisk->getTag(mIndex, tagName);
}


/************************************************

 ************************************************/
void Track::setTag(const QString &tagName, const QString &value)
{
    mDisk->setTag(mIndex, tagName, value);
}


/************************************************

 ************************************************/
void Track::setProgress(Track::Status status, int percent)
{
//    bool status2status [] = {
//        // NoRun   Cancel Error  Abort   OK   Process |_____________
//            0,     1,     1,     1,     1,     1,    // # NoRun    |
//            1,     0,     0,     0,     0,     0,    // # Cancel   |
//            1,     0,     0,     0,     0,     0,    // # Error    | Current
//            1,     0,     1,     0,     0,     0,    // # Aborted  | statuses
//            1,     0,     1,     0,     0,     0,    // # OK       |
//            1,     0,     1,     1,     1,     1,    // # Process  |

//    };
//    int len = 6;
//    int processStatus = 5;
    // Process is some is Splitting, Encoding, Queued  etc.

//    if (status2status[qMin(processStatus, int(mStatus)) * len +
//                      qMin(processStatus, int(status))])
    {
        mStatus = status;
        mProgress = percent;
        project->emitTrackProgress(this);
    }
}


/************************************************

 ************************************************/
QString Track::resultFileName() const
{
    QString pattern = settings->value(Settings::OutFiles_Pattern).toString();
    if (pattern.isEmpty())
        pattern = QString("%a/%y - %A/%n - %t");

    return calcFileName(pattern,
                        disk()->count(),
                        trackNum(),
                        this->album(),
                        this->title(),
                        this->artist(),
                        this->genre(),
                        this->date(),
                        OutFormat::currentFormat()->ext());
}


/************************************************
  %N  Number of tracks       %n  Track number
  %a  Artist                 %A  Album title
  %y  Year                   %g  Genre
  %t  Track title
 ************************************************/
QString Track::calcFileName(const QString &pattern,
                            int trackCount,
                            int trackNum,
                            const QString &album,
                            const QString &title,
                            const QString &artist,
                            const QString &genre,
                            const QString &date,
                            const QString &fileExt)
{
    QHash<QChar, QString> tokens;
    tokens.insert(QChar('N'),   QString("%1").arg(trackCount, 2, 10, QChar('0')));
    tokens.insert(QChar('n'),   QString("%1").arg(trackNum, 2, 10, QChar('0')));
    tokens.insert(QChar('A'),   Disk::safeString(album));
    tokens.insert(QChar('t'),   Disk::safeString(title));
    tokens.insert(QChar('a'),   Disk::safeString(artist));
    tokens.insert(QChar('g'),   Disk::safeString(genre));
    tokens.insert(QChar('y'),   Disk::safeString(date));

    QString res = expandPattern(pattern, &tokens, false);
    return res + "." + fileExt;
}


/************************************************

 ************************************************/
QString Track::expandPattern(const QString &pattern, const QHash<QChar, QString> *tokens, bool optional)
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
                    s = expandPattern(pattern.mid(start, j - start), tokens, true);
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

 ************************************************/
QString Track::resultFilePath() const
{
    QString fileName = resultFileName();
    if (fileName.isEmpty())
        return "";

    QString dir = calcResultFilePath();
    if (dir.endsWith("/") || fileName.startsWith("/"))
        return calcResultFilePath() + fileName;
    else
        return calcResultFilePath() + "/" + fileName;
}


/************************************************

 ************************************************/
QString Track::calcResultFilePath() const
{
    QString settingsDir = settings->value(Settings::OutFiles_Directory).toString();

    if (settingsDir == "~")
        return QDir::homePath();

    if (settingsDir == ".")
        settingsDir = "";

    if (settingsDir.startsWith("~/"))
        return settingsDir.replace(0, 1, QDir::homePath());

    QFileInfo fi(settingsDir);

    if (fi.isAbsolute())
        return fi.absoluteFilePath();

    if (!disk()->audioFileName().isEmpty())
        return QFileInfo(disk()->audioFileName()).dir().absolutePath() + QDir::separator() + settingsDir;

    return QFileInfo(QDir::homePath() + QDir::separator() + settingsDir).absoluteFilePath();
}


/************************************************

 ************************************************/
int Track::trackNum() const
{
    return mDisk->startTrackNum() + mIndex;
}


/************************************************

 ************************************************/
CueIndex Track::cueIndex(int indexNum) const
{
    return mCueIndexes.at(indexNum);
}


/************************************************

 ************************************************/
void Track::setCueIndex(int indexNum, const CueIndex &value)
{
    mCueIndexes[indexNum] = value;
}

