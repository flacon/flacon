/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2023
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

#include "convertertest.h"
#include <QTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonArray>

#include "tools.h"

static QString findFile(const QString &dir, const QString &pattern)
{
    QStringList files = QDir(dir).entryList(QStringList(pattern), QDir::Filter::Files);

    if (files.isEmpty()) {
        throw FlaconError(QString("File %1 not found").arg(pattern));
    }

    if (files.count() > 1) {
        throw FlaconError(QString("Mask %1 matches multiple files.").arg(pattern));
    }

    return files.first();
}

/************************************************

 ************************************************/
ConverterTest::ConverterTest(const QString &dataDir, const QString &dir, const QString &tmpDir) :
    mDataDir(dataDir),
    mDir(dir),
    mTmpDir(tmpDir),
    mCfgFile(mDir + "/flacon.conf"),
    mInDir(mDir + "/IN"),
    mOutDir(mDir + "/OUT")
{
    QDir::setCurrent(mDir);
    QFile::copy(dataDir + "/spec.ini", mDir + "/spec.ini");
    QSettings spec(mDir + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    QDir(mInDir).mkpath(".");
    QDir(mOutDir).mkpath(".");

    // Create config ............................
    {
        QString     src  = dataDir + "/flacon.conf";
        QStringList data = readFile(src);
        for (int i = 0; i < data.length(); ++i) {
            data[i] = data[i].replace("@TEST_DIR@", mDir);
        }
        writeFile(data, mCfgFile);
    }

    // Prepare source audio files ...............
    for (const QString &group : spec.childGroups()) {
        if (!group.toUpper().startsWith("SOURCE_AUDIO"))
            continue;

        spec.beginGroup(group);
        QString dest = mInDir + "/" + spec.value("destination").toString();
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

        // Generate file using script ...........
        if (method == "exec") {
            srcAudioExec(spec);
        }
        spec.endGroup();
    }
    // ..........................................

    // Copy source CUE files ....................
    spec.beginGroup("Source_CUE");
    foreach (auto key, spec.allKeys()) {
        QString src  = dataDir + "/" + key;
        QString dest = mInDir + "/" + spec.value(key).toString();

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
        QString dest = mInDir + "/" + spec.value(key).toString();

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
        QString dest = mDir + "/" + spec.value(key).toString();
        QString src  = dataDir + "/" + spec.value(key).toString();
        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................
}

/************************************************

 ************************************************/
void ConverterTest::srcAudioExec(QSettings &spec) const
{
    spec.beginGroup("commands");

    QStringList keys = spec.allKeys();
    for (const QString &key : qAsConst(keys)) {
        QString cmd = spec.value(key).toString();
        if (QProcess::execute(cmd) != 0) {
            throw FlaconError(QString("Command `%1` failed!").arg(cmd));
        }
    }

    spec.endGroup();
}

/************************************************

 ************************************************/
static bool removeDir(const QString &dirName)
{
    QDir dir(dirName);
    if (!dir.exists())
        return true;

    bool res = false;
    foreach (QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (fi.isDir())
            res = removeDir(fi.absoluteFilePath());
        else
            res = QFile::remove(fi.absoluteFilePath());

        if (!res)
            return false;
    }

    return dir.rmdir(dirName);
}

/************************************************

 ************************************************/
ConverterTest::~ConverterTest()
{
    if (QTest::currentTestFailed()) {
        return;
    }

    if (getenv("FLACON_KEEP_TEST_DATA")) {
        return;
    }

    QDir dir(mDir);
    if (!dir.exists()) {
        return;
    }

    foreach (QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (fi.isDir())
            removeDir(fi.absoluteFilePath());
        else
            QFile::remove(fi.absoluteFilePath());
    }
}

/************************************************

 ************************************************/
bool ConverterTest::run()
{
    QString     flacon = QCoreApplication::applicationDirPath() + "/../flacon";
    QStringList args;
    args << "--config" << mCfgFile;
    args << "--start";
    args << "--debug";
    args << mInDir.toLocal8Bit().data();

    createStartSh(mDir + "/start.sh", flacon, args);

    QProcess proc;
    proc.setStandardErrorFile(mDir + "/out.log");
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

 ************************************************/
void ConverterTest::check()
{
    QSettings spec(mDir + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    QString     msg   = "";
    QStringList files = findFiles(mOutDir, "*");
    QStringList missing;

    // ..........................................
    if (spec.childGroups().contains("Result_Audio")) {
        spec.beginGroup("Result_Audio");
        foreach (auto key, spec.allKeys()) {
            QString file = key;
            QString hash = spec.value(key).toString();

            if (files.removeAll(file) == 0) {
                missing << file;
                continue;
            }

            if (!hash.isEmpty()) {
                if (!compareAudioHash(mOutDir + "/" + file, hash))
                    QFAIL("");
            }
        }
        spec.endGroup();
    }
    // ..........................................

    // ..........................................
    if (spec.childGroups().contains("Result_CUE")) {
        spec.beginGroup("Result_CUE");
        foreach (auto key, spec.allKeys()) {
            QString file     = key;
            QString expected = dir() + "/" + spec.value(key).toString();

            if (files.removeAll(file) == 0) {
                missing << file;
                continue;
            }

            QString err;
            if (!compareCue(mOutDir + "/" + file, expected, &err))
                msg += "\n" + err;
        }
        spec.endGroup();
    }
    // ..........................................

    // ..........................................
    if (spec.childGroups().contains("Result_Files")) {
        spec.beginGroup("Result_Files");
        foreach (auto key, spec.allKeys()) {

            if (files.removeAll(key) == 0) {
                missing << key;
                continue;
            }
        }
        spec.endGroup();
    }
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
                       .arg(mOutDir)
                       .arg(missing.join("\n  * "));

    if (!files.isEmpty())
        msg += QString("\nFiles exists in %1:\n  * %2")
                       .arg(mOutDir)
                       .arg(files.join("\n  * "));

    if (!msg.isEmpty())
        QFAIL(QString("%1\n%2").arg(QTest::currentDataTag()).arg(msg).toLocal8Bit().data());

    // ******************************************
    // Check tags
    bool tagsError = false;
    spec.beginGroup("Result_Tags");
    foreach (auto file, spec.childGroups()) {

        Mediainfo mediainfo(mOutDir + "/" + file);
        mediainfo.save(mOutDir + "/" + file + ".json");

        spec.beginGroup(file);

        try {
            foreach (auto tag, spec.allKeys()) {

                QVariant actual   = mediainfo.value(tag);
                QVariant expected = spec.value(tag);

                if (actual != expected) {
                    printError(file, tag, actual, expected);
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

    checkReplayGain();
}

/************************************************

 ************************************************/
#define DEBUG_REPLAY_GAIN 0
bool ConverterTest::checkReplayGain()
{
    QSettings spec(mDir + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    bool errors = false;
    spec.beginGroup("Result_ReplayGain");
    foreach (auto group, spec.childGroups()) {
        spec.beginGroup(group);

        try {
            QString file = findFile(mOutDir, group);

            Mediainfo mediainfo(mOutDir + "/" + file);
            mediainfo.save(mOutDir + "/" + file + ".json");

            foreach (auto tag, spec.allKeys()) {

                QVariant actual   = mediainfo.value(tag);
                QVariant expected = spec.value(tag);
#if DEBUG_REPLAY_GAIN
                qDebug() << "*******************";
                qDebug() << "actual:" << actual;
                qDebug() << "expected:" << expected;
                qDebug() << "*******************";
#endif
                if (actual.type() != QVariant::Type::Double) {
                    QString s = actual.toString();
                    s         = s.remove("dB", Qt::CaseInsensitive);
                    s         = s.trimmed();

                    bool   ok;
                    double d = s.toDouble(&ok);

                    if (!ok) {
                        QWARN(QString("Can't convert actual value \"%1\" to double (tag: %2)").arg(actual.toString(), tag).toLocal8Bit());
                        errors = true;
                        continue;
                    }

                    actual = d;
                }

                if (actual != expected) {
                    printError(file, tag, actual, expected);
                    errors = true;
                }
            }
        }
        catch (const FlaconError &err) {
            QWARN(err.what());
            errors = true;
        }

        spec.endGroup();
    }
    spec.endGroup();

    if (errors) {
        FAIL("");
    }
    return !errors;
}

/************************************************

 ************************************************/
QStringList ConverterTest::readFile(const QString &fileName)
{
    QStringList res;
    QFile       file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isOpen()) {
        FAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());
        return res;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString str = stream.readLine();
        res << str << "\n";
    }

    file.close();
    return res;
}

/************************************************

 ************************************************/
void ConverterTest::writeFile(const QStringList &strings, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    if (!file.isOpen())
        QFAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());

    foreach (const QString &string, strings) {
        file.write(string.toLocal8Bit());
    }
}

/************************************************

 ************************************************/
void ConverterTest::createStartSh(const QString fileName, const QString flaconBin, const QStringList &args) const
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

 ************************************************/
QStringList ConverterTest::findFiles(const QString &dir, const QString &pattern) const
{
    QStringList  res;
    QDirIterator it(dir, QStringList() << pattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        res << (it.next()).remove(dir + "/");
    }
    return res;
}

/************************************************

 ************************************************/
void ConverterTest::printError(const QString &file, const QString &tag, const QVariant &actual, const QVariant &expected) const
{
    QWARN(QString("Compared values are not the same:\n"
                  "    File: %1\n"
                  "    Tag:  %2\n"
                  "\n"
                  "    Actual   : '%3' (%4)\n"
                  "    Expected : '%5' (%6)\n")

                  .arg(file)
                  .arg(tag)

                  .arg(QString::fromLocal8Bit(actual.toByteArray()))
                  .arg(actual.toByteArray().toHex(' ').data())

                  .arg(QString::fromLocal8Bit(expected.toByteArray()))
                  .arg(expected.toByteArray().toHex(' ').data())

                  .toLocal8Bit());
}

/************************************************

 ************************************************/
Mediainfo::Mediainfo(const QString &fileName) :
    mFileName(fileName)
{
    if (!QFileInfo::exists(fileName)) {
        throw FlaconError(QString("Can't read mediainfo to file '%1' (file don't exists').").arg(mFileName));
    }

    QStringList args;
    args << "--Full";
    args << "--Output=JSON";
    args << mFileName;

    QProcess proc;
    proc.setEnvironment(QStringList("LANG=en_US.UTF-8"));
    proc.start("mediainfo", args);
    proc.waitForFinished();
    if (proc.exitCode() != 0) {
        QString err = QString::fromLocal8Bit(proc.readAll());
        FAIL(QString("Can't read tags from \"%1\": %2").arg(mFileName).arg(err).toLocal8Bit());
    }

    QByteArray data = proc.readAllStandardOutput();
    mJsonDoc        = QJsonDocument::fromJson(data);
}

/************************************************

 ************************************************/
void Mediainfo::save(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(mJsonDoc.toJson(QJsonDocument::Indented));
    file.close();
}

/************************************************

 ************************************************/
static QVariant search(const QJsonObject &root, const QStringList &path)
{
    QJsonObject obj = root;
    for (const QString &key : path.mid(0, path.length() - 1)) {
        obj = obj.take(key).toObject();
    }

    return obj.take(path.last()).toVariant();
}

/************************************************

 ************************************************/
QVariant Mediainfo::value(const QString &key)
{
    QJsonArray arr = mJsonDoc["media"]["track"].toArray();

    QStringList path = key.split("/");

    if (path.isEmpty()) {
        return {};
    }

    for (QJsonValue v : arr) {
        QJsonObject obj = v.toObject();
        QVariant    res = search(obj, path);

        if (!res.isNull()) {
            return res;
        }
    }
    return {};
}
