#ifndef VALIDATORCHECKRESULTORDER_H
#define VALIDATORCHECKRESULTORDER_H

/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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
#include <QStringList>
#include <QList>
#include <QObject>
#include <QCoreApplication>

class ValidatorCheckResultOrder
{
    Q_DECLARE_TR_FUNCTIONS(Validator)

public:
    ValidatorCheckResultOrder(const QList<const Disc *> disks, const Profile *profile);

    void clear();

    bool validate(const Disk *disk);

    QStringList errors() const { return mErrors; }
    QStringList warnings() const { return mWarnings; }

private:
    const QList<const Disc *> mDisks;
    const Profile            *mProfile = nullptr;

    QStringList mErrors;
    QStringList mWarnings;

    bool validateDir(const QString &dir, const Disk *disk, const ValidatorResultFiles &files);

    QString diskString(const Disk *disk);
};

#endif // VALIDATORCHECKRESULTORDER_H
