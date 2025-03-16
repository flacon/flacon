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

#include "runconsole.h"

#include <QCoreApplication>
#include "project.h"
#include "settings.h"
#include "scanner.h"
#include "types.h"
#include "converter/converter.h"
#include "disc.h"
#include <QStringList>
#include <QDebug>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "User")
}

/**************************************
 *
 **************************************/
int RunConsole::run(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    mCommandLineParser.process(app);

    if (mCommandLineParser.debug()) {
        setenv("QT_LOGGING_RULES",
               "*.debug=true;"
               "qt.*.debug=false;"
               "kf.*.debug=false;",
               1);
        qSetMessagePattern("%{time yyyy.MM.dd hh:mm:ss.zzz t} [%{threadid}] %{type}: %{category}: %{message}");
    }
    else {
        setenv("QT_LOGGING_RULES",
               "*.debug=false;"
               "*.info=false;"
               "default.debug=true;"
               "default.info=true;",
               1);
        qSetMessagePattern("%{message}");
    }

    QTimer::singleShot(0, this, &RunConsole::doRun);
    return app.exec();
}

/**************************************
 *
 **************************************/
void RunConsole::doRun()
{
    if (!mCommandLineParser.config().isEmpty()) {
        Settings::setFileName(mCommandLineParser.config());
    }

    Project *project = Project::instance();
    project->load(Settings::i());

    for (const QString &file : mCommandLineParser.files()) {
        QFileInfo fi = QFileInfo(file);

        if (fi.isDir()) {
            Scanner scanner;
            scanner.connect(&scanner, &Scanner::found, this, qOverload<const QString &>(&RunConsole::addFile));
            scanner.start(fi.absoluteFilePath());
        }
        else {
            addFile(file, true);
        }
    }

    if (project->count() == 0) {
        qCCritical(LOG) << "Error: No input files found";
        qApp->exit(int(ExitCodes::NoInputFiles));
        return;
    }

    if (!validate()) {
        qApp->exit(int(ExitCodes::ValidationError));
        return;
    }

    Conv::Converter *converter = new Conv::Converter(this);
    connect(converter, &Conv::Converter::finished, this, &RunConsole::finished);

    if (mCommandLineParser.progress()) {
        connect(converter, &Conv::Converter::trackProgress, this, &RunConsole::printProgress);
    }
    mStartTime = QDateTime::currentDateTime();
    converter->start(*(project->profile()));
}

/**************************************
 *
 **************************************/
void RunConsole::finished(bool success)
{
    mFinishTime = QDateTime::currentDateTime();

    if (success) {
        QTextStream(stdout) << "conversion successfully finished";
        printStatistic();
        qApp->exit(int(ExitCodes::Ok));
    }
    else {
        qCWarning(LOG) << "conversion failed";
        qApp->exit(int(ExitCodes::ConverterError));
    }
}

/**************************************
 *
 **************************************/
void RunConsole::printStatistic()
{
    if (mCommandLineParser.quiet()) {
        return;
    }

    int duration = mFinishTime.toSecsSinceEpoch() - mStartTime.toSecsSinceEpoch();
    if (!duration)
        duration = 1;

    int h = duration / 3600;
    int m = (duration - (h * 3600)) / 60;
    int s = duration - (h * 3600) - (m * 60);

    QString str;

    if (h)
        str = QStringLiteral("Encoding time %4h %3m %2s [%1 sec]").arg(duration).arg(s).arg(m).arg(h);
    else if (m)
        str = QStringLiteral("Encoding time %3m %2s [%1 sec]").arg(duration).arg(s).arg(m);
    else
        str = QStringLiteral("Encoding time %1 sec").arg(duration);

    QTextStream(stdout) << str << "\n";
}

/**************************************
 *
 **************************************/
void RunConsole::addFile(const QString &file)
{
    addFile(file, false);
}

/**************************************
 *
 **************************************/
void RunConsole::addFile(const QString &file, bool showError)
{
    try {
        QFileInfo fi = QFileInfo(file);
        DiscList  discs;
        if (fi.size() > 102400) {
            discs << Project::instance()->addAudioFile(file);
        }
        else {
            Disc *disk = Project::instance()->addCueFile(file);
            discs << disk;
        }
    }
    catch (FlaconError &err) {
        if (showError) {
            qCWarning(LOG) << "Warning: " << err.what();
        }
    }
}

/**************************************
 *
 **************************************/
void RunConsole::printProgress(const Track &track, TrackState state, Percent)
{
    QString status;
    switch (state) {
        case TrackState::Canceled:
            status = "Canceled";
            break;
        case TrackState::Error:
            status = "Error";
            break;
        case TrackState::Aborted:
            status = "Aborted";
            break;
        case TrackState::OK:
            status = "Done";
            break;
        default:
            return;
    }

    QTextStream(stdout)
            << status << " "
            << Project::instance()->profile()->resultFilePath(&track) << "\n";
}

/**************************************
 *
 **************************************/
QString RunConsole::diskDescription(const Disc *disk) const
{
    QString artist = disk->isEmpty() ? "" : disk->tracks().first()->performerTag();
    return QString("Disk \"%1 - %2\"").arg(artist, disk->albumTag());
}

/**************************************
 *
 **************************************/
bool RunConsole::validate() const
{
    Project   *project   = Project::instance();
    Validator &validator = project->validator();

    bool res = true;

    validator.revalidateNow();

    for (const Disk *disk : project->disks()) {
        if (validator.diskHasErrors(disk)) {
            qCCritical(LOG).noquote() << diskDescription(disk) << "has errors:";

            for (QString error : project->validator().diskErrors(disk)) {
                qCCritical(LOG).noquote() << " • " << error;
                res = false;
            }
        }
    }

    for (const Disk *disk : project->disks()) {
        if (validator.diskHasWarnings(disk)) {
            qCWarning(LOG).noquote() << diskDescription(disk) << "has warnings:";

            for (QString warn : project->validator().diskWarnings(disk)) {
                qCWarning(LOG).noquote() << " • " << warn;
            }
        }
    }

    return res;
}
