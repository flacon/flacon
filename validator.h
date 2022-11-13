#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "disc.h"
#include "converter/converter.h"

class Validator
{
public:
    const DiskList &disks() const { return mDisks; }
    void            setDisks(DiskList disks);

    bool canConvert() const;

    bool        diskHasWarnings(const Disk *disk) const;
    QStringList warningsForDisk(const Disk *disk) const;

    bool        diskHasErrors(const Disk *disk) const;
    QStringList errorsForDisk(const Disk *disk) const;

    QStringList converterErros(const Conv::Converter::Jobs &jobs, const Profile &profile) const;

private:
    QList<Disk *> mDisks;

    bool validateCue(const Disc *disk, QStringList &errors) const;
    bool validateAudioFiles(const Disc *disk, QStringList &errors) const;
    bool validateResultFiles(const Disc *disk, QStringList &errors) const;
    bool validateRasampler(const QList<const Track *> &tracks, const Profile &profile, QStringList &errors) const;
};

#endif // VALIDATORS_H
