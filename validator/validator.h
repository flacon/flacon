#ifndef VALIDATOR_H
#define VALIDATOR_H

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

#include "disc.h"
#include "profiles.h"
#include <QMap>
#include <QTimer>
#include <QFileInfo>

class Validator : public QObject
{
    Q_OBJECT
public:
    Validator(QObject *parent = nullptr);

    const Profile *profile() const { return mProfile; }
    void           setProfile(const Profile *profile);

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

    bool isValid() const;

signals:
    void changed();

protected:
    void doRevalidate();

private:
    QList<Disk *>  mDisks;
    const Profile *mProfile = nullptr;

    QTimer mDelayTimer;

    QStringList                      mGlobalErrors;
    QHash<const Disk *, QStringList> mDisksErrors;
    QHash<const Disk *, QStringList> mDisksWarnings;

    bool mResultFilesOverwrite = false;

    bool validateProfile();

    void revalidateDisk(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateCue(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateAudioFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateResultFiles(const Disk *disk, QStringList &errors, QStringList &warnings);
    bool validateDuplicateSourceFiles(const Disk *disk, QStringList &errors, QStringList &warnings) const;
    bool validateDiskWarnings(const Disk *disk, QStringList &warnings);

    bool checkSameAudioForFileTags(const Disk *disk);
};

struct ValidatorResultFile
{
    QFileInfo    file;
    const Track *track;
};

class ValidatorResultFiles : public QList<ValidatorResultFile>
{
public:
    using QList::QList;
    ValidatorResultFiles(const QList<Disc *> &disks, const Profile *profile);
    ValidatorResultFiles(const QList<const Disc *> &disks, const Profile *profile);

    QMap<QString, ValidatorResultFiles> splitByDirectory() const;

    void sortByPath();

    using QList::indexOf;
    using QList::lastIndexOf;

    using UnaryPred = std::function<bool(const ValidatorResultFile &)>;

    int indexOf(const UnaryPred &where) const;
    int lastIndexOf(const UnaryPred &where) const;

    ValidatorResultFiles::const_iterator findFirst(const UnaryPred &where) const;
    ValidatorResultFiles::const_iterator findLast(const UnaryPred &where) const;
    // ValidatorResultFiles::const_iterator searchFirst(std::function<bool(const ValidatorResultFile &)> &where);
};

#endif // VALIDATOR_H
