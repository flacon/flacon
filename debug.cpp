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
