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
#include "flacontest.h"
#include "tools.h"
#include "converter/wavheader.h"
#include "../settings.h"

#include <filesystem>
#include <QString>
#include <QProcessEnvironment>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#ifdef Q_OS_WIN
#define PATH_ENV_SEPARATOR ';'
#define BINARY_EXT ".exe"

#elif defined(Q_OS_OS2)
#define PATH_ENV_SEPARATOR ';'
#define BINARY_EXT ".exe"

#else
#define PATH_ENV_SEPARATOR ':'
#define BINARY_EXT ""

#endif

/************************************************
 *
 ************************************************/
static void createWavFile(const QString &fileName, const int duration)
{
    QFile hdr(TEST_DATA_DIR "CD.wav.hdr");
    if (!hdr.open(QFile::ReadOnly))
        QTest::qFail(QStringLiteral("Can't open header file '%1': %2").arg(hdr.fileName()).arg(hdr.errorString()).toLocal8Bit(), __FILE__, __LINE__);

    createWavFile(fileName, hdr.readAll(), duration);
}

/************************************************
 *
 ************************************************/
void TestFlacon::initTestCase()
{
    initTypes();
    Settings::setFileName(TEST_OUT_DIR "/flacon.conf");

    findPrograms();

    const auto PROGS = {
        ExtProgram::mac(),
        ExtProgram::flac(),
        ExtProgram::wavpack(),
        ExtProgram::ttaenc(),
    };

    for (auto &p : PROGS) {
        if (p->path().isEmpty()) {
            QFAIL(QStringLiteral("%1 program not found").arg(p->name()).toLocal8Bit());
        }
    }

    if (!QDir().mkpath(mTmpDir))
        QTest::qFail(QStringLiteral("Can't create directory '%1'").arg(mTmpDir).toLocal8Bit(), __FILE__, __LINE__);

    createWavFile(mTmpDir + "1sec.wav", 1);
    createWavFile(mTmpDir + "1min.wav", 60);

    mAudio_cd_wav  = mTmpDir + "CD.wav";
    mAudio_cd_ape  = mTmpDir + "CD.ape";
    mAudio_cd_flac = mTmpDir + "CD.flac";
    mAudio_cd_wv   = mTmpDir + "CD.wv";
    mAudio_cd_tta  = mTmpDir + "CD.tta";

    createWavFile(mAudio_cd_wav, 900);

    auto wait_cd_ape  = QtConcurrent::run(encodeAudioFile, mAudio_cd_wav, mAudio_cd_ape);
    auto wait_cd_flac = QtConcurrent::run(encodeAudioFile, mAudio_cd_wav, mAudio_cd_flac);
    auto wait_cd_wv   = QtConcurrent::run(encodeAudioFile, mAudio_cd_wav, mAudio_cd_wv);
    auto wait_cd_tta  = QtConcurrent::run(encodeAudioFile, mAudio_cd_wav, mAudio_cd_tta);

    mAudio_24x96_wav  = mTmpDir + "24x96.wav";
    mAudio_24x96_ape  = mTmpDir + "24x96.ape";
    mAudio_24x96_flac = mTmpDir + "24x96.flac";
    mAudio_24x96_wv   = mTmpDir + "24x96.wv";
    mAudio_24x96_tta  = mTmpDir + "24x96.tta";

    {
        QFile hdr(TEST_DATA_DIR "24x96.wav.hdr");
        if (!hdr.open(QFile::ReadOnly))
            QTest::qFail(QStringLiteral("Can't open header file '%1': %2").arg(hdr.fileName()).arg(hdr.errorString()).toLocal8Bit(), __FILE__, __LINE__);

        createWavFile(mAudio_24x96_wav, hdr.readAll(), 900);
    }
    auto wait_24x96_ape  = QtConcurrent::run(encodeAudioFile, mAudio_24x96_wav, mAudio_24x96_ape);
    auto wait_24x96_flac = QtConcurrent::run(encodeAudioFile, mAudio_24x96_wav, mAudio_24x96_flac);
    auto wait_24x96_wv   = QtConcurrent::run(encodeAudioFile, mAudio_24x96_wav, mAudio_24x96_wv);
    auto wait_24x96_tta  = QtConcurrent::run(encodeAudioFile, mAudio_24x96_wav, mAudio_24x96_tta);

    wait_cd_ape.waitForFinished();
    wait_cd_flac.waitForFinished();
    wait_cd_wv.waitForFinished();
    wait_cd_tta.waitForFinished();

    wait_24x96_ape.waitForFinished();
    wait_24x96_flac.waitForFinished();
    wait_24x96_wv.waitForFinished();
    wait_24x96_tta.waitForFinished();
}

/************************************************
 *
 ************************************************/
void TestFlacon::findPrograms()
{
    for (auto p : ExtProgram::allPrograms()) {
        p->setPath(p->find());
    }
}

/************************************************
 *
 ************************************************/
static QString safePath(const QString &path)
{
    QString res = path;
    res         = res.replace(' ', '_');
    res         = res.replace('\t', '_');
    res         = res.replace('\n', '_');
    res         = res.replace('/', '_');
    return res;
}

/************************************************
 *
 ************************************************/
QString TestFlacon::dir(const QString &subTest)
{
    QString test    = QString::fromLocal8Bit(QTest::currentTestFunction());
    QString subtest = subTest.isEmpty() ? QString::fromLocal8Bit(QTest::currentDataTag()) : subTest;

    return QDir::cleanPath(QStringLiteral("%1/%2/%3")
                                   .arg(TEST_OUT_DIR)
                                   .arg(safePath(test))
                                   .arg(safePath(subtest)));
}

/************************************************
 *
 ************************************************/
QString TestFlacon::sourceDir(const QString &subTest)
{
    QString test    = QString::fromLocal8Bit(QTest::currentTestFunction());
    QString subtest = subTest.isEmpty() ? QString::fromLocal8Bit(QTest::currentDataTag()) : subTest;

    return QDir::cleanPath(QStringLiteral("%1/%2/%3")
                                   .arg(mDataDir)
                                   .arg(safePath(test))
                                   .arg(safePath(subtest)));
}

/************************************************
 *
 ************************************************/
void TestFlacon::copyTestDir(const QString &srcDir, const QString &destDir)
{
    try {
        namespace fs = std::filesystem;
        fs::copy(srcDir.toStdString(), destDir.toStdString(), fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks);
    }
    catch (std::exception &e) {
        QFAIL(e.what());
    }
}

void TestFlacon::copyFile(const QString &srcFile, const QString &destFile)
{
    try {
        namespace fs = std::filesystem;
        fs::remove(destFile.toStdString());
        fs::copy(srcFile.toStdString(), destFile.toStdString(), fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks);
    }
    catch (std::exception &e) {
        QFAIL(e.what());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::init()
{
    static QString prevTestFunction;
    if (prevTestFunction != QTest::currentTestFunction()) {
        prevTestFunction = QTest::currentTestFunction();
        mTestNum++;
    }

    QString dir = this->dir();

    QDir(dir).removeRecursively();
    if (!QDir().mkpath(dir)) {
        QTest::qFail(QStringLiteral("Can't create directory '%1'").arg(dir).toLocal8Bit(), __FILE__, __LINE__);
    }

    Settings::setFileName(dir + "/flacon.conf");

    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "default.debug=true\n");
}

void TestFlacon::cleanup()
{
    if (QTest::currentTestFailed()) {
        return;
    }

    if (getenv("FLACON_KEEP_TEST_DATA")) {
        return;
    }

    removeLargeFiles(dir());
}
