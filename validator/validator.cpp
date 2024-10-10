/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include "validator.h"
#include <QDebug>
#include <QDateTime>
#include "extprogram.h"
#include "validatorcheckresultorder.h"
#include <QDir>

static constexpr int VALIDATE_DELAY_MS = 50;

/************************************************
 *
 ************************************************/
Validator::Validator(QObject *parent) :
    QObject(parent)
{
    mDelayTimer.setInterval(VALIDATE_DELAY_MS);
    mDelayTimer.setSingleShot(true);
    connect(&mDelayTimer, &QTimer::timeout, this, &Validator::revalidate);
}

/************************************************
 *
 ************************************************/
void Validator::setDisks(DiskList disks)
{
    mDisks = disks;
    startDelay();
}

/************************************************
 *
 ************************************************/
void Validator::setProfile(const Profile *profile)
{

    mProfile = profile;
    startDelay();
}

/************************************************
 *
 ************************************************/
int Validator::insertDisk(Disk *disk, int index)
{
    if (index < 0) {
        index = mDisks.count();
    }

    mDisks.insert(index, disk);

    startDelay();
    return index;
}

/************************************************
 *
 ************************************************/
void Validator::removeDisk(const DiskList &disks)
{
    for (Disk *d : disks) {
        disconnect(d);
        mDisks.removeAll(d);
    }

    startDelay();
}

/************************************************
 *
 ************************************************/
void Validator::startDelay()
{
    mDelayTimer.start();
}

/************************************************
 *
 ************************************************/
void Validator::revalidate()
{
    auto oldGlobalErrors  = mGlobalErrors;
    auto oldDisksErrors   = mDisksErrors;
    auto oldDisksWarnings = mDisksWarnings;

    mResultFilesOverwrite = false;
    mGlobalErrors.clear();
    mDisksErrors.clear();
    mDisksWarnings.clear();

    validateProfile();

    for (Disk *disk : std::as_const(mDisks)) {
        QStringList errors = mGlobalErrors;
        QStringList warnings;

        revalidateDisk(disk, errors, warnings);

        mDisksErrors[disk]   = errors;
        mDisksWarnings[disk] = warnings;
    }

    if (mResultFilesOverwrite) {
        mGlobalErrors << tr("Some disks will overwrite the resulting files of another disk.", "error message");
    }

    bool ch = false;

    ch = ch || (oldGlobalErrors != mGlobalErrors);
    ch = ch || (oldDisksErrors != mDisksErrors);
    ch = ch || (oldDisksWarnings != mDisksWarnings);

    if (ch) {
        emit changed();
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
bool Validator::hasErrors() const
{
    for (const Disk *d : mDisks) {
        if (diskHasErrors(d)) {
            return true;
        }
    }

    return false;
}

bool Validator::isValid() const
{
    return mProfile && !mDisks.isEmpty();
}

/************************************************
 *
 ************************************************/
bool Validator::validateProfile()
{
    if (!mProfile->isValid()) {
        mGlobalErrors << tr("Incorrect output profile.", "error message");
        return false;
    }

    QStringList errs;

    ExtProgram::sox()->check(&errs);
    mProfile->outFormat()->check(*mProfile, &errs);

    mGlobalErrors << errs;

    return errs.isEmpty();
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

    bool ok = true;

    ok = validateResultFiles(disk, errors, warnings) && ok;
    ok = validateDuplicateSourceFiles(disk, errors, warnings) && ok;

    mResultFilesOverwrite = mResultFilesOverwrite || !ok;

    validateDiskWarnings(disk, warnings);
}

/************************************************
 *
 ************************************************/
bool Validator::validateCue(const Disk *disk, QStringList &errors, QStringList &warnings)
{
    Q_UNUSED(warnings)

    if (disk->tracks().count() == 0) {
        errors << tr("Cue file not set.");
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
                errors << tr("Audio file not set.", "Warning message");
                res = false;
                continue;
            }

            if (tracks.count() == 1) {
                errors << tr("Audio file not set for track %1.", "Warning message, Placeholders is a track number")
                                  .arg(tracks.first()->trackNumTag());
                res = false;
                continue;
            }

            errors << tr("Audio file not set for tracks %1 to %2.", "Warning message, Placeholders is a track numbers")
                              .arg(tracks.first()->trackNumTag())
                              .arg(tracks.last()->trackNumTag());
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

        int duration = 0;
        for (int i = 0; i < tracks.count() - 1; ++i) {
            duration += tracks[i]->duration();
        }

        if (audio.duration() <= duration) {
            errors << tr("Audio file shorter than expected from CUE sheet.");
            res = false;
            break;
        }
    }

    if (!checkSameAudioForFileTags(disk)) {
        errors << tr("The same audio file is used for different tracks.", "Error message");
        res = false;
    }

    return res;
}

/************************************************
 *
 ************************************************/
bool Validator::checkSameAudioForFileTags(const Disk *disk)
{
    QMap<QString, QString> audioToFileTags;
    for (const Track *track : disk->tracks()) {
        if (track->audioFile().isNull()) {
            continue;
        }

        QString fileTag = audioToFileTags[track->audioFileName()];
        if (fileTag.isEmpty()) {
            audioToFileTags[track->audioFileName()] = track->fileTag();
            continue;
        }

        if (track->fileTag() != fileTag) {
            return false;
        }
    }

    return true;
}

/************************************************
 *
 ************************************************/
bool Validator::validateResultFiles(const Disk *disk, QStringList &inErrors, QStringList &warnings)
{
    Q_UNUSED(warnings)
    QStringList errors;

    for (const Track *track : disk->tracks()) {
        int n = 0;
        for (const Disk *d : mDisks) {
            n++;

            for (const Track *t : d->tracks()) {

                if (t == track) {
                    continue;
                }

                if (mProfile->resultFilePath(t) == mProfile->resultFilePath(track)) {
                    if (d == disk) {
                        errors << tr("Disk %1 \"%2 - %3\" will overwrite its own files.",
                                     "Error message, %1, %2 and %3 is the number, artist and album for the disc, respectively")
                                          .arg(n)
                                          .arg(d->artistTag(), d->albumTag());
                    }
                    else {
                        errors << tr("Disk %1 \"%2 - %3\" will overwrite the files of this disk.",
                                     "Error message, %1, %2 and %3 is the number, artist and album for the disc, respectively")
                                          .arg(n)
                                          .arg(d->artistTag(), d->albumTag());
                    }
                    break;
                }
            }
        }
    }

    QList<const Disk *> disks;
    for (auto d : mDisks) {
        disks << d;
    }

    ValidatorCheckResultOrder resultOrder(disks, mProfile);
    if (!resultOrder.validate(disk)) {
        errors << resultOrder.errors();
        warnings << resultOrder.warnings();
    }

    std::sort(errors.begin(), errors.end());
    errors.removeDuplicates();
    inErrors << errors;
    return errors.isEmpty();
}

/************************************************
 *
 ************************************************/
bool Validator::validateDuplicateSourceFiles(const Disk *disk, QStringList &errors, QStringList &warnings) const
{
    Q_UNUSED(errors)

    QStringList audioFiles = disk->audioFilePaths();

    int n = 0;
    for (const Disk *d : mDisks) {
        n++;

        if (d == disk) {
            continue;
        }

        if (d->cueFilePath() == disk->cueFilePath()) {
            warnings << tr("Disk %1 \"%2 - %3\" uses the same CUE file.",
                           "Warning message, %1, %2 and %3 is the number, artist and album for the disc, respectively")
                                .arg(n)
                                .arg(d->artistTag(), d->albumTag());
        }

        for (const QString &path : d->audioFilePaths()) {
            if (audioFiles.contains(path)) {

                warnings << tr("Disk %1 \"%2 - %3\" uses the same audio file.",
                               "Warning message, %1, %2 and %3 is the number, artist and album for the disc, respectively. %4 is an audio file name")
                                    .arg(n)
                                    .arg(d->artistTag(), d->albumTag());
            }
        }
    }

    return true;
}

/************************************************
 *
 ************************************************/
bool Validator::validateDiskWarnings(const Disk *disk, QStringList &warnings)
{
    bool res = true;

    for (const InputAudioFile &audioFile : disk->audioFiles()) {
        int bps = audioFile.bitsPerSample();
        if (mProfile->bitsPerSample() != BitsPerSample::AsSourcee) {
            bps = qMin(audioFile.bitsPerSample(), int(mProfile->bitsPerSample()));
        }

        if (bps > int(mProfile->maxBitPerSample())) {
            warnings << tr("A maximum of %1-bit per sample is supported by this format.\nThis value will be used for encoding.", "Warning message")
                                .arg(int(mProfile->maxBitPerSample()));
            res = false;
        }

        int sr = audioFile.sampleRate();
        if (mProfile->sampleRate() != SampleRate::AsSource) {
            sr = qMin(sr, int(mProfile->sampleRate()));
        }

        if (sr > int(mProfile->maxSampleRate())) {
            warnings << tr("A maximum sample rate of %1 is supported by this format.\nThis value will be used for encoding.", "Warning message")
                                .arg(int(mProfile->maxSampleRate()));
            res = false;
        }

        if (mProfile->gainType() != GainType::Disable && audioFile.channelsCount() > 2) {
            warnings << tr("ReplayGain calculation is not supported for multi-channel audio.\nThe ReplayGain will be disabled for this disk.", "Warning message");
            res = false;
        }
    }

    return res;
}

ValidatorResultFiles::ValidatorResultFiles(const QList<Disc *> disks, const Profile *profile)
{
    for (const Disk *d : disks) {
        for (Track *t : d->tracks()) {
            *this << ValidatorResultFile { QFileInfo(profile->resultFilePath(t)), t };
        }
    }
}

ValidatorResultFiles::ValidatorResultFiles(const QList<const Disc *> disks, const Profile *profile)
{
    for (const Disk *d : disks) {
        for (Track *t : d->tracks()) {
            *this << ValidatorResultFile { QFileInfo(profile->resultFilePath(t)), t };
        }
    }
}

QMap<QString, ValidatorResultFiles> ValidatorResultFiles::splitByDirectory() const
{
    QMap<QString, ValidatorResultFiles> res;
    for (const ValidatorResultFile &f : *this) {
        res[f.file.dir().absolutePath()].append(f);
    }

    return res;
}

void ValidatorResultFiles::sortByPath()
{
    std::sort(begin(), end(), [](auto &f1, auto &f2) {
        return f1.file.absoluteFilePath() < f2.file.absoluteFilePath();
    });
}

int ValidatorResultFiles::indexOf(const UnaryPred &where) const
{
    for (int i = 0; i < size(); ++i) {
        if (where(at(i))) {
            return i;
        }
    }

    return -1;
}

int ValidatorResultFiles::lastIndexOf(const UnaryPred &where) const
{
    for (int i = size() - 1; i >= 0; --i) {
        if (where(at(i))) {
            return i;
        }
    }

    return -1;
}

ValidatorResultFiles::const_iterator ValidatorResultFiles::findFirst(const UnaryPred &where) const
{
    return std::find_if(cbegin(), cend(), where);
}

ValidatorResultFiles::const_iterator ValidatorResultFiles::findLast(const UnaryPred &where) const
{
    for (auto it = cend() - 1; it >= cbegin(); --it) {
        if (where(*it)) {
            return it;
        }
    }

    return cend();
}
