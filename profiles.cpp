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
#include "outformat.h"
#include <QSettings>
#include <QDebug>

QHash<QString, QVariant> &operator<<(QHash<QString, QVariant> &values, const QHash<QString, QVariant> &other)
{
    for (auto i= other.constBegin(); i!=other.constEnd(); ++i) {
        if (!values.contains(i.key()))
            values.insert(i.key(), i.value());
    }
    return values;
}


class OutFormat_Null: public OutFormat
{
public:
    OutFormat_Null()
    {
        mId   = "";
        mExt  = "";
        mName = "";
    }

    virtual QString encoderProgramName() const override { return ""; }
    virtual QString gainProgramName() const override { return ""; }

    virtual QStringList encoderArgs(const Profile &, const Track *, const QString &) const override
    {
        return QStringList();
    }

    virtual QStringList gainArgs(const QStringList &, const GainType) const override
    {
        return QStringList();
    }

    QHash<QString, QVariant> defaultParameters() const override
    {
        return QHash<QString, QVariant>();
    }

    EncoderConfigPage *configPage(const Profile &, QWidget *) const override
    {
        return nullptr;
    }

    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::AsSourcee; }
    virtual SampleRate    maxSampleRate()   const override { return SampleRate::AsSource; }
};

static OutFormat_Null *nullFormat() {
    static OutFormat_Null res;
    return &res;
}


/************************************************
 *
 ************************************************/
Profile::Profile():
    mFormat(nullFormat())
{
    setDefaultValues();
}


/************************************************
 *
 ************************************************/
Profile::Profile(const QString &id):
    mId(id),
    mFormat(nullFormat())
{
    setDefaultValues();
}


/************************************************
 *
 ************************************************/
Profile::Profile(OutFormat &format, const QString &id):
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
    mValues[BITS_PER_SAMPLE_KEY] = 0;
    mValues[SAMPLE_RATE_KEY]     = 0;
    mValues[CREATE_CUE_KEY]      = false;
    mValues[CUE_FILE_NAME_KEY]   = "%a-%A.cue";
    mValues[PREGAP_TYPE_KEY]     = preGapTypeToString(PreGapType::ExtractToFile);
}


/************************************************
 *
 ************************************************/
Profile::Profile(const Profile &other):
    mFormat(other.mFormat)
{
    operator=(other);
}


/************************************************
 *
 ************************************************/
Profile &Profile::operator =(const Profile &other)
{
    if (this != &other) {
        mId       = other.mId;
        mFormat   = other.mFormat;
        mName     = other.mName;
        mValues   = other.mValues;
    }
    return *this;
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
    return !mId.isEmpty() &&
           !mFormat->id().isEmpty();
}


/************************************************
 *
 ************************************************/
GainType Profile::gainType() const
{
    QString s = value("ReplayGain").toString();
    return strToGainType(s);
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
int Profile::sampleRate() const
{
    return value(SAMPLE_RATE_KEY, int(SampleRate::AsSource)).toInt();
}



/************************************************
 *
 ************************************************/
void Profile::setSampleRate(int value)
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
    return mFormat->configPage(*this, parent);
}


/************************************************
 *
 ************************************************/
void Profile::load(QSettings &settings, const QString &group)
{
    settings.beginGroup(group);

    if (settings.contains("Format")) {
        auto *fmt = OutFormat::formatForId(settings.value("Format").toString());
        mFormat = fmt ? fmt : nullFormat();
        mValues << fmt->defaultParameters();
    }


    if (settings.contains("Name")) {
        setName(settings.value("Name").toString());
    }
    else {
        setName(mFormat->name());
    }

    for (QString key: settings.allKeys()) {
        QString uKey = key.toUpper();

        if (uKey == "NAME")   continue;
        if (uKey == "FORMAT") continue;

        setValue(key, settings.value(key));
    }

    settings.endGroup();
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
    settings.endGroup();
}


/************************************************
 *
 ************************************************/
int Profiles::indexOf(const QString &id, int from) const
{
    for (int i=from; i<count(); ++i) {
        if (at(i).id() == id)
            return i;
    }
    return -1;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Profile &profile)
{
    dbg.nospace().noquote() << "ID:     " << profile.id()       << "\n";
    dbg.nospace().noquote() << "Format: " << profile.formatId() << "\n";
    dbg.nospace().noquote() << "Name:   " << profile.name()     << "\n";
    dbg.nospace().noquote() << "Valid:  " << profile.isValid()  << "\n";
    for (auto i = profile.mValues.constBegin(); i != profile.mValues.constEnd(); ++i) {
        dbg.nospace().noquote() << "  " << i.key() << " = " << i.value() << "\n";
    }

    return dbg.space();
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Profiles &profiles)
{
    dbg.nospace().noquote() << profiles.count() << " items .....................\n";
    for (const Profile &p: profiles) {
        dbg << p;
        dbg << "\n";
    }
    dbg.nospace().noquote() << "....................................";
    return dbg.space();
}
