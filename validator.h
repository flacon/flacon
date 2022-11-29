#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "disc.h"
#include "profiles.h"
#include <QMap>

class Validator : public QObject
{
    Q_OBJECT
public:
    Validator(QObject *parent = nullptr);

    const DiskList &disks() const { return mDisks; }
    void            setDisks(DiskList disks);

    Profile profile() const { return mProfile; }
    void    setProfile(const Profile &profile);

    void revalidate();

    QStringList converterErrors() const { return mGlobalErrors; }

    QStringList diskWarnings(const Disk *disk) const;
    bool        diskHasWarnings(const Disk *disk) const;

    QStringList diskErrors(const Disk *disk) const;
    bool        diskHasErrors(const Disk *disk) const;

private:
    QList<Disk *> mDisks;
    Profile       mProfile;

    QStringList                     mGlobalErrors;
    QMap<const Disk *, QStringList> mDisksErrors;
    QMap<const Disk *, QStringList> mDisksWarnings;

    bool mResultFilesOverwrite = false;

    bool validateProfile();

    void revalidateDisk(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateCue(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateAudioFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateResultFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateRasampler(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool vaslidateDiskWarnings(const Disk *disk, QStringList &warnings);
};

#endif // VALIDATORS_H
