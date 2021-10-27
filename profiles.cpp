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

#include "profiles.h"
#include "formats_out/outformat.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "encoder.h"
#include "gain.h"

QHash<QString, QVariant> &operator<<(QHash<QString, QVariant> &values, const QHash<QString, QVariant> &other)
{
    for (auto i = other.constBegin(); i != other.constEnd(); ++i) {
        if (!values.contains(i.key()))
            values.insert(i.key(), i.value());
    }
    return values;
}

class Encoder_Null : public Conv::Encoder
{
public:
    QString     programName() const override { return ""; }
    QStringList programArgs() const override { return QStringList(); }
};

class OutFormat_Null : public OutFormat
{
public:
    OutFormat_Null()
    {
        mId   = "";
        mExt  = "";
        mName = "";
    }

    virtual QString gainProgramName() const override { return ""; }

    QHash<QString, QVariant> defaultParameters() const override
    {
        return QHash<QString, QVariant>();
    }

    EncoderConfigPage *configPage(QWidget *) const override
    {
        return nullptr;
    }

    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::AsSourcee; }
    virtual SampleRate    maxSampleRate() const override { return SampleRate::AsSource; }

    Conv::Encoder *createEncoder() const override { return new Encoder_Null(); }
    Conv::Gain *   createGain(const Profile &profile) const override { return new Conv::NoGain(profile); }
};

static OutFormat_Null *nullFormat()
{
    static OutFormat_Null res;
    return &res;
}

/************************************************
 *
 ************************************************/
Profile::Profile() :
    mFormat(nullFormat())
{
    setDefaultValues();
}

/************************************************
 *
 ************************************************/
Profile::Profile(const QString &id) :
    mId(id),
    mFormat(nullFormat())
{
    setDefaultValues();
}

/************************************************
 *
 ************************************************/
Profile::Profile(OutFormat &format, const QString &id) :
    mId(id.isEmpty() ? format.id() : id),
    mFormat(&format)
{
    mName = format.name();
    setDefaultValues();
    mValues << format.defaultParameters();
}

/************************************************
 *
 ************************************************/
void Profile::setDefaultValues()
{
    QString outDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (outDir.isEmpty())
        outDir = "~/Music";

    outDir.replace(QDir::homePath(), "~");

    mValues[OUT_DIRECTORY_KEY]   = outDir;
    mValues[OUT_PATTERN_KEY]     = "%a/{%y - }%A/%n - %t";
    mValues[BITS_PER_SAMPLE_KEY] = 0;
    mValues[SAMPLE_RATE_KEY]     = 0;
    mValues[CREATE_CUE_KEY]      = false;
    mValues[CUE_FILE_NAME_KEY]   = "%a-%A.cue";
    mValues[PREGAP_TYPE_KEY]     = preGapTypeToString(PreGapType::ExtractToFile);
}

/************************************************
 *
 ************************************************/
CoverOptions Profile::embedCoverOptions() const
{
    return mEmbedCoverOptions;
}

/************************************************
 *
 ************************************************/
void Profile::setEmbedCoverOptions(const CoverOptions &embedCoverOptions)
{
    mEmbedCoverOptions = embedCoverOptions;
}

/************************************************
 *
 ************************************************/
CoverOptions Profile::copyCoverOptions() const
{
    return mCopyCoverOptions;
}

/************************************************
 *
 ************************************************/
void Profile::setCopyCoverOptions(const CoverOptions &copyCoverOptions)
{
    mCopyCoverOptions = copyCoverOptions;
}

/************************************************
 *
 ************************************************/
void Profile::setName(const QString &value)
{
    mName = value;
}

/************************************************
 *
 ************************************************/
QVariant Profile::value(const QString &key, const QVariant &defaultValue) const
{
    QVariant res = mValues.value(key);
    if (res.isNull())
        res = defaultValue;

    return res;
}

/************************************************
 *
 ************************************************/
void Profile::setValue(const QString &key, const QVariant &value)
{
    mValues.insert(key, value);
}

/************************************************
 *
 ************************************************/
bool Profile::isValid() const noexcept
{
    return !mId.isEmpty() && !mFormat->id().isEmpty();
}

/************************************************
 *
 ************************************************/
QString Profile::outFileDir() const
{
    return value(OUT_DIRECTORY_KEY).toString();
}

/************************************************
 *
 ************************************************/
void Profile::setOutFileDir(const QString &value)
{
    setValue(OUT_DIRECTORY_KEY, value);
}

/************************************************
 *
 ************************************************/
QString Profile::outFilePattern() const
{
    return value(OUT_PATTERN_KEY).toString();
}

/************************************************
 *
 ************************************************/
void Profile::setOutFilePattern(const QString &value)
{
    setValue(OUT_PATTERN_KEY, value);
}

/************************************************
 *
 ************************************************/
GainType Profile::gainType() const
{
    QString s = value(REPLAY_GAIN_KEY).toString();
    return strToGainType(s);
}

/************************************************
 *
 ************************************************/
void Profile::setGainType(GainType value)
{
    setValue(REPLAY_GAIN_KEY, gainTypeToString(value));
}

/************************************************
 *
 ************************************************/
int Profile::bitsPerSample() const
{
    return value(BITS_PER_SAMPLE_KEY, int(BitsPerSample::AsSourcee)).toInt();
}

/************************************************
 *
 ************************************************/
void Profile::setBitsPerSample(int value)
{
    setValue(BITS_PER_SAMPLE_KEY, value);
}

/************************************************
 *
 ************************************************/
SampleRate Profile::sampleRate() const
{
    return SampleRate(value(SAMPLE_RATE_KEY, int(SampleRate::AsSource)).toInt());
}

/************************************************
 *
 ************************************************/
void Profile::setSampleRate(SampleRate value)
{
    setValue(SAMPLE_RATE_KEY, value);
}

/************************************************
 *
 ************************************************/
bool Profile::isCreateCue() const
{
    return value(CREATE_CUE_KEY, false).toBool();
}

/************************************************
 *
 ************************************************/
void Profile::setCreateCue(bool value)
{
    setValue(CREATE_CUE_KEY, value);
}

/************************************************
 *
 ************************************************/
bool Profile::isEmbedCue() const
{
    return value(EMBED_CUE_KEY, false).toBool();
}

/************************************************
 *
 ************************************************/
void Profile::setEmbedCue(bool value)
{
    setValue(EMBED_CUE_KEY, value);
}

/************************************************
 *
 ************************************************/
QString Profile::cueFileName() const
{
    return value(CUE_FILE_NAME_KEY).toString();
}

/************************************************
 *
 ************************************************/
void Profile::setCueFileName(const QString &value)
{
    setValue(CUE_FILE_NAME_KEY, value);
}

/************************************************
 *
 ************************************************/
PreGapType Profile::preGapType() const
{
    return strToPreGapType(value(PREGAP_TYPE_KEY).toString());
}

/************************************************
 *
 ************************************************/
void Profile::setPregapType(PreGapType value)
{
    setValue(PREGAP_TYPE_KEY, preGapTypeToString(value));
}

/************************************************
 *
 ************************************************/
EncoderConfigPage *Profile::configPage(QWidget *parent) const
{
    return mFormat->configPage(parent);
}

/************************************************
 *
 ************************************************/
void Profile::load(QSettings &settings, const QString &group)
{
    settings.beginGroup(group);

    if (settings.contains("Format")) {
        auto *fmt = OutFormat::formatForId(settings.value("Format").toString());
        if (fmt) {
            mFormat = fmt;
            mValues << fmt->defaultParameters();
        }
        else {
            mFormat = nullFormat();
        }
    }

    if (settings.contains("Name")) {
        setName(settings.value("Name").toString());
    }
    else {
        setName(mFormat->name());
    }

    for (QString key : settings.allKeys()) {
        QString uKey = key.toUpper();

        if (uKey == "NAME")
            continue;
        if (uKey == "FORMAT")
            continue;

        setValue(key, settings.value(key));
    }

    settings.endGroup();

    mCopyCoverOptions.mode = strToCoverMode(value(COVER_FILE_MODE_KEY).toString());
    mCopyCoverOptions.size = value(COVER_FILE_SIZE_KEY).toInt();

    mEmbedCoverOptions.mode = strToCoverMode(value(COVER_EMBED_MODE_KEY).toString());
    mEmbedCoverOptions.size = value(COVER_EMBED_SIZE_KEY).toInt();
}

/************************************************
 *
 ************************************************/
void Profile::save(QSettings &settings, const QString &group) const
{
    settings.beginGroup(group);
    settings.setValue("Name", name());
    settings.setValue("Format", formatId());

    for (auto i = mValues.constBegin(); i != mValues.constEnd(); ++i) {
        settings.setValue(i.key(), i.value());
    }

    settings.setValue(COVER_FILE_MODE_KEY, coverModeToString(mCopyCoverOptions.mode));
    settings.setValue(COVER_FILE_SIZE_KEY, mCopyCoverOptions.size);

    settings.setValue(COVER_EMBED_MODE_KEY, coverModeToString(mEmbedCoverOptions.mode));
    settings.setValue(COVER_EMBED_SIZE_KEY, mEmbedCoverOptions.size);

    settings.endGroup();
}

/************************************************
 *
 ************************************************/
int Profiles::indexOf(const QString &id, int from) const
{
    for (int i = from; i < count(); ++i) {
        if (at(i).id() == id)
            return i;
    }
    return -1;
}

/************************************************
 *
 ************************************************/
bool Profiles::update(const Profile &profile)
{
    int n = indexOf(profile.id());
    if (n < 0)
        return false;

    this->operator[](n) = profile;
    return true;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug debug, const Profile &profile)
{
    QDebug &dbg = debug.noquote().noquote();
    dbg << "ID:     " << profile.id() << "\n";
    dbg << "Format: " << profile.formatId() << "\n";
    dbg << "Name:   " << profile.name() << "\n";
    dbg << "Valid:  " << profile.isValid() << "\n";
    for (auto i = profile.mValues.constBegin(); i != profile.mValues.constEnd(); ++i) {
        dbg << "  " << i.key() << " = " << i.value() << "\n";
    }

    return debug;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Profiles &profiles)
{
    dbg.nospace().noquote() << profiles.count() << " items .....................\n";
    for (const Profile &p : profiles) {
        dbg << p;
        dbg << "\n";
    }
    dbg.nospace().noquote() << "....................................";
    return dbg.space();
}

/************************************************
 *
 ************************************************/
Profile &NullProfile()
{
    static Profile res;
    return res;
}
