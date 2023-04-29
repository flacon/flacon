/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include "profiles.h"

class OutFormat;

class Settings : public QSettings
{
public:
    static Settings *i();

    static QString fileName() { return mFileName; }
    static void    setFileName(const QString &fileName);

    Profiles readProfiles();
    void     writeProfiles(const Profiles &profiles);

    Profile readProfile(const QString &profileId);

    QString readCurrentProfileId() const;
    void    writeCurrentProfileId(const QString &profileId);

    void readExtPrograms() const;
    void writeExtPrograms();

    QString defaultCodepage() const;
    void    setDefaultCodepage(const QString &value);

protected:
    explicit Settings(const QString &organization, const QString &application);
    explicit Settings(const QString &fileName);

private:
    static QString   mFileName;
    static Settings *mInstance;

    void writeProfile(const Profile &profile);

    BitsPerSample readBitsPerSample(const QString &key, BitsPerSample def) const;
    SampleRate    readSampleRate(const QString &key, SampleRate def) const;
    uint          readThreadsCount(const QString &key, uint def) const;
};

#endif // SETTINGS_H
