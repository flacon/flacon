#include "validator.h"
#include "../profiles.h"
#include "../settings.h"
#include "sox.h"
#include "disc.h"
#include <QDebug>

/************************************************
 *
 ************************************************/
void Validator::setDisks(DiskList disks)
{
    mDisks = disks;
}

/************************************************
 *
 ************************************************/
bool Validator::canConvert() const
{
    if (mDisks.isEmpty()) {
        return false;
    }

    if (!Settings::i()->currentProfile().isValid()) {
        return false;
    }

    {
        // At least one disk does not contain errors
        bool hasValidDisk = false;
        for (const Disk *d : mDisks) {
            hasValidDisk = hasValidDisk || (diskHasErrors(d) == false);
        }

        if (!hasValidDisk) {
            return false;
        }
    }

    return true;
}

/************************************************
 *
 ************************************************/
bool Validator::hasWarnings() const
{
    for (const Disk *d : mDisks) {

        if (diskHasWarnings(d)) {
            return true;
        }
    }

    return false;
}

/************************************************
 *
 ************************************************/
bool Validator::diskHasWarnings(const Disk *disk) const
{
    return !warningsForDisk(disk).isEmpty();
}

/************************************************
 *
 ************************************************/
QStringList Validator::warningsForDisk(const Disc *disk) const
{

    QStringList res;
    for (const InputAudioFile &audioFile : disk->audioFiles()) {
        int bps = audioFile.bitsPerSample();
        if (Settings::i()->currentProfile().bitsPerSample() != BitsPerSample::AsSourcee) {
            bps = qMin(audioFile.bitsPerSample(), int(Settings::i()->currentProfile().bitsPerSample()));
        }

        if (bps > int(Settings::i()->currentProfile().maxBitPerSample())) {
            res << QObject::tr("A maximum of %1-bit per sample is supported by this format. This value will be used for encoding.", "Warning message")
                            .arg(int(Settings::i()->currentProfile().maxBitPerSample()));
        }

        int sr = audioFile.sampleRate();
        if (Settings::i()->currentProfile().sampleRate() != SampleRate::AsSource) {
            sr = qMin(sr, int(Settings::i()->currentProfile().sampleRate()));
        }

        if (sr > int(Settings::i()->currentProfile().maxSampleRate())) {
            res << QObject::tr("A maximum sample rate of %1 is supported by this format. This value will be used for encoding.", "Warning message")
                            .arg(int(Settings::i()->currentProfile().maxSampleRate()));
        }

        if (Settings::i()->currentProfile().gainType() != GainType::Disable && audioFile.channelsCount() > 2) {
            res << QObject::tr("ReplayGain calculation is not supported for multi-channel audio. The ReplayGain will be disabled for this disk.", "Warning message");
        }
    }
    return res;
}

/************************************************
 *
 ************************************************/
bool Validator::hasErrors() const
{
    for (const Disk *d : mDisks) {
        if (diskHasErrors(d)) {
            return true;
        }
    }

    return false;
}

/************************************************
 *
 ************************************************/
bool Validator::diskHasErrors(const Disk *disk) const
{
    return !errorsForDisk(disk).isEmpty();
}

/************************************************
 *
 ************************************************/
QStringList Validator::errorsForDisk(const Disk *disk) const
{
    QStringList errors;

    if (!validateCue(disk, errors)) {
        return errors;
    }

    validateAudioFiles(disk, errors);
    validateResultFiles(disk, errors);

    return errors;
}

/************************************************
 *
 ************************************************/
bool Validator::validateCue(const Disc *disk, QStringList &errors) const
{
    if (disk->count() > 0) {
        return true;
    }

    errors << QObject::tr("Cue file not set.");
    return false;
}

/************************************************
 *
 ************************************************/
bool Validator::validateAudioFiles(const Disc *disk, QStringList &errors) const
{
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
bool Validator::validateResultFiles(const Disc *disk, QStringList &errors) const
{
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
        if (disk == d) {
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
QStringList Validator::converterErros(const Conv::Converter::Jobs &jobs, const Profile &profile) const
{
    QStringList          errors;
    QList<const Track *> tracks;

    for (const Conv::Converter::Job &job : jobs) {
        for (const Track *track : job.tracks) {
            tracks << track;
        }
    }

    if (!profile.isValid()) {
        errors << QObject::tr("Incorrect output profile", "error message");
        return errors;
    }

    if (!profile.outFormat()->check(profile, &errors)) {
        return errors;
    }

    if (!validateRasampler(tracks, profile, errors)) {
        return errors;
    }

    {
        QStringList errs;
        for (const Conv::Converter::Job &j : jobs) {
            validateResultFiles(j.disc, errs);
        }

        if (!errs.isEmpty()) {
            errors << QObject::tr("Some disks will overwrite the resulting files of another disk.", "error message");
        }
    }
    return errors;
}

/************************************************
 *
 ************************************************/
bool Validator::validateRasampler(const QList<const Track *> &tracks, const Profile &profile, QStringList &errors) const
{
    bool needSox = false;
    needSox      = needSox || (profile.bitsPerSample() != BitsPerSample::AsSourcee || profile.sampleRate() != SampleRate::AsSource);

    for (const Track *track : tracks) {
        needSox = needSox || track->preEmphased();
    }

    if (!needSox) {
        return true;
    }

    return Settings::i()->checkProgram(Conv::Sox::programName(), &errors);
}
