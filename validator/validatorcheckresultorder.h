#ifndef VALIDATORCHECKRESULTORDER_H
#define VALIDATORCHECKRESULTORDER_H

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
