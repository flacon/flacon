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


/************************************************
 *
 ************************************************/
Profile::Profile()
{
}


/************************************************
 *
 ************************************************/
Profile::Profile(const QString &id):
    mId(id)
{

}


/************************************************
 *
 ************************************************/
Profile::Profile(const Profile &other)
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
        mFormatId = other.mFormatId;
        mName     = other.mName;
        mValues   = other.mValues;
    }
    return *this;
}


/************************************************
 *
 ************************************************/
void Profile::setName(const QString value)
{
    mName = value;
}


/************************************************
 *
 ************************************************/
void Profile::setFormatId(QString formatId)
{
    mFormatId = formatId;
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
OutFormat *Profile::format() const
{
    return OutFormat::formatForId(mFormatId);
}


/************************************************
 *
 ************************************************/
bool Profile::isValid() const noexcept
{
    return !mId.isEmpty() &&
            format() != nullptr;
}


/************************************************
 *
 ************************************************/
void Profile::load(QSettings &settings, QString group)
{
    settings.beginGroup(group);
    for (QString key: settings.allKeys()) {
        QString uKey = key.toUpper();

        if (uKey == "NAME") {
            setName(settings.value(key).toString());
            continue;
        }

        if (uKey == "FORMAT") {
            setFormatId(settings.value(key).toString());
            continue;
        }

        setValue(key, settings.value(key));
    }
    settings.endGroup();

    if (name().isEmpty() && format()) {
        setName(format()->name());
    }
}


/************************************************
 *
 ************************************************/
void Profile::save(QSettings &settings, QString group)
{
    settings.beginGroup(group);
    settings.setValue("Name", name());
    settings.setValue("Format", formatId());

    for (auto i = mValues.constBegin(); i != mValues.constEnd(); ++i) {
        settings.setValue(i.key(), i.value());
    }
    settings.endGroup();
}
