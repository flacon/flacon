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
#include "extprogram.h"

static constexpr auto PROFILES_GROUP = "Profiles";
static constexpr auto PROGRAMS_GROUP = "Programs";

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

/************************************************
 *
 ************************************************/
void Settings::readExtPrograms() const
{
    for (ExtProgram *p : ExtProgram::allPrograms()) {
        auto key = QString("%1/%2").arg(PROGRAMS_GROUP, p->name());

        QString path = value(key).toString();
        if (path.isEmpty()) {
            path = p->find();
        }
        p->setPath(path);
    }
}

/************************************************
 *
 ************************************************/
void Settings::writeExtPrograms()
{
    remove(PROGRAMS_GROUP);
    for (ExtProgram *p : ExtProgram::allPrograms()) {
        auto key = QString("%1/%2").arg(PROGRAMS_GROUP, p->name());
        setValue(key, p->path());
    }
}
