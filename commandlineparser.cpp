/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2025
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

#include "commandlineparser.h"

#include <QTextStream>
#include "appconfig.h"
#include <QDate>
#include <QDebug>

/**************************************
 *
 **************************************/
bool CommandLineParser::isConsoleMode(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);

        if (arg == "--start") {
            return true;
        }

        if (arg.startsWith("-") && !arg.startsWith("--")) {
            if (arg.contains("s")) {
                return true;
            }
        }
    }

    return false;
}

/**************************************
 *
 **************************************/
QString CommandLineParser::appVersion() const
{
    if (IS_TAG_RELEASE) {
        return APP_VERSION;
    }
    else {
        return QString("%1 + git %2 %3").arg(APP_VERSION, APP_GIT_COMMIT_HASH, APP_GIT_COMMIT_DATE);
    }
}

/**************************************
 *
 **************************************/
void CommandLineParser::process(const QCoreApplication &app)
{
    QCoreApplication::setApplicationVersion(appVersion());

    mParser.addPositionalArgument("file", "CUE or Audio file.");
    mParser.setApplicationDescription(" Flacon extracts individual tracks from one big audio file.");

    mParser.addHelpOption();
    mParser.addVersionOption();

    mParser.addOption(QCommandLineOption(
            QStringList()
                    << "s"
                    << "start",
            "Start to convert immediately."));

    mParser.addOption(QCommandLineOption(
            QStringList()
                    << "c"
                    << "config",
            "config file.",
            "FILE"));

    mParser.addOption(QCommandLineOption(
            QStringList()
                    << "q"
                    << "quiet",
            "Quiet mode (no output)."));

    mParser.addOption(QCommandLineOption(
            QStringList()
                    << "p"
                    << "progress",
            "Show progress during conversion."));

    mParser.addOption(QCommandLineOption(
            QStringList()
                    << "d"
                    << "debug",
            "Enable debug output."));

    mParser.process(app);
}

/**************************************
 *
 **************************************/
QStringList CommandLineParser::files()
{
    return mParser.positionalArguments();
}

/**************************************
 *
 **************************************/
bool CommandLineParser::debug() const
{
    return mParser.isSet("debug") || getenv("FLACON_DEBUG");
}

/**************************************
 *
 **************************************/
bool CommandLineParser::start() const
{
    return mParser.isSet("start");
}

/**************************************
 *
 **************************************/
bool CommandLineParser::quiet() const
{
    return mParser.isSet("quiet");
}

/**************************************
 *
 **************************************/
bool CommandLineParser::progress() const
{
    return mParser.isSet("progress");
}

/**************************************
 *
 **************************************/
QString CommandLineParser::config() const
{
    return mParser.value("config");
}
