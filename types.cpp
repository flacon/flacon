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
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

/************************************************
 *
 ************************************************/
QString preGapTypeToString(PreGapType type)
{
    // clang-format off
    switch (type) {
        case PreGapType::ExtractToFile:   return "Extract";
        case PreGapType::AddToFirstTrack: return "AddToFirst";
        case PreGapType::Skip:            return "Skip";
    }
    // clang-format on

    return "Skip";
}

/************************************************

 ************************************************/
PreGapType strToPreGapType(const QString &str, PreGapType def)
{
    QString s = str.toUpper();

    // clang-format off
    if (s == "DISABLE")    return PreGapType::Skip;
    if (s == "SKIP")       return PreGapType::Skip;
    if (s == "EXTRACT")    return PreGapType::ExtractToFile;
    if (s == "ADDTOFIRST") return PreGapType::AddToFirstTrack;
    // clang-format on

    return def;
}

/************************************************

 ************************************************/
QString gainTypeToString(GainType type)
{
    // clang-format off
    switch (type) {
        case GainType::Disable: return "Disable";
        case GainType::Track:   return "Track";
        case GainType::Album:   return "Album";
    }
    // clang-format on

    return "Disable";
}

/************************************************

 ************************************************/
GainType strToGainType(const QString &str, GainType def)
{
    QString s = str.toUpper();

    // clang-format off
    if (s == "DISABLE") return GainType::Disable;
    if (s == "TRACK")   return GainType::Track;
    if (s == "ALBUM")   return GainType::Album;
    // clang-format on

    return def;
}

/************************************************

 ************************************************/
QString coverModeToString(CoverMode mode)
{
    // clang-format off
    switch (mode) {
        case CoverMode::Disable:  return "Disable";
        case CoverMode::OrigSize: return "OrigSize";
        case CoverMode::Scale:    return "Scale";
    }
    // clang-format on

    return "Disable";
}

/************************************************

 ************************************************/
CoverMode strToCoverMode(const QString &str, CoverMode def)
{
    QString s = str.toUpper();

    // clang-format off
    if (s == "DISABLE")  return CoverMode::Disable;
    if (s == "ORIGSIZE") return CoverMode::OrigSize;
    if (s == "SCALE")    return CoverMode::Scale;
    // clang-format on

    return def;
}

/************************************************

 ************************************************/
unsigned int levenshteinDistance(const QString &s1, const QString &s2)
{
    const unsigned int    len1 = s1.size(), len2 = s2.size();
    QVector<unsigned int> col(len2 + 1), prevCol(len2 + 1);

    for (int i = 0; i < prevCol.size(); i++)
        prevCol[i] = i;

    for (unsigned int i = 0; i < len1; i++) {
        col[0] = i + 1;
        for (unsigned int j = 0; j < len2; j++)
            col[j + 1] = qMin(qMin(1 + col[j], 1 + prevCol[1 + j]),
                              prevCol[j] + (s1[i] == s2[j] ? 0 : 1));
        col.swap(prevCol);
    }
    return prevCol[len2];
}

/************************************************

 ************************************************/
CueTime::CueTime(const QString &str)
{
    if (!str.isEmpty())
        mNull = !parse(str);
}

/************************************************

 ************************************************/
QString CueTime::toString(bool cdQuality) const
{
    if (cdQuality) {
        int min = mCdValue / (60 * 75);
        int sec = (mCdValue - min * 60 * 75) / 75;
        int frm = mCdValue - (min * 60 + sec) * 75;

        return QStringLiteral("%1:%2:%3")
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(frm, 2, 10, QChar('0'));
    }
    else {
        int min  = mHiValue / (60 * 1000);
        int sec  = (mHiValue - min * 60 * 1000) / 1000;
        int msec = mHiValue - (min * 60 + sec) * 1000;

        return QStringLiteral("%1:%2.%3")
                .arg(min, 2, 10, QChar('0'))
                .arg(sec, 2, 10, QChar('0'))
                .arg(msec, 3, 10, QChar('0'));
    }
}

/************************************************

************************************************/
CueTime CueTime::operator+(const CueTime &other) const
{
    CueTime res;
    res.mCdValue = this->mCdValue + other.mCdValue;
    res.mHiValue = this->mHiValue + other.mHiValue;
    res.mNull    = false;
    return res;
}

/************************************************

 ************************************************/
CueTime CueTime::operator-(const CueTime &other) const
{
    CueTime res;
    res.mCdValue = this->mCdValue - other.mCdValue;
    res.mHiValue = this->mHiValue - other.mHiValue;
    res.mNull    = false;
    return res;
}

/************************************************

 ************************************************/
bool CueTime::operator==(const CueTime &other) const
{
    return this->mHiValue == other.mHiValue;
}

/************************************************

 ************************************************/
bool CueTime::operator!=(const CueTime &other) const
{
    return !(this->operator==(other));
}

/************************************************

 ************************************************/
bool CueTime::parse(const QString &str)
{
    static const QRegularExpression regexp = QRegularExpression("\\D");

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList sl = str.split(regexp, QString::KeepEmptyParts);
#else
    QStringList sl = str.split(regexp, Qt::KeepEmptyParts);
#endif

    if (sl.length() < 3)
        return false;

    bool ok;
    int  min = sl[0].toInt(&ok);
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
CueIndex::CueIndex(const QString &str, const QByteArray &file) :
    CueTime(str),
    mFile(file)
{
}

/************************************************

 ************************************************/
bool CueIndex::operator==(const CueIndex &other) const
{
    return CueTime::operator==(other) && mFile == other.mFile;
}

/************************************************

 ************************************************/
bool CueIndex::operator!=(const CueIndex &other) const
{
    return !(this->operator==(other));
}

/************************************************

 ************************************************/
QByteArray leftPart(const QByteArray &line, const char separator)
{
    int n = line.indexOf(separator);
    if (n > -1)
        return line.left(n);
    else
        return line;
}

/************************************************

 ************************************************/
QByteArray rightPart(const QByteArray &line, const char separator)
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
    qRegisterMetaType<DiskState>("DiskState");
}

Messages::Handler *Messages::mHandler = nullptr;

/************************************************
 *
 ************************************************/
QString htmlToText(const QString &html)
{
    static const QRegularExpression re = QRegularExpression("<[^>]*>");

    QString res(html);
    res.replace("<br>", " ", Qt::CaseInsensitive);
    res.remove(re);
    res.replace("\\n", "\n");
    return res;
}

/************************************************
 *
 ************************************************/
void Messages::error(const QString &message)
{
    qCritical() << htmlToText(message);

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
    Windows special symbols
    Use any character, except for the following:
    The following reserved characters:
            < (less than)
            > (greater than)
            : (colon)
            " (double quote)
            / (forward slash)
            \ (backslash)
            | (vertical bar or pipe)
            ? (question mark)
            * (asterisk)
            \0 Integer value zero, sometimes referred to as the ASCII NUL character.

            \1-\31 Characters whose integer representations are in the range from 1 through 31
     https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#file-and-directory-names

    Characters from 1 through 31, except TAB and NEW LINE	remove
    TAB (\t)	replace to " " (space)
    NEW LINE (\n)	replace to " " (space)
    /	replace to - (dash)
    \	replace to - (dash)
    :	replace to _ (underscore)
    *	replace to _ (underscore)
    ?	remove
    <	replace to _
    >	replace to _
    "	replace to ' (single quote)
 ************************************************/
QString safeString(const QString &str)
{
    QString res = "";

    foreach (const QChar c, str) {
        switch (c.unicode()) {
            case '\t':
            case '\n':
                res += " ";
                continue;

            case '\\':
            case '/':
                res += "-";
                continue;

            case ':':
            case '*':
                res += "_";
                continue;

            case '<':
                res += "[";
                continue;
            case '>':
                res += "]";
                continue;

            case '?':
                continue;

            case '"':
                res += "'";
                continue;
        }

        if (c < ' ')
            continue;

        res += c;
    }

    if (res == ".")
        return "_";

    if (res == "..")
        return "__";

    return res;
}

/************************************************
 *
 ************************************************/
QString debugProgramArgs(const QString &prog, const QStringList &args)
{
    QStringList res;

    res << prog;
    foreach (QString arg, args) {
        if (arg.contains(' ') || arg.contains('\t'))
            res << ("'" + arg + "'");
        else
            res << arg;
    }
    return res.join(" ");
}

/************************************************
 *
 ************************************************/
int calcQuality(int input, int preferences, int formatMax)
{
    formatMax   = formatMax ? formatMax : std::numeric_limits<int>::max();
    preferences = preferences ? preferences : std::numeric_limits<int>::max();

    return qMin(input, qMin(preferences, formatMax));
}

/************************************************
 *
 ************************************************/
int calcSampleRate(int input, SampleRate resample)
{
    if (resample == SampleRate::AsSource) {
        return input;
    };

    return qMin(input, int(resample));
}

/************************************************
 *
 ************************************************/
CueFlags::CueFlags(const QString &tag) :
    digitalCopyPermitted(tag.contains("DCP")),
    fourChannel(tag.contains("4CH")),
    preEmphasis(tag.contains("PRE")),
    serialCopyManagement(tag.contains("SCMS"))

{
}

/************************************************
 *
 ************************************************/
QString CueFlags::toString() const
{
    // clang-format off
    QStringList res;
    if (digitalCopyPermitted) { res << "DCP"; }
    if (fourChannel)          { res <<"4CH";  }
    if (preEmphasis)          { res <<"PRE";  }
    if (serialCopyManagement) { res <<"SCMS"; }
    return res.join(" ");
    // clang-format on
}

/************************************************
 *
 ************************************************/
bool CueFlags::isEmpty() const
{
    // clang-format off
    return
        !digitalCopyPermitted &&
        !fourChannel          &&
        !preEmphasis          &&
        !serialCopyManagement ;
    // clang-format on
}

/************************************************
 *
 ************************************************/
DiskState calcDiskState(const QList<TrackState> &trackStates)
{
    uint32_t state = 0;
    for (TrackState s : trackStates) {
        state |= (1 << uint(s));
    }

    if (state == (1 << uint(TrackState::NotRunning))) {
        return DiskState::NotRunning;
    }

    if (state == (1 << uint(TrackState::OK))) {
        return DiskState::OK;
    }

    if (state & (1 << uint(TrackState::Error))) {
        return DiskState::Error;
    }

    if (state & (1 << uint(TrackState::Canceled))) {
        return DiskState::Canceled;
    }

    if (state & (1 << uint(TrackState::Aborted))) {
        return DiskState::Aborted;
    }

    if (state > 0) {
        return DiskState::Running;
    }
    else {
        return DiskState::NotRunning;
    }
}

/************************************************
 *
 ************************************************/
QString expandFilePath(const QString &path)
{
    QString res = path;
    if (res == "~" || res == "~//") {
        return QDir::homePath();
    }

    if (res == ".") {
        res = QDir::currentPath();
    }

    if (res.startsWith("~/")) {
        return res.replace(0, 1, QDir::homePath());
    }

    return QFileInfo(res).absoluteFilePath();
}

QString firstNotNullString(const QString &s1, const QString &s2)
{
    // clang-format off
    if (!s1.isNull()) return s1;
    return s2;
    // clang-format on
}

QString firstNotNullString(const QString &s1, const QString &s2, const QString &s3)
{
    // clang-format off
    if (!s1.isNull()) return s1;
    if (!s2.isNull()) return s2;
    return s3;
    // clang-format on
}

QString firstNotEmptyString(const QString &s1, const QString &s2)
{
    // clang-format off
    if (!s1.isEmpty()) return s1;
    return s2;
    // clang-format on
}

QString firstNotEmptyString(const QString &s1, const QString &s2, const QString &s3)
{
    // clang-format off
    if (!s1.isEmpty()) return s1;
    if (!s2.isEmpty()) return s2;
    return s3;
    // clang-format on
}
