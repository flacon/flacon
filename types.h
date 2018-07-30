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

#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QIcon>

#define CODEC_AUTODETECT "AUTODETECT"

typedef quint16 DiskNum;
typedef quint16 TrackNum;
typedef uint    Duration;

enum class PreGapType
{
    Skip,
    ExtractToFile,
    AddToFirstTrack
};

QString preGapTypeToString(PreGapType type);
PreGapType strToPreGapType(const QString &str);


enum class GainType
{
    Disable,
    Track,
    Album
};

QString gainTypeToString(GainType type);
GainType strToGainType(const QString &str);


enum class CoverMode
{
    Disable,
    OrigSize,
    Scale
};

QString coverModeToString(CoverMode mode);
CoverMode strToCoverMode(const QString &str);

typedef quint16 DiskNum;
typedef quint16 TrackNum;

unsigned int levenshteinDistance(const QString &s1, const QString & s2);
QIcon loadIcon(const QString &iconName, bool loadDisable = true);

class FlaconError: public std::exception
{
public:
    FlaconError(const QString &msg):
        std::exception(),
        mMsg(msg)
    {
    }

    const char* what() const noexcept
    {
        return mMsg.toLocal8Bit().constData();
    }

    QString message()  const noexcept
    {
        return mMsg;
    }

protected:
    QString mMsg;
};


class CueIndex
{
public:
    explicit CueIndex(const QString &str = "");

    bool isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueIndex operator-(const CueIndex &other) const;
    bool operator==(const CueIndex &other) const;
    bool operator!=(const CueIndex &other) const;

    uint milliseconds() const { return mHiValue; }
    uint frames() const { return mCdValue; }

private:
    bool mNull;
    int mCdValue;
    int mHiValue;

    bool parse(const QString &str);
};

typedef CueIndex CueTime;


enum class TrackState
{
    NotRunning  = 0,
    Canceled    = 1,
    Error       = 2,
    Aborted     = 3,
    OK          = 4,
    Splitting   = 5,
    Encoding    = 6,
    Queued      = 7,
    WaitGain    = 8,
    CalcGain    = 9,
    WriteGain   = 10
};

typedef quint8 Percent;
typedef quint64 TrackId;

QByteArray leftPart(const QByteArray &line, const QChar separator);
QByteArray rightPart(const QByteArray &line, const QChar separator);
void initTypes();

#endif // TYPES_H
