/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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

void noDebugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            break;

        case QtInfoMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;

        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", localMsg.constData());
            break;

        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", localMsg.constData());
            break;

        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", localMsg.constData());
            break;
    }
}


void debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    switch (type) {
        case QtDebugMsg:    fprintf(stderr, "::::: Debug: %s:%u, %s\n", file, context.line, function); break;
        case QtInfoMsg:     fprintf(stderr, "::::: Info: %s:%u, %s\n", file, context.line, function); break;
        case QtWarningMsg:  fprintf(stderr, "::::: Warning: %s:%u, %s\n", file, context.line, function); break;
        case QtCriticalMsg: fprintf(stderr, "::::: Error: %s:%u, %s\n", file, context.line, function); break;
        case QtFatalMsg:    fprintf(stderr, "::::: Fatal: %s:%u, %s\n", file, context.line, function); break;
    }
    for (const QString &line: msg.split('\n')) {
        fprintf(stderr, "    %s\n", line.toLocal8Bit().constData());
    }
}



QString debugProgramArgs(const QString &prog, const QStringList &args)
{
    QStringList res;

    res << prog;
    foreach (QString arg, args)
    {
        if (arg.contains(' ') || arg.contains('\t'))
            res << ("'" + arg + "'");
        else
            res << arg;
    }
    return res.join(" ");
}
