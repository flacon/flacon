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
#include <QSet>
//#include "types.h"
#include "profiles.h"

class OutFormat;

class Settings : public QSettings
{
public:
    static Settings *i();

    static QString fileName() { return mFileName; }
    static void    setFileName(const QString &fileName);

    void     extracted(Profiles &res);
    Profiles readProfiles();
    void     writeProfiles(const Profiles &profiles);

    QString readCurrentProfileId() const;
    void    writeCurrentProfileId(const QString &profileId);

    // REMOVE ================
    QString programName(const QString &program) const { return ""; }
    QString programPath(const QString &program) const { return ""; }
    QString findProgram(const QString &program) const { return ""; }
    bool    checkProgram(const QString &program, QStringList *errors = nullptr) const { return true; }
    // REMOVE ================
private:
    static QString   mFileName;
    static Settings *mInstance;

    explicit Settings(const QString &organization, const QString &application);
    explicit Settings(const QString &fileName);

    Profile readProfile(const QString &profileId);
    void    writeProfile(const Profile &profile);

    BitsPerSample readBitsPerSample(const QString &key, BitsPerSample def) const;
    SampleRate    readSampleRate(const QString &key, SampleRate def) const;
    uint          readThreadsCount(const QString &key, uint def) const;
};

class Settings_OLD : public QSettings
{
    Q_OBJECT
public:
    enum Key {
        // Tags_DefaultCodepage,

        // Globals ******************************
        // Encoder_ThreadCount,
        // Encoder_TmpDir,

        // Out Files ****************************
        OutFiles_DirectoryHistory,
        OutFiles_Profile,
        OutFiles_PatternHistory,

        // Misc *********************************
        Misc_LastDir,

        // ConfigureDialog **********************
        ConfigureDialog_Width,
        ConfigureDialog_Height,
    };

    explicit Settings_OLD(const QString &fileName);
    virtual ~Settings_OLD();

    static Settings_OLD *i();
    static void          setFileName(const QString &fileName);
    static QString       fileName() { return mFileName; }

    QVariant value(Key key, const QVariant &defaultValue = QVariant()) const;
    void     setValue(Key key, const QVariant &value);

    void     setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    QString programName(const QString &program) const;
    QString programPath(const QString &program) const;
    QString findProgram(const QString &program) const;
    bool    checkProgram(const QString &program, QStringList *errors = nullptr) const;

    QSet<QString> programs() const { return mPrograms; }

protected:
    explicit Settings_OLD(const QString &organization, const QString &application);

private:
    void        init();
    void        setDefaultValue(const QString &key, const QVariant &defaultValue);
    void        setDefaultValue(Key key, const QVariant &defaultValue);
    QString     keyToString(Key key) const;
    QStringList groups(const QString &parentGroup) const;
    void        loadProfiles();
    // void        initProfiles();

    QSet<QString>        mPrograms;
    static QString       mFileName;
    static Settings_OLD *mInstance;
    Profiles             mProfiles;
};

#endif // SETTINGS_H
