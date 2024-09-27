#include "debug.h"
#include <QtGlobal>

namespace {
QStringList      messages;
QtMessageHandler defaultMessageHandler = nullptr;
bool             debugOn               = false;
}

static void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (debugOn || type > QtMsgType::QtDebugMsg || strcmp(context.category, "default") == 0) {
        (defaultMessageHandler)(type, context, message);
    }

    messages << qFormatLogMessage(type, context, message);
}

void initDebug(bool debug)
{
    debugOn = debug;

    setenv("QT_LOGGING_RULES", "*.debug=true;qt.*.debug=false;kf.*.debug=false;", 1);
    qSetMessagePattern("%{time yyyy.MM.dd hh:mm:ss.zzz t} [%{threadid}] %{type}: %{category}: %{message}");

    defaultMessageHandler = qInstallMessageHandler(msgHandler);
}

QStringList debugMessages()
{
    return messages;
}
