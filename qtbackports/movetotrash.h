#ifndef MOVETOTRASH_H
#define MOVETOTRASH_H

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
bool moveFileToTrash(const QString &fileName, QString *pathInTrash = nullptr);
#endif

#endif // MOVETOTRASH_H
