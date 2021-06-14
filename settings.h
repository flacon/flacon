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
#include "types.h"
#include "profiles.h"

class OutFormat;

class Settings : public QSettings
{
    Q_OBJECT
public:
    enum Key {
        Tags_DefaultCodepage,

        // MainWindow ***************************
        MainWindow_Width,
        MainWindow_Height,

        // Globals ******************************
        Encoder_ThreadCount,
        Encoder_TmpDir,

        // Out Files ****************************
        OutFiles_DirectoryHistory,
        OutFiles_Profile,
        OutFiles_PatternHistory,

        // Internet *****************************
        Inet_CDDBHost,

        // Misc *********************************
        Misc_LastDir,

        // ConfigureDialog **********************
        ConfigureDialog_Width,
        ConfigureDialog_Height,

        // Cover image **************************
        Cover_Mode,
        Cover_Size,
        EmbededCover_Mode,
        EmbededCover_Size,

    };

    static Settings *i();
    static void      setFileName(const QString &fileName);
    static QString   fileName() { return mFileName; }

    QVariant value(Key key, const QVariant &defaultValue = QVariant()) const;
    void     setValue(Key key, const QVariant &value);

    void     setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    bool    checkProgram(const QString &program) const;
    QString programName(const QString &program) const;

    QSet<QString> programs() const { return mPrograms; }
    QString       findProgram(const QString &program) const;

    OutFormat *outFormat() const;

    QString tmpDir() const;
    void    setTmpDir(const QString &value);

    QString defaultCodepage() const;
    void    setDefaultCodepage(const QString &value);

    CoverMode coverMode() const;
    void      setCoverMode(CoverMode value);

    int  coverImageSize() const;
    void setCoverImageSize(int value);

    CoverMode embededCoverMode() const;
    void      setEmbededCoverMode(CoverMode value);

    int  embededCoverImageSize() const;
    void setEmbededCoverImageSize(int value);

    Profiles &      profiles();
    const Profiles &profiles() const;
    void            setProfiles(const Profiles &profiles);

    const Profile &currentProfile() const;
    Profile &      currentProfile();
    bool           selectProfile(const QString &profileId);

signals:
    void changed();

protected:
    explicit Settings(const QString &organization, const QString &application);
    explicit Settings(const QString &fileName);
    virtual ~Settings();

private:
    void        init();
    void        setDefaultValue(const QString &key, const QVariant &defaultValue);
    void        setDefaultValue(Key key, const QVariant &defaultValue);
    QString     keyToString(Key key) const;
    QStringList groups(const QString &parentGroup) const;
    void        loadProfiles();

    QSet<QString>    mPrograms;
    static QString   mFileName;
    static Settings *mInstance;
    Profiles         mProfiles;
};

#endif // SETTINGS_H
