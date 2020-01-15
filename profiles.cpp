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

class OutFormat_Null: public OutFormat
{
public:
    OutFormat_Null()
    {
        mId   = "";
        mExt  = "";
        mName = "";
        mSettingsGroup = "NULL_FORMAT";
    }

    virtual QString encoderProgramName() const override { return ""; }
    virtual QString gainProgramName() const override { return ""; }

    virtual QStringList encoderArgs(const Track *, const QString &) const override
    {
        return QStringList();
    }

    virtual QStringList gainArgs(const QStringList &) const override
    {
        return QStringList();
    }

    QHash<QString, QVariant> defaultParameters() const override
    {
        return QHash<QString, QVariant>();
    }

    EncoderConfigPage *configPage(Profile *, QWidget *) const override
    {
        return nullptr;
    }

    virtual bool hasConfigPage() const override { return false; }

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
}


/************************************************
 *
 ************************************************/
Profile::Profile(const QString &id):
    mId(id),
    mFormat(nullFormat())
{

}


/************************************************
 *
 ************************************************/
Profile::Profile(OutFormat &format, const QString &id):
    mId(id.isEmpty() ? format.id() : id),
    mFormat(&format)
{
    mName = format.name();
    mValues = format.defaultParameters();
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
EncoderConfigPage *Profile::configPage(QWidget *parent)
{
    return mFormat->configPage(this, parent);
}


/************************************************
 *
 ************************************************/
void Profile::load(QSettings &settings, const QString &group)
{
    settings.beginGroup(group);
    for (QString key: settings.allKeys()) {
        QString uKey = key.toUpper();

        if (uKey == "NAME") {
            setName(settings.value(key).toString());
            continue;
        }

        if (uKey == "FORMAT") {
            auto *fmt = OutFormat::formatForId(settings.value(key).toString());
            mFormat = fmt ? fmt : nullFormat();
            continue;
        }

        setValue(key, settings.value(key));
    }
    settings.endGroup();

    if (name().isEmpty()) {
        setName(mFormat->name());
    }
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
