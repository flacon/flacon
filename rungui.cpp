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

#include "rungui.h"
#include "application.h"
#include "project.h"
#include "settings.h"
#include "mainwindow.h"
#include "commandlineparser.h"
#include "appconfig.h"

#include <QLibraryInfo>
#include <QTranslator>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "User")
}

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

namespace {
QStringList      messages;
QtMessageHandler defaultMessageHandler = nullptr;
bool             debugOn               = false;

/**************************************
 *
 **************************************/
static void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (debugOn || type > QtMsgType::QtDebugMsg || strcmp(context.category, "default") == 0) {
        (defaultMessageHandler)(type, context, message);
    }

    messages << qFormatLogMessage(type, context, message);
}
} // namespace

/**************************************
 *
 **************************************/
namespace LibraryInfo {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QString path(QLibraryInfo::LibraryLocation p)
{
    return QLibraryInfo::location(p);
}
#else
QString path(QLibraryInfo::LibraryPath p)
{
    return QLibraryInfo::path(p);
}
#endif
}

/**************************************
 *
 **************************************/
int RunGui::run(int argc, char *argv[])
{
    Application app(argc, argv);
    translate(&app);
    CommandLineParser commandLineParser;
    commandLineParser.process(app);

    debugOn               = commandLineParser.debug();
    defaultMessageHandler = qInstallMessageHandler(msgHandler);

    setenv("QT_LOGGING_RULES", "*.debug=true;qt.*.debug=false;kf.*.debug=false;", 1);
    qSetMessagePattern("%{time yyyy.MM.dd hh:mm:ss.zzz t} [%{threadid}] %{type}: %{category}: %{message}");

    qCDebug(LOG) << "Start flacon " << APP_VERSION;
    qCDebug(LOG) << "git info: " << APP_GIT_COMMIT_HASH << " " << APP_GIT_COMMIT_DATE;

    if (!commandLineParser.config().isEmpty()) {
        Settings::setFileName(commandLineParser.config());
    }

    Project::instance()->load(Settings::i());

    MainWindow window;

    foreach (QString file, commandLineParser.files()) {
        window.addFileOrDir(file);
    }

    QObject::connect(&app, &Application::openFile,
                     &window, &MainWindow::addFileOrDir);

    window.show();

#ifdef MAC_UPDATER
    QTimer::singleShot(0, []() {
        Updater &updater = Updater::sharedUpdater();
        if (updater.automaticallyChecksForUpdates()) {
            updater.checkForUpdatesInBackground();
        }
    });
#endif

    int res = app.exec();
    Project::instance()->save(Settings::i());
    return res;
}

/**************************************
 *
 **************************************/
void RunGui::translate(QApplication *app)
{
#ifdef MAC_BUNDLE
    QString appDir = LibraryInfo::path(QLibraryInfo::TranslationsPath);
#elif APPIMAGE_BUNDLE
    QString appDir = LibraryInfo::path(QLibraryInfo::DataPath) + "/share/flacon/translations";
#else
    QString appDir = TRANSLATIONS_DIR;
#endif

    QString locale = QLocale::system().name();

    QTranslator *qtTranslator = new QTranslator(app);
    if (qtTranslator->load("qt_" + locale, LibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app->installTranslator(qtTranslator);
    }

    QTranslator *appTranslator = new QTranslator(app);
    if (appTranslator->load(QStringLiteral("flacon_%2.qm").arg(locale)) || appTranslator->load(QStringLiteral("%1/flacon_%2.qm").arg(appDir, locale))) {
        app->installTranslator(appTranslator);
    }
}

/**************************************
 *
 **************************************/
QStringList RunGui::logMessages()
{
    return messages;
}
