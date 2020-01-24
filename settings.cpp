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
#include "formats/informat.h"
#include "settings.h"
#include "inputaudiofile.h"
#include "outformat.h"
#include "converter/resampler.h"

#include <QDir>
#include <QDebug>
#include <QProcessEnvironment>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMetaEnum>

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

#define PROFILES_PREFIX "Profiles/"

QString Settings::mFileName;
Settings *Settings::mInstance = nullptr;

/************************************************

 ************************************************/
Settings *Settings::i()
{
    if (!mInstance)
    {
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
Settings::Settings(const QString &fileName):
    QSettings(fileName, QSettings::IniFormat)
{
    setIniCodec("UTF-8");
    init();
}


/************************************************

 ************************************************/
void Settings::init()
{

    setDefaultValue(Tags_DefaultCodepage,   "AUTODETECT");

    // Globals **********************************
    setDefaultValue(Encoder_ThreadCount,    8);
    setDefaultValue(Encoder_TmpDir,         "");

    // Out Files ********************************
    setDefaultValue(OutFiles_Pattern,       "%a/{%y - }%A/%n - %t");

    QString outDir = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();

    outDir.replace(QDir::homePath(), "~");
    setDefaultValue(OutFiles_Directory,     outDir);
    setDefaultValue(OutFiles_Profile,       "FLAC");

    // Internet *********************************
    setDefaultValue(Inet_CDDBHost,          "freedb.freedb.org");

    // Misc *************************************
    setDefaultValue(Misc_LastDir,           QDir::homePath());

    // PerTrackCue **************************
    setDefaultValue(PerTrackCue_Create,     false);
    setDefaultValue(PerTrackCue_Pregap,     preGapTypeToString(PreGapType::ExtractToFile));
    setDefaultValue(PerTrackCue_FileName,   "%a-%A.cue");

    // Cover image **************************
    setDefaultValue(Cover_Mode,             coverModeToString(CoverMode::Scale));
    setDefaultValue(Cover_Size,             500);

    // ConfigureDialog **********************
    setDefaultValue(ConfigureDialog_Width,  645);
    setDefaultValue(ConfigureDialog_Height, 425);

    mPrograms << Resampler::programName();

    foreach(OutFormat *format, OutFormat::allFormats())
    {
        QHashIterator<QString, QVariant> i(format->defaultParameters());
        while (i.hasNext())
        {
            i.next();
            setDefaultValue(i.key(), i.value());
        }

        mPrograms << format->encoderProgramName();
        mPrograms << format->gainProgramName();
    }


    foreach (const AudioFormat *format, AudioFormat::inputFormats())
    {
        mPrograms << format->decoderProgramName();
    }

    mPrograms.remove("");

    foreach(QString program, mPrograms)
    {
        if (!checkProgram(program))
            setValue("Programs/" + program, findProgram(program));
    }
}


/************************************************

 ************************************************/
QString Settings::keyToString(Settings::Key key) const
{
    switch (key)
    {
    case Tags_DefaultCodepage:      return "Tags/DefaultCodepage";

    // MainWindow **************************
    case MainWindow_Width:          return "MainWindow/Width";
    case MainWindow_Height:         return "MainWindow/Height";

    // Globals *****************************
    case Encoder_ThreadCount:       return "Encoder/ThreadCount";
    case Encoder_TmpDir:            return "Encoder/TmpDir";

    // Out Files ***************************
    case OutFiles_Pattern:          return "OutFiles/Pattern";
    case OutFiles_Directory:        return "OutFiles/Directory";
    case OutFiles_Profile:          return "OutFiles/Profile";
    case OutFiles_PatternHistory:   return "OutFiles/PatternHistory";
    case OutFiles_DirectoryHistory: return "OutFiles/DirectoryHistory";

    // Internet ****************************
    case Inet_CDDBHost:             return "Inet/CDDBHost";


    // Misc *********************************
    case Misc_LastDir:              return "Misc/LastDirectory";


    // PerTrackCue **************************
    case PerTrackCue_Create:        return "PerTrackCue/Create";
    case PerTrackCue_Pregap:        return "PerTrackCue/Pregap";
    case PerTrackCue_FileName:      return "PerTrackCue/FileName";

    // ConfigureDialog **********************
    case ConfigureDialog_Width:     return "ConfigureDialog/Width";
    case ConfigureDialog_Height:    return "ConfigureDialog/Height";


    // Cover image **************************
    case Cover_Mode:                return "Cover/Mode";
    case Cover_Size:                return "Cover/Size";
    }

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
    for (const QString &key: allKeys()) {
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
    foreach(QString path, paths)
    {
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
bool Settings::createCue() const
{
    return value(PerTrackCue_Create).toBool();
}


/************************************************

 ************************************************/
void Settings::setCreateCue(bool value)
{
    setValue(PerTrackCue_Create, value);
}


/************************************************

 ************************************************/
PreGapType Settings::preGapType() const
{
    return strToPreGapType(value(Settings::PerTrackCue_Pregap).toString());
}


/************************************************

 ************************************************/
void Settings::setPregapType(PreGapType value)
{
    setValue(Settings::PerTrackCue_Pregap, preGapTypeToString(value));
}


/************************************************
 *
 ************************************************/
QString Settings::outFilePattern() const
{
    return value(OutFiles_Pattern).toString();
}


/************************************************
 *
 ************************************************/
void Settings::setOutFilePattern(const QString &value)
{
    setValue(OutFiles_Pattern, value);
}


/************************************************

 ************************************************/
QString Settings::outFileDir() const
{
    return value(OutFiles_Directory).toString();
}


/************************************************

 ************************************************/
void Settings::setOutFileDir(const QString &value)
{
    setValue(OutFiles_Directory, value);
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
 *
 ************************************************/
CoverMode Settings::coverMode() const
{
    return strToCoverMode(value(Cover_Mode).toString());
}


/************************************************
 *
 ************************************************/
int Settings::coverImageSize() const
{
    return value(Cover_Size).toInt();
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
Profiles Settings::profiles() const
{
    if (mProfiles.isEmpty()) {
        const_cast<Settings*>(this)->loadProfiles();
    }

    return mProfiles;
}


/************************************************
 * Added on 6.0.0 release, remove after 8.0.0 release
 ************************************************/
void loadOldFormatValues(Settings *settings, Profile &profile)
{
    settings->beginGroup("Resample");
    profile.setValue("BitsPerSample", settings->value("BitsPerSample", profile.value("BitsPerSample")));
    profile.setValue("SampleRate",    settings->value("SampleRate",    profile.value("SampleRate")));
    settings->endGroup();

    if (profile.formatId() == "WAV")  settings->beginGroup("WAV");
    if (profile.formatId() == "FLAC") settings->beginGroup("Flac");
    if (profile.formatId() == "AAC" ) settings->beginGroup("Aac");
    if (profile.formatId() == "MP3" ) settings->beginGroup("Mp3");
    if (profile.formatId() == "OGG" ) settings->beginGroup("Ogg");
    if (profile.formatId() == "OPUS") settings->beginGroup("Opus");
    if (profile.formatId() == "WV"  ) settings->beginGroup("WV");

    for (const QString &key : settings->allKeys()) {
        profile.setValue(key, settings->value(key, profile.value(key)));
    }

    settings->endGroup();
}


/************************************************
 *
 ************************************************/
void Settings::loadProfiles()
{
    QSet<QString> loaded;
    allKeys();
    beginGroup(PROFILES_PREFIX);
    for (QString id: childGroups()) {

        Profile profile(id);
        profile.load(*this, id);

        if (profile.isValid())
        {
            mProfiles << profile;
            loaded << profile.formatId();
        }
    }
    endGroup();

    for (OutFormat *fmt: OutFormat::allFormats()) {
        if (!loaded.contains(fmt->id())) {
            Profile profile(*fmt);
            loadOldFormatValues(this, profile); // Added on 6.0.0 release, remove after 8.0.0 release
            mProfiles << profile;
        }
    }
}


/************************************************
 *
 ************************************************/
void Settings::setProfiles(const Profiles &profiles)
{
    allKeys();
    beginGroup(PROFILES_PREFIX);
    QSet<QString> old = QSet<QString>::fromList(childGroups());

    for (const Profile &profile: profiles) {
        if (profile.hasConfigPage()) {
            old.remove(profile.id());
            profile.save(*this, profile.id());
        }
    }

    for (const QString &id: old) {
        remove(id);
    }
    endGroup();
    mProfiles.clear();
}


/************************************************
 *
 ************************************************/
Profile &Settings::currentProfile() const
{
    int n = profiles().indexOf(value(OutFiles_Profile).toString());
    return profiles()[qMax(0, n)];
}
