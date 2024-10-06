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
#include <QMetaType>

#define CODEC_AUTODETECT "AUTODETECT"

using DiscNum    = quint16;
using DiscCount  = quint16;
using TrackNum   = quint16;
using TrackCount = quint16;
using Duration   = uint;
using mSec       = int;
using Percent    = quint8;

enum class TagId {
    Album,
    Catalog,
    CDTextfile,
    Comment,
    Date,
    Flags,
    Genre,
    ISRC,
    Artist,
    SongWriter,
    Title,
    DiscId,
    File,
    DiscNum,
    DiscCount,
    CueFile,
    AlbumArtist,
    TrackNum,
    TrackCount,
};

enum class PreGapType {
    Skip = 0,
    ExtractToFile,
    AddToFirstTrack
};

QString    preGapTypeToString(PreGapType type);
PreGapType strToPreGapType(const QString &str, PreGapType def = PreGapType::AddToFirstTrack);

enum class GainType {
    Disable,
    Track,
    Album
};

QString  gainTypeToString(GainType type);
GainType strToGainType(const QString &str, GainType def = GainType::Disable);

enum class CoverMode {
    Disable,
    OrigSize,
    Scale
};

QString   coverModeToString(CoverMode mode);
CoverMode strToCoverMode(const QString &str, CoverMode def = CoverMode::Disable);

struct CoverOptions
{
    CoverMode mode = CoverMode::Disable;
    int       size = 0;
};

unsigned int levenshteinDistance(const QString &s1, const QString &s2);

class FlaconError : public std::runtime_error
{
public:
    explicit FlaconError(const char *msg) :
        std::runtime_error(msg) { }
    explicit FlaconError(const std::string &msg) :
        std::runtime_error(msg) { }
    explicit FlaconError(const QString &msg) :
        std::runtime_error(msg.toStdString()) { }
};

class CueTime
{
public:
    explicit CueTime(const QString &str = "");

    bool    isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueTime operator+(const CueTime &other) const;
    CueTime operator-(const CueTime &other) const;
    bool    operator==(const CueTime &other) const;
    bool    operator!=(const CueTime &other) const;

    uint milliseconds() const { return mHiValue; }
    uint frames() const { return mCdValue; }

private:
    bool mNull    = true;
    int  mCdValue = 0;
    int  mHiValue = 0;

    bool parse(const QString &str);
};

class CueIndex : public ::CueTime
{
public:
    CueIndex() = default;
    explicit CueIndex(const QString &str, const QByteArray &file);

    QByteArray file() const { return mFile; }

    bool isEmpty() const { return mFile.isEmpty(); }
    bool operator==(const CueIndex &other) const;
    bool operator!=(const CueIndex &other) const;

private:
    QByteArray mFile;
};

struct CueFlags
{
public:
    CueFlags()        = default;
    CueFlags &operator=(const CueFlags &other) = default;

    explicit CueFlags(const QString &tag);
    QString toString() const;

    bool isEmpty() const;

    bool digitalCopyPermitted = false; /// DCP – Digital copy permitted
    bool fourChannel          = false; /// 4CH – Four channel audio
    bool preEmphasis          = false; /// PRE – Pre-emphasis enabled (audio tracks only)
    bool serialCopyManagement = false; /// SCMS – Serial copy management system (not supported by all recorders)
};

enum class TrackState {
    NotRunning = 0,
    Canceled   = 1,
    Error      = 2,
    Aborted    = 3,
    OK         = 4,
    Splitting  = 5,
    Encoding   = 6,
    Queued     = 7,
    WaitGain   = 8,
    CalcGain   = 9,
    WriteGain  = 10
};

Q_DECLARE_METATYPE(TrackState)

enum class DiskState {
    NotRunning = 0,
    Canceled   = 1,
    Error      = 2,
    Aborted    = 3,
    OK         = 4,
    Running    = 5,
};

Q_DECLARE_METATYPE(DiskState)

DiskState calcDiskState(const QList<TrackState> &trackStates);

enum BitsPerSample {
    AsSourcee = 0,
    Bit_16    = 16,
    Bit_24    = 24,
    Bit_32    = 32,
    Bit_64    = 64,
};
Q_DECLARE_METATYPE(BitsPerSample)

enum SampleRate {
    AsSource  = 0,
    Hz_44100  = 44100,
    Hz_48000  = 48000,
    Hz_96000  = 96000,
    Hz_192000 = 192000,
    Hz_384000 = 384000,
    Hz_768000 = 768000,
};
Q_DECLARE_METATYPE(SampleRate)

enum class FormatOption {
    NoOptions            = 0x0,
    Lossless             = 0x1,
    SupportGain          = 0x2,
    SupportEmbeddedCue   = 0x4,
    SupportEmbeddedImage = 0x8,
};

Q_DECLARE_FLAGS(FormatOptions, FormatOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(FormatOptions)
inline bool operator&&(const FormatOptions &flags, const FormatOption flag) noexcept
{
    return flags.testFlag(flag);
}

int calcSampleRate(int input, SampleRate resample);

int calcQuality(int input, int preferences, int formatMax);

QByteArray leftPart(const QByteArray &line, const char separator);
QByteArray rightPart(const QByteArray &line, const char separator);
void       initTypes();

QString safeString(const QString &str);
QString debugProgramArgs(const QString &prog, const QStringList &args);

QString htmlToText(const QString &html);

class Messages
{
public:
    static void error(const QString &message);

    class Handler
    {
    public:
        virtual void showErrorMessage(const QString &message) = 0;
    };

    static void setHandler(Handler *handler);

private:
    static Handler *mHandler;
};

QString expandFilePath(const QString &path);

inline QString firstNotNullString(const QString &s1, const QString &s2)
{
    // clang-format off
    if (!s1.isNull()) return s1;
    return s2;
    // clang-format on
}

inline QString firstNotNullString(const QString &s1, const QString &s2, const QString &s3)
{
    // clang-format off
    if (!s1.isNull()) return s1;
    if (!s2.isNull()) return s2;
    return s3;
    // clang-format on
}

#endif // TYPES_H
