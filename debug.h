#ifndef DEBUG_H
#define DEBUG_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(DEBUG1)

#define DEBUG_1 qDebug(DEBUG1)

//typedef auto DEBUG1 = Debug::deb

#endif // DEBUG_H
