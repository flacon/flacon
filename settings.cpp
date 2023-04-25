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

#include "settings.h"

static constexpr auto PROFILES_GROUP = "Profiles";

static constexpr auto CURRENT_PROFILE_ID           = "OutFiles/Profile";
static constexpr auto KNOWN_FORMATS_KEY            = "KnownFormats";
static constexpr auto PROFILE_NAME_KEY             = "Name";
static constexpr auto PROFILE_FORMAT_KEY           = "Format";
static constexpr auto PROFILE_OUT_DIRECTORY_KEY    = "OutDirectory";
static constexpr auto PROFILE_OUT_PATTERN_KEY      = "OutPattern";
static constexpr auto PROFILE_BITS_PER_SAMPLE_KEY  = "BitsPerSample";
static constexpr auto PROFILE_SAMPLE_RATE_KEY      = "SampleRate";
static constexpr auto PROFILE_CREATE_CUE_KEY       = "CreateCue";
static constexpr auto PROFILE_EMBED_CUE_KEY        = "EmbedCue";
static constexpr auto PROFILE_CUE_FILE_NAME_KEY    = "CueFileName";
static constexpr auto PROFILE_PREGAP_TYPE_KEY      = "PregapType";
static constexpr auto PROFILE_REPLAY_GAIN_KEY      = "ReplayGain";
static constexpr auto PROFILE_COVER_FILE_MODE_KEY  = "CoverFile/Mode";
static constexpr auto PROFILE_COVER_FILE_SIZE_KEY  = "CoverFile/Size";
static constexpr auto PROFILE_COVER_EMBED_MODE_KEY = "CoverEmbed/Mode";
static constexpr auto PROFILE_COVER_EMBED_SIZE_KEY = "CoverEmbed/Size";

static constexpr auto DEFAULTCODEPAGE_KEY     = "Tags/DefaultCodepage";
static constexpr auto ENCODER_THREADCOUNT_KEY = "Encoder/ThreadCount";
static constexpr auto ENCODER_TMPDIR_KEY      = "Encoder/TmpDir";

QString   Settings::mFileName;
Settings *Settings::mInstance = nullptr;

/************************************************
 *
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
 *
 ************************************************/
void Settings::setFileName(const QString &fileName)
{
    mFileName = fileName;
    delete mInstance;
    mInstance = nullptr;
}

/************************************************
 *
 ************************************************/
Settings::Settings(const QString &organization, const QString &application) :
    QSettings(organization, application)
{
    setIniCodec("UTF-8");
}
#include <QDir>

/************************************************
 *
 ************************************************/
Settings::Settings(const QString &fileName) :
    QSettings(expandFilePath(fileName), QSettings::IniFormat)
{
    QString s = this->fileName();
    setIniCodec("UTF-8");
}

/************************************************
 *
 ************************************************/
Profile Settings::readProfile(const QString &profileId)
{
    QString group = QString("%1/%2").arg(PROFILES_GROUP, profileId);

    beginGroup(group);

    Profile profile(value(PROFILE_FORMAT_KEY).toString(), profileId);

    if (!profile.outFormat()) {
        endGroup();
        return profile;
    }

    profile.setName(value(PROFILE_NAME_KEY, profile.outFormat()->name()).toString());
    profile.setOutFileDir(value(PROFILE_OUT_DIRECTORY_KEY, profile.outFileDir()).toString());
    profile.setOutFilePattern(value(PROFILE_OUT_PATTERN_KEY, profile.outFilePattern()).toString());
    profile.setCueFileName(value(PROFILE_CUE_FILE_NAME_KEY, profile.cueFileName()).toString());

    profile.setBitsPerSample(readBitsPerSample(PROFILE_BITS_PER_SAMPLE_KEY, profile.bitsPerSample()));
    profile.setSampleRate(readSampleRate(PROFILE_SAMPLE_RATE_KEY, profile.sampleRate()));

    profile.setGainType(strToGainType(value(PROFILE_REPLAY_GAIN_KEY).toString(), profile.gainType()));
    profile.setPregapType(strToPreGapType(value(PROFILE_PREGAP_TYPE_KEY).toString(), profile.pregapType()));

    profile.setCreateCue(value(PROFILE_CREATE_CUE_KEY, profile.isCreateCue()).toBool());
    profile.setEmbedCue(value(PROFILE_EMBED_CUE_KEY, profile.isEmbedCue()).toBool());

    CoverOptions opts = profile.copyCoverOptions();
    opts.mode         = strToCoverMode(value(PROFILE_COVER_FILE_MODE_KEY).toString(), opts.mode);
    opts.size         = value(PROFILE_COVER_FILE_SIZE_KEY, opts.size).toInt();
    profile.setCopyCoverOptions(opts);

    if (profile.outFormat()->options().testFlag(FormatOption::SupportEmbeddedImage)) {
        CoverOptions opts = profile.embedCoverOptions();
        opts.mode         = strToCoverMode(value(PROFILE_COVER_EMBED_MODE_KEY).toString(), opts.mode);
        opts.size         = value(PROFILE_COVER_EMBED_SIZE_KEY, opts.size).toInt();
        profile.setEmbedCoverOptions(opts);
    }

    QHash<QString, QVariant> vals = profile.encoderValues();
    for (auto i = vals.begin(); i != vals.end(); ++i) {
        i.value() = value(i.key(), i.value());
    }
    profile.setEncoderValues(vals);

    endGroup();

    profile.setTmpDir(value(ENCODER_TMPDIR_KEY, profile.tmpDir()).toString());
    profile.setDefaultCodepage(value(DEFAULTCODEPAGE_KEY, profile.defaultCodepage()).toString());
    profile.setEncoderThreadsCount(readThreadsCount(ENCODER_THREADCOUNT_KEY, profile.encoderThreadsCount()));

    return profile;
}

/************************************************
 *
 ************************************************/
void Settings::writeProfile(const Profile &profile)
{
    if (!profile.isValid()) {
        return;
    }

    QString group = QString("%1/%2").arg(PROFILES_GROUP, profile.id());
    beginGroup(group);

    setValue(PROFILE_NAME_KEY, profile.name());
    setValue(PROFILE_OUT_DIRECTORY_KEY, profile.outFileDir());
    setValue(PROFILE_OUT_PATTERN_KEY, profile.outFilePattern());
    setValue(PROFILE_CUE_FILE_NAME_KEY, profile.cueFileName());

    setValue(PROFILE_FORMAT_KEY, profile.outFormat()->id());

    setValue(PROFILE_BITS_PER_SAMPLE_KEY, profile.bitsPerSample());
    setValue(PROFILE_SAMPLE_RATE_KEY, profile.sampleRate());

    setValue(PROFILE_REPLAY_GAIN_KEY, gainTypeToString(profile.gainType()));
    setValue(PROFILE_PREGAP_TYPE_KEY, preGapTypeToString(profile.pregapType()));

    setValue(PROFILE_CREATE_CUE_KEY, profile.isCreateCue());
    setValue(PROFILE_EMBED_CUE_KEY, profile.isEmbedCue());

    setValue(PROFILE_COVER_FILE_MODE_KEY, coverModeToString(profile.copyCoverOptions().mode));
    setValue(PROFILE_COVER_FILE_SIZE_KEY, profile.copyCoverOptions().size);

    if (profile.outFormat()->options().testFlag(FormatOption::SupportEmbeddedImage)) {
        setValue(PROFILE_COVER_EMBED_MODE_KEY, coverModeToString(profile.embedCoverOptions().mode));
        setValue(PROFILE_COVER_EMBED_SIZE_KEY, profile.embedCoverOptions().size);
    }

    QHash<QString, QVariant> vals = profile.encoderValues();
    for (auto i = vals.constBegin(); i != vals.constEnd(); ++i) {
        setValue(i.key(), i.value());
    }
    endGroup();

    setValue(ENCODER_TMPDIR_KEY, profile.tmpDir());
    setValue(DEFAULTCODEPAGE_KEY, profile.defaultCodepage());
    setValue(ENCODER_THREADCOUNT_KEY, profile.encoderThreadsCount());
}

/************************************************
 *
 ************************************************/
Profiles Settings::readProfiles()
{
    Profiles res;
    allKeys();

    beginGroup(PROFILES_GROUP);
    QStringList ids = childGroups();
    endGroup();

    for (const QString &id : ids) {
        Profile profile = readProfile(id);
        if (profile.isValid()) {
            res << profile;
        }
    }

    if (res.isEmpty()) {
        //  If this is the first launch, we create standard profiles for ALL formats
        res << createStandardProfiles();
    }
    else {
        // If a new format has been added in this version of the program,
        // then we create a standard profile for the NEW FORMAT

        // This functionality is introduced in version 8.4, this is a list of formats known in 8.3.
        QStringList def = { "AAC", "FLAC", "MP3", "OGG", "OPUS", "WAV", "WV" };
        QStringList old = value(KNOWN_FORMATS_KEY, def).toStringList();

        Profiles ps = createStandardProfiles();
        for (const Profile &p : ps) {
            if (!old.contains(p.outFormat()->id())) {
                res << p;
            }
        }
    }

    return res;
}

/************************************************
 *
 ************************************************/
BitsPerSample Settings::readBitsPerSample(const QString &key, BitsPerSample def) const
{
    uint n = value(key, int(def)).toUInt();
    switch (n) {
        case BitsPerSample::AsSourcee:
        case BitsPerSample::Bit_16:
        case BitsPerSample::Bit_24:
        case BitsPerSample::Bit_32:
        case BitsPerSample::Bit_64:
            return BitsPerSample(n);
    }
    return def;
}

/************************************************
 *
 ************************************************/
SampleRate Settings::readSampleRate(const QString &key, SampleRate def) const
{
    uint n = value(key, int(def)).toUInt();
    switch (n) {
        case SampleRate::AsSource:
        case SampleRate::Hz_44100:
        case SampleRate::Hz_48000:
        case SampleRate::Hz_96000:
        case SampleRate::Hz_192000:
        case SampleRate::Hz_384000:
        case SampleRate::Hz_768000:
            return SampleRate(n);
    }
    return def;
}

/************************************************
 *
 ************************************************/
uint Settings::readThreadsCount(const QString &key, uint def) const
{
    int res = value(key, def).toInt();
    if (res > 1) {
        return res;
    }

    return def;
}

/************************************************
 *
 ************************************************/
void Settings::writeProfiles(const Profiles &profiles)
{
    allKeys();
    setValue(KNOWN_FORMATS_KEY, OutFormat::allFormatsId());

    remove(PROFILES_GROUP);
    for (const Profile &p : profiles) {
        writeProfile(p);
    }
}

/************************************************
 *
 ************************************************/
QString Settings::readCurrentProfileId() const
{
    return value(CURRENT_PROFILE_ID).toString();
}

/************************************************
 *
 ************************************************/
void Settings::writeCurrentProfileId(const QString &profileId)
{
    setValue(CURRENT_PROFILE_ID, profileId);
}

#include "types.h"
#include "formats_in/informat.h"

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
static constexpr char const *KNOWN_FORMATS = "KnownFormats";

QString       Settings_OLD::mFileName;
Settings_OLD *Settings_OLD::mInstance = nullptr;

/************************************************

 ************************************************/
Settings_OLD *Settings_OLD::i()
{
    if (!mInstance) {
        if (mFileName.isEmpty())
            mInstance = new Settings_OLD("flacon", "flacon_OLD");
        else
            mInstance = new Settings_OLD(mFileName);
    }

    return mInstance;
}

/************************************************

 ************************************************/
void Settings_OLD::setFileName(const QString &fileName)
{
    mFileName = fileName;
    delete mInstance;
    mInstance = nullptr;
}

/************************************************

 ************************************************/
Settings_OLD::Settings_OLD(const QString &organization, const QString &application) :
    QSettings(organization, application)
{
    setIniCodec("UTF-8");
    init();
}

/************************************************

 ************************************************/
Settings_OLD::Settings_OLD(const QString &fileName) :
    QSettings(fileName, QSettings::IniFormat)
{
    setIniCodec("UTF-8");
    init();
}

/************************************************

 ************************************************/
void Settings_OLD::init()
{
    // setDefaultValue(Tags_DefaultCodepage, "AUTODETECT");

    // Globals **********************************
    // setDefaultValue(Encoder_ThreadCount, qMax(4, QThread::idealThreadCount()));
    // setDefaultValue(Encoder_TmpDir, "");

    // Out Files ********************************
    setDefaultValue(OutFiles_Profile, "FLAC");

    // Misc *************************************
    setDefaultValue(Misc_LastDir, QDir::homePath());

    // ConfigureDialog **********************
    setDefaultValue(ConfigureDialog_Width, 645);
    setDefaultValue(ConfigureDialog_Height, 425);

    mPrograms << Conv::Sox::programName();

    foreach (OutFormat *format, OutFormat::allFormats()) {
        mPrograms << format->encoderProgramName();
    }

    foreach (const InputFormat *format, InputFormat::allFormats()) {
        mPrograms << format->decoderProgramName();
    }

    mPrograms.remove("");

    foreach (QString program, mPrograms) {
        if (!checkProgram(program))
            setValue("Programs/" + program, findProgram(program));
    }

    // initProfiles();
}

/************************************************

 ************************************************/
QString Settings_OLD::keyToString(Settings_OLD::Key key) const
{
    switch (key) {
            //        case Tags_DefaultCodepage:
            //            return "Tags/DefaultCodepage";

            // Globals *****************************
            //        case Encoder_ThreadCount:
            //            return "Encoder/ThreadCount";
            //        case Encoder_TmpDir:
            //            return "Encoder/TmpDir";

        // Out Files ***************************
        case OutFiles_Profile:
            return "OutFiles/Profile";
        case OutFiles_PatternHistory:
            return "OutFiles/PatternHistory";
        case OutFiles_DirectoryHistory:
            return "OutFiles/DirectoryHistory";

        // Misc *********************************
        case Misc_LastDir:
            return "Misc/LastDirectory";

        // ConfigureDialog **********************
        case ConfigureDialog_Width:
            return "ConfigureDialog/Width";
        case ConfigureDialog_Height:
            return "ConfigureDialog/Height";

        default:
            return "";
    }

    assert(false);
    return "";
}

/************************************************

 ************************************************/
Settings_OLD::~Settings_OLD()
{
}

/************************************************

 ************************************************/
QVariant Settings_OLD::value(Key key, const QVariant &defaultValue) const
{
    return value(keyToString(key), defaultValue);
}

/************************************************

 ************************************************/
QVariant Settings_OLD::value(const QString &key, const QVariant &defaultValue) const
{
    return QSettings::value(key, defaultValue);
}

/************************************************
 *
 ************************************************/
QStringList Settings_OLD::groups(const QString &parentGroup) const
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
bool Settings_OLD::checkProgram(const QString &program, QStringList *errors) const
{
    QString path = programPath(program);

    if (path.isEmpty()) {
        if (errors) {
            *errors << tr("The %1 program is not installed.<br>Verify that all required programs are installed and in your preferences.",
                          "Error message. %1 - is an program name")
                               .arg(program);
        }
        return false;
    }

    QFileInfo fi(path);
    if (!fi.exists()) {
        if (errors) {
            *errors << tr("The %1 program is installed according to your settings, but the binary file can’t be found.<br>"
                          "Verify that all required programs are installed and in your preferences.",
                          "Error message. %1 - is an program name")
                               .arg(program);
        }
        return false;
    }

    if (!fi.isExecutable()) {
        if (errors) {
            *errors << tr("The %1 program is installed according to your settings, but the file is not executable.<br>"
                          "Verify that all required programs are installed and in your preferences.",
                          "Error message. %1 - is an program name")
                               .arg(program);
        }
        return false;
    }

    return true;
}

/************************************************

 ************************************************/
QString Settings_OLD::programName(const QString &program) const
{
#ifdef MAC_BUNDLE
    return QDir(qApp->applicationDirPath()).absoluteFilePath(program);
#else
    return value("Programs/" + program).toString();
#endif
}

/************************************************

 ************************************************/
QString Settings_OLD::programPath(const QString &program) const
{
#ifdef MAC_BUNDLE
    return programName(program);
#endif

    QString name = programName(program);

    if (name.isEmpty()) {
        return "";
    }

    if (QFileInfo(name).isAbsolute()) {
        return name;
    }

    return findProgram(name);
}

/************************************************

 ************************************************/
QString Settings_OLD::findProgram(const QString &program) const
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

 ************************************************/
void Settings_OLD::setValue(Settings_OLD::Key key, const QVariant &value)
{
    setValue(keyToString(key), value);
}

/************************************************

 ************************************************/
void Settings_OLD::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
}

/************************************************

 ************************************************/
void Settings_OLD::setDefaultValue(Key key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}

/************************************************

 ************************************************/
void Settings_OLD::setDefaultValue(const QString &key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}

/************************************************
 *
 ************************************************/

/************************************************
 *
 ************************************************/
