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

#include "types.h"
#include "formats_in/informat.h"
#include "settings.h"
#include "inputaudiofile.h"
#include "formats_out/outformat.h"
#include "converter/sox.h"

#include <assert.h>
#include <QtGlobal>
#include <QDir>
#include <QDebug>
#include <QProcessEnvironment>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMetaEnum>
#include <QThread>

#ifdef Q_OS_WIN
#define PATH_ENV_SEPARATOR ';'
#define BINARY_EXT ".exe"

#elif defined(Q_OS_OS2)
#define PATH_ENV_SEPARATOR ';'
#define BINARY_EXT ".exe"

#else
#define PATH_ENV_SEPARATOR ':'
#define BINARY_EXT ""

#endif

#define PROFILES_PREFIX "Profiles"

QString   Settings::mFileName;
Settings *Settings::mInstance = nullptr;

/************************************************

 ************************************************/
Settings *Settings::i()
{
    if (!mInstance) {
        if (mFileName.isEmpty())
            mInstance = new Settings("flacon", "flacon");
        else
            mInstance = new Settings(mFileName);
    }

    return mInstance;
}

/************************************************

 ************************************************/
void Settings::setFileName(const QString &fileName)
{
    mFileName = fileName;
    delete mInstance;
    mInstance = nullptr;
}

/************************************************

 ************************************************/
Settings::Settings(const QString &organization, const QString &application) :
    QSettings(organization, application)
{
    setIniCodec("UTF-8");
    init();
}

/************************************************

 ************************************************/
Settings::Settings(const QString &fileName) :
    QSettings(fileName, QSettings::IniFormat)
{
    setIniCodec("UTF-8");
    init();
}

/************************************************

 ************************************************/
void Settings::init()
{
    setDefaultValue(Tags_DefaultCodepage, "AUTODETECT");

    // Globals **********************************
    setDefaultValue(Encoder_ThreadCount, qMax(4, QThread::idealThreadCount()));
    setDefaultValue(Encoder_TmpDir, "");

    // Out Files ********************************
    setDefaultValue(OutFiles_Profile, "FLAC");

    // Internet *********************************
    setDefaultValue(Inet_CDDBHost, "https://gnudb.gnudb.org/");

    // Misc *************************************
    setDefaultValue(Misc_LastDir, QDir::homePath());

    // ConfigureDialog **********************
    setDefaultValue(ConfigureDialog_Width, 645);
    setDefaultValue(ConfigureDialog_Height, 425);

    mPrograms << Conv::Sox::programName();

    foreach (OutFormat *format, OutFormat::allFormats()) {
        mPrograms << format->encoderProgramName();
        mPrograms << format->gainProgramName();
    }

    foreach (const InputFormat *format, InputFormat::allFormats()) {
        mPrograms << format->decoderProgramName();
    }

    mPrograms.remove("");

    foreach (QString program, mPrograms) {
        if (!checkProgram(program))
            setValue("Programs/" + program, findProgram(program));
    }

    if (!childGroups().contains(PROFILES_PREFIX)) {
        foreach (OutFormat *format, OutFormat::allFormats()) {
            QString group = QString("%1/%2/").arg(PROFILES_PREFIX, format->id());
            setDefaultValue(group + "Format", format->id());
            setDefaultValue(group + "Name", format->name());
        }
    }
}

/************************************************

 ************************************************/
QString Settings::keyToString(Settings::Key key) const
{
    switch (key) {
        case Tags_DefaultCodepage:
            return "Tags/DefaultCodepage";

        // MainWindow **************************
        case MainWindow_Width:
            return "MainWindow/Width";
        case MainWindow_Height:
            return "MainWindow/Height";

        // Globals *****************************
        case Encoder_ThreadCount:
            return "Encoder/ThreadCount";
        case Encoder_TmpDir:
            return "Encoder/TmpDir";

        // Out Files ***************************
        case OutFiles_Profile:
            return "OutFiles/Profile";
        case OutFiles_PatternHistory:
            return "OutFiles/PatternHistory";
        case OutFiles_DirectoryHistory:
            return "OutFiles/DirectoryHistory";

        // Internet ****************************
        case Inet_CDDBHost:
            return "Inet/CDDBHost";

        // Misc *********************************
        case Misc_LastDir:
            return "Misc/LastDirectory";

        // ConfigureDialog **********************
        case ConfigureDialog_Width:
            return "ConfigureDialog/Width";
        case ConfigureDialog_Height:
            return "ConfigureDialog/Height";
    }

    assert(false);
    return "";
}

/************************************************

 ************************************************/
Settings::~Settings()
{
}

/************************************************

 ************************************************/
QVariant Settings::value(Key key, const QVariant &defaultValue) const
{
    return value(keyToString(key), defaultValue);
}

/************************************************

 ************************************************/
QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    return QSettings::value(key, defaultValue);
}

/************************************************
 *
 ************************************************/
QStringList Settings::groups(const QString &parentGroup) const
{
    QStringList res;
    for (const QString &key : allKeys()) {
        if (key.startsWith(parentGroup)) {
            res << key.section("/", 1, 1);
        }
    }
    res.removeDuplicates();
    return res;
}

/************************************************

 ************************************************/
bool Settings::checkProgram(const QString &program) const
{
    QString val = programName(program);

    if (val.isEmpty())
        return false;

    QFileInfo fi(val);
    return fi.exists() && fi.isExecutable();
}

/************************************************

 ************************************************/
QString Settings::programName(const QString &program) const
{
#ifdef MAC_BUNDLE
    return QDir(qApp->applicationDirPath()).absoluteFilePath(program);
#else
    return value("Programs/" + program).toString();
#endif
}

/************************************************

 ************************************************/
QString Settings::findProgram(const QString &program) const
{
    QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(PATH_ENV_SEPARATOR);
    foreach (QString path, paths) {
        QFileInfo fi(path + QDir::separator() + program + BINARY_EXT);
        if (fi.exists() && fi.isExecutable())
            return fi.absoluteFilePath();
    }
    return "";
}

/************************************************
 *
 ************************************************/
OutFormat *Settings::outFormat() const
{
    OutFormat *format = OutFormat::formatForId(currentProfile().formatId());
    if (format)
        return format;

    return OutFormat::allFormats().first();
}

/************************************************
 *
 ************************************************/
QString Settings::tmpDir() const
{
    return value(Encoder_TmpDir).toString();
}

/************************************************
 *
 ************************************************/
void Settings::setTmpDir(const QString &value)
{
    setValue(Encoder_TmpDir, value);
}

/************************************************

 ************************************************/
QString Settings::defaultCodepage() const
{
    return value(Tags_DefaultCodepage).toString();
}

/************************************************

 ************************************************/
void Settings::setDefaultCodepage(const QString &value)
{
    setValue(Tags_DefaultCodepage, value);
}

/************************************************

 ************************************************/
void Settings::setValue(Settings::Key key, const QVariant &value)
{
    setValue(keyToString(key), value);
    emit changed();
}

/************************************************

 ************************************************/
void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
    emit changed();
}

/************************************************

 ************************************************/
void Settings::setDefaultValue(Key key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}

/************************************************

 ************************************************/
void Settings::setDefaultValue(const QString &key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}

/************************************************

 ************************************************/
const Profiles &Settings::profiles() const
{
    if (mProfiles.isEmpty()) {
        const_cast<Settings *>(this)->loadProfiles();
    }

    return mProfiles;
}

/************************************************

 ************************************************/
Profiles &Settings::profiles()
{
    if (mProfiles.isEmpty()) {
        const_cast<Settings *>(this)->loadProfiles();
    }

    return mProfiles;
}

/************************************************
 *
 ************************************************/
void Settings::loadProfiles()
{
    QSet<QString> loaded;
    allKeys();
    beginGroup(PROFILES_PREFIX);
    for (QString id : childGroups()) {

        Profile profile(id);
        profile.load(*this, id);

        if (profile.isValid()) {
            mProfiles << profile;
            loaded << profile.formatId();
        }
    }
    endGroup();
}

/************************************************
 *
 ************************************************/
void Settings::setProfiles(const Profiles &profiles)
{
    allKeys();
    beginGroup(PROFILES_PREFIX);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QSet<QString> old = QSet<QString>::fromList(childGroups());
#else
    // After 5.14.0, QT has stated range constructors are available and preferred.
    // See: https://doc.qt.io/qt-5/qset.html#toList
    QList<QString> groups = childGroups();
    QSet<QString>  old    = QSet<QString>(groups.begin(), groups.end());
#endif

    for (const Profile &profile : profiles) {
        old.remove(profile.id());
        profile.save(*this, profile.id());
    }

    for (const QString &id : old) {
        remove(id);
    }
    endGroup();
    mProfiles.clear();
}

/************************************************
 *
 ************************************************/
const Profile &Settings::currentProfile() const
{
    int n = profiles().indexOf(value(OutFiles_Profile).toString());
    if (n > -1) {
        return profiles()[qMax(0, n)];
    }

    return NullProfile();
}

/************************************************
 *
 ************************************************/
Profile &Settings::currentProfile()
{

    int n = profiles().indexOf(value(OutFiles_Profile).toString());
    if (n > -1) {
        return profiles()[qMax(0, n)];
    }

    return NullProfile();
}

/************************************************
 *
 ************************************************/
bool Settings::selectProfile(const QString &profileId)
{
    if (profiles().indexOf(profileId) < 0)
        return false;

    setValue(OutFiles_Profile, profileId);
    return true;
}

/************************************************
 *
 ************************************************/
uint Settings::encoderThreadsCount() const
{
    return value(Encoder_ThreadCount).toUInt();
}

/************************************************
 *
 ************************************************/
void Settings::setEncoderThreadsCount(uint value)
{
    setValue(Encoder_ThreadCount, value);
}

/************************************************
 *
 ************************************************/
QString Settings::cddbHost() const
{
    return value(Inet_CDDBHost).toString();
}

/************************************************
 *
 ************************************************/
void Settings::setCddbHost(const QString &value)
{
    setValue(Inet_CDDBHost, value);
}
