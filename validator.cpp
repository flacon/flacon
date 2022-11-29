#include "validator.h"
#include "sox.h"
#include "settings.h"
#include <QDebug>
#include <QDateTime>

/************************************************
 *
 ************************************************/
Validator::Validator(QObject *parent) :
    QObject(parent)
{
}

/************************************************
 *
 ************************************************/
void Validator::setDisks(DiskList disks)
{
    mDisks = disks;
    revalidate();
}

/************************************************
 *
 ************************************************/
void Validator::setProfile(const Profile &profile)
{
    mProfile = profile;
    revalidate();
}

/************************************************
 *
 ************************************************/
void Validator::revalidate()
{
    qDebug() << Q_FUNC_INFO << QDateTime::currentDateTime();

    mResultFilesOverwrite = false;
    mGlobalErrors.clear();
    mDisksErrors.clear();
    mDisksWarnings.clear();

    validateProfile();

    for (Disk *disk : qAsConst(mDisks)) {
        QStringList errors = mGlobalErrors;
        QStringList warnings;

        revalidateDisk(disk, errors, warnings);

        mDisksErrors[disk]   = errors;
        mDisksWarnings[disk] = warnings;
    }

    if (mResultFilesOverwrite) {
        mGlobalErrors << QObject::tr("Some disks will overwrite the resulting files of another disk.", "error message");
    }
}

/************************************************
 *
 ************************************************/
QStringList Validator::diskWarnings(const Disk *disk) const
{
    return mDisksWarnings.value(disk, {});
}

/************************************************
 *
 ************************************************/
bool Validator::diskHasWarnings(const Disk *disk) const
{
    return !diskWarnings(disk).isEmpty();
}

/************************************************
 *
 ************************************************/
QStringList Validator::diskErrors(const Disk *disk) const
{
    return mDisksErrors.value(disk, {});
}

/************************************************
 *
 ************************************************/
bool Validator::diskHasErrors(const Disk *disk) const
{
    return !diskErrors(disk).isEmpty();
}

/************************************************
 *
 ************************************************/
bool Validator::validateProfile()
{
    if (!mProfile.isValid()) {
        mGlobalErrors << QObject::tr("Incorrect output profile", "error message");
        return false;
    }

    QStringList errs;
    if (!mProfile.outFormat()->check(mProfile, &errs)) {
        mGlobalErrors << errs;
        return false;
    }

    return true;
}

/************************************************
 *
 ************************************************/
void Validator::revalidateDisk(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    if (!validateCue(disk, errors, warnings)) {
        return;
    }
    validateAudioFiles(disk, errors, warnings);

    bool ok = validateResultFiles(disk, errors, warnings);

    mResultFilesOverwrite = mResultFilesOverwrite || !ok;

    validateRasampler(disk, errors, warnings);

    vaslidateDiskWarnings(disk, warnings);
}

/************************************************
 *
 ************************************************/
bool Validator::validateCue(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    Q_UNUSED(warnings)

    if (disk->count() == 0) {
        errors << QObject::tr("Cue file not set.");
        return false;
    }

    return true;
}

/************************************************
 *
 ************************************************/
bool Validator::validateAudioFiles(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    Q_UNUSED(warnings)

    bool res = true;

    const QList<TrackPtrList> &audioFileTracks = disk->tracksByFileTag();
    for (const TrackPtrList &tracks : audioFileTracks) {
        const InputAudioFile &audio = tracks.first()->audioFile();

        if (audio.isNull()) {
            if (audioFileTracks.count() == 1) {
                errors << QObject::tr("Audio file not set.", "Warning message");
                res = false;
                continue;
            }

            if (tracks.count() == 1) {
                errors << QObject::tr("Audio file not set for track %1.", "Warning message, Placeholders is a track number")
                                  .arg(tracks.first()->trackNum());
                res = false;
                continue;
            }

            errors << QObject::tr("Audio file not set for tracks %1 to %2.", "Warning message, Placeholders is a track numbers")
                              .arg(tracks.first()->trackNum())
                              .arg(tracks.last()->trackNum());
            res = false;
            continue;
        }

        if (!audio.isValid()) {
            if (audioFileTracks.count() == 1) {
                errors << audio.errorString();
                res = false;
                continue;
            }
        }
    }

    for (const TrackPtrList &tracks : audioFileTracks) {
        InputAudioFile audio = tracks.first()->audioFile();
        if (audio.isNull() || !audio.isValid()) {
            continue;
        }

        uint duration = 0;
        for (int i = 0; i < tracks.count() - 1; ++i) {
            duration += tracks[i]->duration();
        }

        if (audio.duration() <= duration) {
            errors << QObject::tr("Audio file shorter than expected from CUE sheet.");
            res = false;
            break;
        }
    }

    return res;
}

/************************************************
 *
 ************************************************/
bool Validator::validateResultFiles(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    Q_UNUSED(warnings)

    bool res = true;

    auto resultFiles = [](const Disk *disk) -> QSet<QString> {
        QSet<QString> files;
        for (int i = 0; i < disk->count(); ++i) {
            files.insert(disk->track(i)->resultFilePath());
        }
        return files;
    };

    QSet<QString> diskFiles = resultFiles(disk);

    int n = 0;
    for (const Disk *d : mDisks) {
        n++;
        if (d == disk) {
            continue;
        }

        if (diskFiles.intersects(resultFiles(d))) {
            errors << QString("Disk %1 \"%2 - %3\" will overwrite the files of this disk.")
                              .arg(n)
                              .arg(d->discTag(TagId::Artist))
                              .arg(d->discTag(TagId::Album));

            res = false;
        }
    }

    return res;
}

/************************************************
 *
 ************************************************/
bool Validator::validateRasampler(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    Q_UNUSED(warnings)

    bool needSox = false;
    needSox      = needSox || (mProfile.bitsPerSample() != BitsPerSample::AsSourcee || mProfile.sampleRate() != SampleRate::AsSource);

    for (const Track *track : disk->tracks()) {
        needSox = needSox || track->preEmphased();
    }

    if (!needSox) {
        return true;
    }

    return Settings::i()->checkProgram(Conv::Sox::programName(), &errors);
}

/************************************************
 *
 ************************************************/
bool Validator::vaslidateDiskWarnings(const Disk *disk, QStringList &warnings)
{
    bool res = true;

    for (const InputAudioFile &audioFile : disk->audioFiles()) {
        int bps = audioFile.bitsPerSample();
        if (mProfile.bitsPerSample() != BitsPerSample::AsSourcee) {
            bps = qMin(audioFile.bitsPerSample(), int(mProfile.bitsPerSample()));
        }

        if (bps > int(mProfile.maxBitPerSample())) {
            warnings << QObject::tr("A maximum of %1-bit per sample is supported by this format. This value will be used for encoding.", "Warning message")
                                .arg(int(mProfile.maxBitPerSample()));
            res = false;
        }

        int sr = audioFile.sampleRate();
        if (mProfile.sampleRate() != SampleRate::AsSource) {
            sr = qMin(sr, int(mProfile.sampleRate()));
        }

        if (sr > int(mProfile.maxSampleRate())) {
            warnings << QObject::tr("A maximum sample rate of %1 is supported by this format. This value will be used for encoding.", "Warning message")
                                .arg(int(mProfile.maxSampleRate()));
            res = false;
        }

        if (mProfile.gainType() != GainType::Disable && audioFile.channelsCount() > 2) {
            warnings << QObject::tr("ReplayGain calculation is not supported for multi-channel audio. The ReplayGain will be disabled for this disk.", "Warning message");
            res = false;
        }
    }

    return res;
}
