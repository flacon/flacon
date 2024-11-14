/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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

#include "patternexpander.h"
#include "track.h"
#include "disc.h"
#include <QDir>

class Tokens : public QHash<QChar, QString>
{
public:
    Tokens() :
        QHash<QChar, QString>()
    {
    }

    bool isEmptyForOptional(QChar key) const
    {
        // If album contains only one disc,
        // discCount and discNum are optional
        if (key == 'd' || key == 'D') {
            return (value('D').toInt() <= 1);
        }

        return this->value(key).isEmpty();
    }
};

/************************************************
 *
 ************************************************/
PatternExpander::PatternExpander()
{
}

/**************************************
 *
 **************************************/
PatternExpander::PatternExpander(const AlbumTags &albumTags, const TrackTags &trackTags, const TrackTags &firstTrackTags) :
    mAlbumTags(albumTags),
    mTrackTags(trackTags),
    mFirstTrackTags(firstTrackTags)
{
}

/************************************************
 *
 ************************************************/
PatternExpander::PatternExpander(const Track &track) :
    PatternExpander(track.disk()->toTags(), track.toTags(), track.disk()->tracks().first()->toTags())
{
}

/************************************************

 ************************************************/
static QString doExpandPattern(const QString &pattern, const Tokens &tokens, bool optional)
{
    QString res;
    bool    perc    = false;
    bool    hasVars = false;
    bool    isValid = true;

    for (int i = 0; i < pattern.length(); ++i) {
        QChar c = pattern.at(i);

        // Sub pattern .................................
        if (c == '{') {
            int level = 0;
            int start = i + 1;

            QString s = "{";

            for (int j = i; j < pattern.length(); ++j) {
                c = pattern.at(j);
                if (c == '{')
                    level++;
                else if (c == '}')
                    level--;

                if (level == 0) {
                    s = doExpandPattern(pattern.mid(start, j - start), tokens, true);
                    i = j;
                    break;
                }
            }
            res += s;
        }
        // Sub pattern .................................

        else {
            if (perc) {
                perc = false;
                if (tokens.contains(c)) {
                    hasVars = true;
                    if (optional && tokens.isEmptyForOptional(c)) {
                        isValid = false;
                    }
                    else {
                        QString s = tokens.value(c);
                        res += s;
                    }
                }
                else {
                    if (c == '%')
                        res += "%";
                    else
                        res += QStringLiteral("%") + c;
                }
            }
            else {
                if (c == '%')
                    perc = true;
                else
                    res += c;
            }
        }
    }

    if (perc)
        res += "%";

    if (optional) {
        if (hasVars) {
            if (!isValid)
                return "";
        }
        else {
            return "{" + res + "}";
        }
    }

    return res;
}

/**************************************
 *
 **************************************/
QString PatternExpander::expand(const QString &pattern) const
{
    int n = lastDirSeparattor(pattern);
    if (n < 0) {
        return expand(pattern, Mode::Track);
    }

    // If the disc is a collection, the files fall into different directories.
    // So we use the tag Disc::Performer for expand the directory path.
    QString dirs = expand(pattern.left(n), Mode::Album);
    QString file = expand(pattern.mid(n), Mode::Track);
    return dirs + file;
}

/**************************************
 *
 **************************************/
QString PatternExpander::expand(const QString &pattern, Mode mode) const
{
    const TrackTags &trackTags = (mode == Mode::Album) ? mFirstTrackTags : mTrackTags;

    Tokens tokens;
    tokens.insert(QChar('N'), QStringLiteral("%1").arg(mAlbumTags.trackCount(), 2, 10, QChar('0')));
    tokens.insert(QChar('n'), QStringLiteral("%1").arg(trackTags.trackNum(), 2, 10, QChar('0')));
    tokens.insert(QChar('D'), QStringLiteral("%1").arg(mAlbumTags.discCount(), 2, 10, QChar('0')));
    tokens.insert(QChar('d'), QStringLiteral("%1").arg(mAlbumTags.discNum(), 2, 10, QChar('0')));
    tokens.insert(QChar('A'), safeString(mAlbumTags.album()));
    tokens.insert(QChar('t'), safeString(trackTags.title()));
    tokens.insert(QChar('g'), safeString(trackTags.genre()));
    tokens.insert(QChar('a'), safeString(trackTags.performer()));
    tokens.insert(QChar('y'), safeString(trackTags.date()));

    return doExpandPattern(pattern, tokens, false);
}

/************************************************
 *
 ************************************************/
static QString safeFilePathLen(const QString &path)
{
    QString file = path;
    QString ext  = QFileInfo(path).suffix();
    if (!ext.isEmpty()) {
        ext = "." + ext;
        file.resize(file.length() - ext.length());
    }

    QStringList res;
    for (QString f : file.split(QDir::separator())) {
        while (f.toUtf8().length() > 250) {
            f.resize(f.length() - 1);
        }
        res << f;
    }
    return res.join(QDir::separator()) + ext;
}

/************************************************
 *
 ************************************************/
int PatternExpander::lastDirSeparattor(const QString &pattern)
{
    int squareNum = 0;
    for (int i = pattern.length() - 1; i >= 0; --i) {
        QChar c = pattern.at(i);

        if (c == QDir::separator() && squareNum == 0) {
            return i;
        }

        if (c == '}') {
            squareNum++;
        }

        if (c == '{') {
            squareNum--;
        }
    }

    return -1;
}

/************************************************
 *
 ************************************************/
QString PatternExpander::resultFileName(const QString &aPattern, const Track *track, const QString &ext)
{
    QString pattern = aPattern;
    if (pattern.isEmpty()) {
        pattern = QStringLiteral("%a/%y - %A/%n - %t");
    }

    PatternExpander expander(*track);
    return safeFilePathLen(expander.expand(pattern) + "." + ext);
}

/************************************************
 *
 ************************************************/
QString PatternExpander::example(const QString &pattern)
{
    AlbumTags album;
    album.setDiscNum(1);
    album.setDiscCount(1);
    album.setAlbum("Help");
    album.setTrackCount(14);

    TrackTags track;
    track.setArtist("The Beatles");
    track.setTrackNum(13);
    track.setTitle("Yesterday");
    track.setGenre("Pop");
    track.setDate("1965");

    PatternExpander expander(album, track, track);

    return expander.expand(pattern);
}
