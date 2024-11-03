#ifndef VALIDATORDATA_H
#define VALIDATORDATA_H

#include <QList>
#include <disc.h>

class Track;
class Profile;

struct ValidatorTrack
{
    ValidatorTrack(const Track *track, const Profile *profile);

    bool operator==(const ValidatorTrack &other) const { return id == other.id; }

    uint64_t id     = 0;
    uint64_t diskId = 0;

    QString resultFilePath;
    int     resultFilePathIndex = -1;
};

struct ValidatorDisk
{
    ValidatorDisk(const Disc *disk, const Profile *profile);

    bool operator==(const ValidatorDisk &other) const { return id == other.id; }

    uint64_t id = 0;
    QString  artistTag;
    QString  albumTag;

    QList<ValidatorTrack> tracks;
};

struct ValidatorData
{

public:
    void clear();
    void fill(QList<::Disc *> disks, const Profile *profile);

    QList<ValidatorDisk> disks;

private:
    void fillResultFilePathIndex();
};

#endif // VALIDATORDATA_H
