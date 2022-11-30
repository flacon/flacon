#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "disc.h"
#include "profiles.h"
#include <QMap>
#include <QTimer>

class Validator : public QObject
{
    Q_OBJECT
public:
    Validator(QObject *parent = nullptr);

    Profile profile() const { return mProfile; }
    void    setProfile(const Profile &profile);

    const DiskList &disks() const { return mDisks; }
    void            setDisks(DiskList disks);

    int  insertDisk(Disk *disk, int index = -1);
    void removeDisk(const DiskList &disks);

    void revalidate();

    QStringList converterErrors() const { return mGlobalErrors; }

    QStringList diskWarnings(const Disk *disk) const;
    bool        diskHasWarnings(const Disk *disk) const;

    QStringList diskErrors(const Disk *disk) const;
    bool        diskHasErrors(const Disk *disk) const;

    bool hasWarnings() const;
    bool hasErrors() const;

signals:
    void changed();

private:
    QList<Disk *> mDisks;
    Profile       mProfile;

    QTimer mDelayTimer;

    QStringList                     mGlobalErrors;
    QMap<const Disk *, QStringList> mDisksErrors;
    QMap<const Disk *, QStringList> mDisksWarnings;

    bool mResultFilesOverwrite = false;

    void startDelay();
    bool validateProfile();

    void revalidateDisk(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateCue(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateAudioFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateResultFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateRasampler(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool vaslidateDiskWarnings(const Disk *disk, QStringList &warnings);
};

#endif // VALIDATORS_H
