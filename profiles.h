/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019-2020
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

#ifndef PROFILES_H
#define PROFILES_H

#include <QString>
#include <QVariant>
#include <QVector>

#include "formats_out/outformat.h"
#include "types.h"

class QSettings;

class Profile
{
    using EncoderValues = QHash<QString, QVariant>;

public:
    Profile()                     = default;
    Profile(const Profile &other) = default;
    Profile &operator=(const Profile &other) = default;
    virtual ~Profile()                       = default;

    Profile(const QString &formatId, const QString &id = "");

    bool isValid() const noexcept;

    QString id() const { return mId; }

    QString name() const { return mName; }
    void    setName(const QString &value);

    QString outFileDir() const { return mOutFileDir; }
    void    setOutFileDir(const QString &value);

    QString outFilePattern() const { return mOutFilePattern; }
    void    setOutFilePattern(const QString &value);

    GainType gainType() const { return mGainType; }
    void     setGainType(GainType value);

    BitsPerSample bitsPerSample() const { return mBitsPerSample; }
    void          setBitsPerSample(BitsPerSample value);

    SampleRate sampleRate() const { return mSampleRate; }
    void       setSampleRate(SampleRate value);

    bool isCreateCue() const { return mCreateCue; }
    void setCreateCue(bool value);

    bool isEmbedCue() const { return mEmbedCue; }
    void setEmbedCue(bool value);

    QString cueFileName() const { return mCueFileName; }
    void    setCueFileName(const QString &value);

    PreGapType pregapType() const { return mPregapType; }
    void       setPregapType(PreGapType value);

    CoverOptions copyCoverOptions() const { return mCopyCoverOptions; }
    void         setCopyCoverOptions(const CoverOptions &value);

    CoverOptions embedCoverOptions() const { return mEmbedCoverOptions; }
    void         setEmbedCoverOptions(const CoverOptions &value);

    EncoderValues encoderValues() const { return mEncoderValues; }
    void          setEncoderValues(const EncoderValues &values);

    QVariant encoderValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void     setEncoderValue(const QString &key, const QVariant &value);

    const OutFormat *outFormat() const { return mFormat; }
    QString          formatId() const { return mFormat->id(); }
    QString          formatName() const { return mFormat->name(); }
    QString          ext() const { return mFormat->ext(); }
    FormatOptions    formatOptions() const { return mFormat->options(); }
    BitsPerSample    maxBitPerSample() const { return mFormat->maxBitPerSample(); }
    SampleRate       maxSampleRate() const { return mFormat->maxSampleRate(); }

    QString tmpDir() const { return globalParams().mTmpDir; }
    void    setTmpDir(const QString &value);

    uint encoderThreadsCount() const { return globalParams().mEncoderThreadsCount; }
    void setEncoderThreadsCount(uint value);

    QString resultFileName(const Track *track) const;
    QString resultFileDir(const Track *track) const;
    QString resultFilePath(const Track *track) const;

private:
    QString mId;
    QString mName;
    QString mOutFileDir     = defaultOutFileDir();
    QString mOutFilePattern = "%a/{%y - }%A/%n - %t";
    QString mCueFileName    = "%a-%A.cue";

    GainType      mGainType      = GainType::Disable;
    BitsPerSample mBitsPerSample = BitsPerSample::AsSourcee;
    SampleRate    mSampleRate    = SampleRate::AsSource;
    PreGapType    mPregapType    = PreGapType::ExtractToFile;

    bool mCreateCue = false;
    bool mEmbedCue  = false;

    OutFormat *mFormat = nullptr;

    CoverOptions mCopyCoverOptions  = { CoverMode::Disable, 1024 };
    CoverOptions mEmbedCoverOptions = { CoverMode::Disable, 1024 };

    EncoderValues mEncoderValues;

    struct GlobalParams
    {
        QString mTmpDir;
        uint    mEncoderThreadsCount = defaultEncoderThreadCount();
    };

    static GlobalParams &globalParams();

    static QString defaultOutFileDir();
    static uint    defaultEncoderThreadCount();

    QString safeFilePathLen(const QString &path) const;
    QString calcResultFilePath(const Track *track) const;
};

class Profiles : public QVector<Profile>
{
public:
    int            indexOf(const QString &id, int from = 0) const;
    Profile       *find(const QString &id);
    const Profile *find(const QString &id) const;
};

Profiles createStandardProfiles();

QDebug operator<<(QDebug dbg, const Profile &profile);
QDebug operator<<(QDebug dbg, const Profiles &profiles);

#endif // PROFILES_H
