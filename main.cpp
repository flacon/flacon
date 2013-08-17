/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
 *
 * Copyright: 2012-2013
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


#include <QApplication>
#include "mainwindow.h"
#include "settings.h"
#include "converter/converter.h"
#include <QString>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>

#include <QMessageBox>
#include <QTextStream>

void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: flacon [options] [file]" << endl;
    out << endl;
    out << "Flacon extracts individual tracks from one big audio file" << endl;
    out << endl;

    out << "Generic options:" << endl;
    out << "  -s --start                Start to convert immediately." << endl;
    out << "  -c --config <file>        Specify an alternative configuration file." << endl;

    out << "  -h, --help                Show help about options" << endl;
    out << "  --version                 Show version information" << endl;


    out << endl;
    out << "Arguments:" << endl;
    out << "  file                      CUE or Audio file" << endl;
}



void printVersion()
{
    QTextStream out(stdout);
    out << "flacon " << FLACON_VERSION << endl;
    out << "Copyright (c) 2013 Alexander Sokolov" << endl;
    out << "   https://github.com/SokoloffA/flacon" << endl;
    out << endl;
    out << "License LGPLv2.1+: GNU GNU Lesser General Public License version 2.1" << endl;
    out << "or later <http://www.gnu.org/licenses/lgpl-2.1.html>." << endl;
    out << "This is free software: you are free to change and redistribute it." << endl;
    out << "There is NO WARRANTY, to the extent permitted by law." << endl;
}

void translate(QApplication *app)
{
    QString locale = QLocale::system().name();

    QTranslator *qtTranslator = new QTranslator(app);
    qtTranslator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app->installTranslator(qtTranslator);

    QTranslator *appTranslator = new QTranslator(app);
    appTranslator->load(QString("flacon_%2.qm").arg(locale)) ||
            appTranslator->load(QString("%1/flacon_%2.qm").arg(TRANSLATIONS_DIR, locale));
    app->installTranslator(appTranslator);
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    translate(&app);


    bool start = false;
    QStringList files;
    QStringList args = app.arguments();
    for (int i=1; i < args.count(); ++i)
    {
        QString arg = args.at(i);

        if (arg == "--help" || arg == "-h")
        {
            printHelp();
            return 0;
        }

        if (arg == "--version")
        {
            printVersion();
            return 0;
        }

        if (arg == "--start" || arg == "-s")
        {
            start = true;
            continue;
        }

        if (arg == "--config" || arg == "-c")
        {
            i++;
            if (i<args.count())
                Settings::setFileName(args.at(i));

            continue;
        }

        files << arg;

    }

    MainWindow window;

    foreach(QString file, files)
        window.addFileOrDir(file);

    if (start)
    {
        Converter converter;
        QEventLoop loop;
        loop.connect(&converter, SIGNAL(finished()), &loop, SLOT(quit()));

        converter.start();
        loop.exec();
        return 0;
    }

    window.show();
    return app.exec();
}
