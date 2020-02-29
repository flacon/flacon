/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#include <QTest>
#include "testflacon.h"
#include "types.h"
//#include "../settings.h"
#include "tools.h"

#include <QDir>
#include <QProcess>
#include <QDirIterator>
#include <QSettings>


/************************************************
 *
 ************************************************/
static QStringList findFiles(const QString &dir, const QString &pattern)
{
    QStringList res;
    QDirIterator it(dir, QStringList() << pattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        res << (it.next()).remove(dir + "/");
    }
    return res;
}


/************************************************
 *
 ************************************************/
void TestFlacon::testConvert()
{
    QFETCH(QString, dataDir);

    QFile::copy(dataDir + "/spec.ini", dir() + "/spec.ini");
    QSettings spec(dir() + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    const QString cfgFile(dir() + "/flacon.conf");
    const QString inDir(dir()  + "/IN");
    const QString outDir(dir() + "/OUT");
    QDir(inDir).mkpath(".");
    QDir(outDir).mkpath(".");


    // Create config ............................
    {
        QString src = dataDir + "/flacon.conf";
        if (!QFile::copy(src,  cfgFile))
            QFAIL(QString("Can't copy config file \"%1\"").arg(src).toLocal8Bit());

        QSettings cfg(cfgFile, QSettings::IniFormat);
        cfg.setIniCodec("UTF-8");
        cfg.setValue("OutFiles/Directory", outDir);
        cfg.sync();
    }
    // ..........................................


    // Prepare source audio files ...............
    for (QString group: spec.childGroups()) {
        if (!group.toUpper().startsWith("SOURCE_AUDIO"))
            continue;

        spec.beginGroup(group);
        QString dest = inDir   + "/" + spec.value("destination").toString();
        QFileInfo(dest).dir().mkpath(".");

        QString method = spec.value("method", "copy").toString().toLower();

        // Copy audio file ......................
        if ( method == "copy") {
            QString src  = mTmpDir + "/" + spec.value("source").toString();;

            if (!QFile::copy(src,  dest))
                QFAIL(QString("Can't copy audio file \"%1\"").arg(src).toLocal8Bit());
        }

        // Generate file ........................
        if (method == "generate") {
            QString header = dataDir + "/" + spec.value("header").toString();
            createWavFile(dest, readFile(header).join("\n"));
        }
        spec.endGroup();
    }
    // ..........................................

    // Copy source CUE files ....................
    spec.beginGroup("Source_CUE");
    foreach (auto key, spec.allKeys())
    {
        QString src  = dataDir + "/" + key;
        QString dest = inDir   + "/" + spec.value(key).toString();

        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src,  dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................

    // Copy expected CUE files ....................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys())
    {
        QString dest = dir()   + "/" + spec.value(key).toString();
        QString src  = dataDir + "/" + spec.value(key).toString();
        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src,  dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................

    // Run flacon ...............................
    QString flacon = QCoreApplication::applicationDirPath() + "/../flacon";
    QStringList args;
    args << "--config" << cfgFile;
    args << "--start";
    args << "--quiet";
    args << inDir.toLocal8Bit().data();

    {
        QFile file(dir() + "/start.sh");
        file.open(QIODevice::WriteOnly);
        file.write("\"" + flacon.toLocal8Bit() + "\"");
        for (QString a: args)
        {
            a.replace("\"", "\\\"");
            file.write(" \\\n    \"" + a.toLocal8Bit() + "\"");
        }

        file.setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther |
                            QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther |
                            QFileDevice::ExeUser);

        file.close();
    }

    QProcess proc;
    proc.start(flacon, args);
    if (!proc.waitForFinished(30 * 1000))
    {
        QFAIL(QString("The program timed out waiting for the result: %1.")
              .arg(QString::fromLocal8Bit(proc.readAllStandardError())).toLocal8Bit());
    }

    if (proc.exitCode() != 0)
    {
        QFAIL(QString("flacon returned non-zero exit status %1: %2")
              .arg(proc.exitCode())
              .arg(QString::fromLocal8Bit(proc.readAll())).toLocal8Bit());
    }
    // ..........................................

    // Check ____________________________________
    QString msg = "";
    QStringList files = findFiles(outDir, "*");
    QStringList missing;

    // ..........................................
    spec.beginGroup("Result_Audio");
    foreach (auto key, spec.allKeys())
    {
        QString file = key;
        QString hash = spec.value(key).toString();

        if (files.removeAll(file) == 0)
        {
            missing << file;
            continue;
        }

        if (!hash.isEmpty())
            if (!compareAudioHash(outDir + "/" + file, hash))
                QFAIL("");

    }
    spec.endGroup();
    // ..........................................

    // ..........................................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys())
    {
        QString file = key;
        QString expected = dir() + "/" + spec.value(key).toString();

        if (files.removeAll(file) == 0)
        {
            missing << file;
            continue;
        }

        QString err;
        if (!TestFlacon::compareCue(outDir + "/" + file, expected, &err))
            msg += "\n" + err;
    }
    spec.endGroup();
    // ..........................................


    if (!missing.isEmpty())
        msg += QString("\nFiles not exists in %1:\n  * %2")
                .arg(outDir)
                .arg(missing.join("\n  * "));

    if (!files.isEmpty())
        msg += QString("\nFiles exists in %1:\n  * %2")
                .arg(outDir)
                .arg(files.join("\n  * "));

    if (!msg.isEmpty())
        QFAIL(QString("%1\n%2").arg(QTest::currentDataTag()).arg(msg).toLocal8Bit().data());


    // ..........................................

    clearDir(dir());
}


/************************************************
 *
 ************************************************/
void TestFlacon::testConvert_data()
{
    if (QProcessEnvironment::systemEnvironment().contains("FLACON_SKIP_CONVERT_TEST"))
        QTest::qSkip("Skipping testConvert", __FILE__, __LINE__);


    QTest::addColumn<QString>("dataDir", nullptr);

    QString dataDir = mDataDir + "/testConvert/";

    static const QStringList mask("*");
    foreach (auto dir, QDir(dataDir).entryList(mask, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if (QDir(dataDir + "/" + dir).exists("spec.ini"))
            QTest::newRow(dir.toUtf8()) << dataDir + "/" + dir;
    }
}

