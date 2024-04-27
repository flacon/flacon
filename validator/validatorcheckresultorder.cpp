#include "validatorcheckresultorder.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "track.h"
#include "disc.h"
#include "profiles.h"
#include <QApplication>

ValidatorCheckResultOrder::ValidatorCheckResultOrder(const QList<const Disk *> disks, const Profile *profile) :
    mDisks(disks),
    mProfile(profile)
{
}

void ValidatorCheckResultOrder::clear()
{
    mWarnings.clear();
    mErrors.clear();
}

bool ValidatorCheckResultOrder::validate(const Disk *disk)
{
    ValidatorResultFiles files(mDisks, mProfile);
    files.sortByPath();

    QMap<QString, ValidatorResultFiles> byDir = files.splitByDirectory();

    bool res = true;
    for (auto it = byDir.begin(); it != byDir.end(); ++it) {
        res = validateDir(it.key(), disk, it.value()) && res;
    }

    return res;
}

bool ValidatorCheckResultOrder::validateDir(const QString &, const Disk *disk, const ValidatorResultFiles &files)
{
    QStringList ourFiles;
    for (auto f : files) {
        if (f.track->disc() == disk) {
            ourFiles << f.file.fileName();
        }
    }

    int firstIndex = files.indexOf([disk](const ValidatorResultFile &f) { return f.track->disc() == disk; });
    if (firstIndex < 0) {
        return true;
    }

    int lastIndex = files.lastIndexOf([disk](const ValidatorResultFile &f) { return f.track->disc() == disk; });

    bool res = true;
    for (int i = firstIndex; i <= lastIndex; ++i) {
        const Track *track = files.at(i).track;
        if (track->disc() != disk) {

            if (ourFiles.contains(files.at(i).file.fileName())) {
                continue; // The situation when files overwrite each other is handled in another check
            }

            res = false;
            mErrors << tr("The output files of the disc are mixed with the files of disc %1.\n"
                          "You could change the \"Start num\" for one of them.",
                          "Error message, %1 is the disk description, artist and album for the disc, respectively")
                               .arg(diskString(track->disc()));
        }
    }

    return res;
}

QString ValidatorCheckResultOrder::diskString(const Disk *disk)
{
    return QString("%1 \"%2 - %3\"").arg(mDisks.indexOf(disk) + 1).arg(disk->discTag(TagId::Artist), disk->discTag(TagId::Album));
}
