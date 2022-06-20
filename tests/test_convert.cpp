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
    QStringList  res;
    QDirIterator it(dir, QStringList() << pattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        res << (it.next()).remove(dir + "/");
    }
    return res;
}

/************************************************
 *
 ************************************************/
static void createStartSh(const QString fileName, const QString flaconBin, const QStringList &args)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write("\"" + flaconBin.toLocal8Bit() + "\"");
    for (QString a : args) {
        a.replace("\"", "\\\"");
        file.write(" \\\n    \"" + a.toLocal8Bit() + "\"");
    }

    file.setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther | QFileDevice::ExeUser);

    file.close();
}

/************************************************
 *
 ************************************************/
static bool runConvert(const QString &dir, const QString &inDir, const QString &cfgFile)
{
    QString     flacon = QCoreApplication::applicationDirPath() + "/../flacon";
    QStringList args;
    args << "--config" << cfgFile;
    args << "--start";
    args << "--debug";
    args << inDir.toLocal8Bit().data();

    createStartSh(dir + "/start.sh", flacon, args);

    QProcess proc;
    proc.setStandardErrorFile(dir + "/out.log");
    proc.start(flacon, args);

    if (!proc.waitForFinished(30 * 1000)) {
        FAIL(QString("The program timed out waiting for the result: %1.")
                     .arg(QString::fromLocal8Bit(proc.readAllStandardError()))
                     .toLocal8Bit());
        return false;
    }

    if (proc.exitCode() != 0) {
        FAIL(QString("flacon returned non-zero exit status %1: %2")
                     .arg(proc.exitCode())
                     .arg(QString::fromLocal8Bit(proc.readAll()))
                     .toLocal8Bit());
        return false;
    }

    return true;
}

/************************************************
 *
 ************************************************/
static QByteArray readTag(const QString &file, const QString &tag)
{
    if (!QFile::exists(file)) {
        throw FlaconError(QString("File %1 not found").arg(file));
    }

    if (!QFile::exists(file)) {
        throw FlaconError(QString("File %1 not found").arg(file));
    }

    QStringList tags;
    tags << tag;
    tags << tag.toUpper();
    tags << tag.toLower();

    for (const QString &t : tags) {
        QStringList args;
        args << QString("--Inform=General;%%1%").arg(t);
        args << file;

        QProcess proc;
        proc.setEnvironment(QStringList("LANG=en_US.UTF-8"));
        proc.start("mediainfo", args);
        proc.waitForFinished();
        if (proc.exitCode() != 0) {
            QString err = QString::fromLocal8Bit(proc.readAll());
            FAIL(QString("Can't read \"%1\" tag from \"%2\": %3").arg(tag).arg(file).arg(err).toLocal8Bit());
        }

        QByteArray res = proc.readAllStandardOutput().trimmed();

        if (res.isEmpty()) {
            continue;
        }

        // Workaround, older versions of mediainfo return the
        // "Track/Position_Total" tag as "2 / 2".
        if (tag == "Track/Position_Total") {
            return res.split('/').at(0).trimmed();
        }

        return res;
    }

    return {};
}

/************************************************
 *
 ************************************************/
void TestFlacon::testConvert()
{
    QFETCH(QString, dataDir);

    QDir::setCurrent(dir());

    QFile::copy(dataDir + "/spec.ini", dir() + "/spec.ini");
    QSettings spec(dir() + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    const QString cfgFile(dir() + "/flacon.conf");
    const QString inDir(dir() + "/IN");
    const QString outDir(dir() + "/OUT");
    QDir(inDir).mkpath(".");
    QDir(outDir).mkpath(".");

    // Create config ............................
    {
        QString     src  = dataDir + "/flacon.conf";
        QStringList data = readFile(src);
        for (int i = 0; i < data.length(); ++i) {
            data[i] = data[i].replace("@TEST_DIR@", dir());
        }
        writeFile(data, cfgFile);
    }
    // ..........................................

    // Prepare source audio files ...............
    for (const QString &group : spec.childGroups()) {
        if (!group.toUpper().startsWith("SOURCE_AUDIO"))
            continue;

        spec.beginGroup(group);
        QString dest = inDir + "/" + spec.value("destination").toString();
        QFileInfo(dest).dir().mkpath(".");

        QString method = spec.value("method", "copy").toString().toLower();

        // Copy audio file ......................
        if (method == "copy") {
            QString src = mTmpDir + "/" + spec.value("source").toString();
            ;

            if (!QFile::copy(src, dest))
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
    foreach (auto key, spec.allKeys()) {
        QString src  = dataDir + "/" + key;
        QString dest = inDir + "/" + spec.value(key).toString();

        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................

    // Copy source files ....................
    spec.beginGroup("Source_Files");
    foreach (auto key, spec.allKeys()) {
        QString src  = dataDir + "/" + key;
        QString dest = inDir + "/" + spec.value(key).toString();

        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest)) {
            QFAIL(QString("Can't copy file \"%1\"").arg(src).toLocal8Bit());
        }
    }
    spec.endGroup();
    // ..........................................

    // Copy expected CUE files ....................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys()) {
        QString dest = dir() + "/" + spec.value(key).toString();
        QString src  = dataDir + "/" + spec.value(key).toString();
        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................

    // Run flacon ...............................
    if (!runConvert(dir(), inDir, cfgFile)) {
        return;
    }

    // Check ____________________________________
    QString     msg   = "";
    QStringList files = findFiles(outDir, "*");
    QStringList missing;

    // ..........................................
    spec.beginGroup("Result_Audio");
    foreach (auto key, spec.allKeys()) {
        QString file = key;
        QString hash = spec.value(key).toString();

        if (files.removeAll(file) == 0) {
            missing << file;
            continue;
        }

        if (!hash.isEmpty()) {
            if (!compareAudioHash(outDir + "/" + file, hash))
                QFAIL("");
        }
    }
    spec.endGroup();
    // ..........................................

    // ..........................................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys()) {
        QString file     = key;
        QString expected = dir() + "/" + spec.value(key).toString();

        if (files.removeAll(file) == 0) {
            missing << file;
            continue;
        }

        QString err;
        if (!TestFlacon::compareCue(outDir + "/" + file, expected, &err))
            msg += "\n" + err;
    }
    spec.endGroup();
    // ..........................................

    // ..........................................
    spec.beginGroup("Result_Files");
    foreach (auto key, spec.allKeys()) {

        if (files.removeAll(key) == 0) {
            missing << key;
            continue;
        }
    }
    spec.endGroup();
    // ..........................................

    //    // ******************************************
    //    // Check commands
    //    spec.beginGroup("Check_Commands");
    //    foreach (auto key, spec.allKeys()) {
    //        QString cmd = spec.value(key).toString();
    //        int     res = QProcess::execute(cmd);
    //        if (res != 0) {
    //            QFAIL(QString("Chack is failed for %1").arg(cmd).toLocal8Bit());
    //        }
    //    }
    //    spec.endGroup();

    //    // ******************************************

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

    // ******************************************
    // Check tags
    bool tagsError = false;
    spec.beginGroup("Result_Tags");
    foreach (auto file, spec.childGroups()) {

        spec.beginGroup(file);

        try {
            foreach (auto tag, spec.allKeys()) {

                QByteArray actual = readTag(outDir + "/" + file, tag);

                QByteArray expected = spec.value(tag).toByteArray();

                if (actual != expected) {
                    QWARN(QString("Compared tags are not the same:\n"
                                  "    File: %1\n"
                                  "    Tag:  %2\n"
                                  "\n"
                                  "    Actual   : '%3' (%4)\n"
                                  "    Expected : '%5' (%6)\n")

                                  .arg(file)
                                  .arg(tag)

                                  .arg(QString::fromLocal8Bit(actual))
                                  .arg(actual.toHex(' ').data())

                                  .arg(QString::fromLocal8Bit(expected))
                                  .arg(expected.toHex(' ').data())

                                  .toLocal8Bit());
                    tagsError = true;
                }
            }
        }
        catch (const FlaconError &err) {
            QFAIL(err.what());
        }

        spec.endGroup();
    }
    spec.endGroup();

    if (tagsError) {
        QFAIL("Some tags not the same");
    }

    // ******************************************

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

    QString                  curDir = QDir::currentPath();
    static const QStringList mask("*");
    foreach (auto dir, QDir(dataDir).entryList(mask, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (QDir(dataDir + "/" + dir).exists("spec.ini"))
            QTest::newRow(dir.toUtf8()) << dataDir + "/" + dir;
    }
    QDir::setCurrent(curDir);
}
