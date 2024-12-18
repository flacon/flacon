/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
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

#include <QTest>
#include "flacontest.h"
#include "tools.h"

#include <QDebug>
#include <QProcess>
#include <QBuffer>
#include <QFileInfo>
#include <QDir>
#include <QThreadPool>
#include <QFileInfo>

#include "../disc.h"
#include "../settings.h"
#include "../project.h"
#include "../inputaudiofile.h"
#include "converter/converter.h"
#include "converter/wavheader.h"
#include "converter/splitter.h"
#include "../formats_out/outformat.h"
#include "converter/discpipline.h"

int TestFlacon::mTestNum = -1;
Q_DECLARE_METATYPE(GainType)

/************************************************

 ************************************************/
TestFlacon::TestFlacon(QObject *parent) :
    QObject(parent),
    mTmpDir(TEST_OUT_DIR),
    mDataDir(TEST_DATA_DIR),
    mStandardDisc(nullptr)
{
}

/************************************************

 ************************************************/
void TestFlacon::writeTextFile(const QString &fileName, const QString &content)
{
    QFile file(fileName);
    file.open(QFile::WriteOnly | QFile::Truncate);
    file.write(content.toLocal8Bit());
    file.close();
}

/************************************************

 ************************************************/
void TestFlacon::writeTextFile(const QString &fileName, const QStringList &content)
{
    QFile file(fileName);
    file.open(QFile::WriteOnly | QFile::Truncate);
    for (int i = 0; i < content.count(); ++i) {
        file.write(content.at(i).toLocal8Bit());
        file.write("\n");
    }
    file.close();
}

/************************************************

 ************************************************/
bool TestFlacon::removeDir(const QString &dirName) const
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
bool TestFlacon::clearDir(const QString &dirName) const
{
    if (getenv("FLACON_KEEP_TEST_DATA")) {
        return true;
    }

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

    return true;
}

void TestFlacon::removeLargeFiles(const QString &dirPath, qint64 sizeLimit)
{
    QDir dir(dirPath);

    if (!dir.exists()) {
        return;
    }

    // Получаем список файлов и папок в текущей директории
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            removeLargeFiles(entry.absoluteFilePath(), sizeLimit);
        }
        else if (entry.isFile() && entry.size() > sizeLimit) {
            QFile::remove(entry.absoluteFilePath());
        }
    }
}

/************************************************

 ************************************************/
void TestFlacon::checkFileExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists())
        QFAIL(QStringLiteral("File not exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
}

/************************************************

 ************************************************/
void TestFlacon::checkFileNotExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (fi.exists())
        QFAIL(QStringLiteral("File exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
}

/************************************************

 ************************************************/
void TestFlacon::applySettings(const SettingsValues &config)
{
    SettingsValues::const_iterator i;
    i = config.begin();
    for (i = config.begin(); i != config.end(); ++i) {
        Settings::i()->setValue(i.key(), i.value());
    }
    Settings::i()->sync();
}

/************************************************

 ************************************************/
Disc *TestFlacon::standardDisc()
{
    if (mStandardDisc == nullptr) {
        QString cueFile = dir() + "testTrackResultFileName.cue";

        QStringList cue;
        cue << "REM GENRE \"Genre\"";
        cue << "REM DATE 2013";
        cue << "REM DISCID 123456789";
        cue << "REM COMMENT \"ExactAudioCopy v0.99pb4\"";
        cue << "PERFORMER \"Artist\"";
        cue << "TITLE \"Album\"";
        cue << "FILE \"en.wav\" WAVE";
        cue << "  TRACK 01 AUDIO";
        cue << "    TITLE \"Song01\"";
        cue << "    INDEX 01 00:00:00";
        cue << "  TRACK 02 AUDIO";
        cue << "    TITLE \"Song02\"";
        cue << "    INDEX 01 03:39:10";
        cue << "  TRACK 03 AUDIO";
        cue << "    TITLE \"Song03\"";
        cue << "    INDEX 01 07:25:42";
        cue << "  TRACK 04 AUDIO";
        cue << "    TITLE \"Song04\"";
        cue << "    INDEX 01 12:04:72";

        writeTextFile(cueFile, cue);
        mStandardDisc = loadFromCue(cueFile);
    }

    return mStandardDisc;
}

/************************************************

 ************************************************/
void removeEmptyLines(QByteArray &data)
{
    data = data.trimmed();
    while (true) {
        int n = data.length();
        data.replace("\n\n", "\n");
        if (n == data.length())
            break;
    }
}

/************************************************

 ************************************************/
void TestFlacon::testByteArraySplit()
{
    QFETCH(QByteArray, inArray);
    QFETCH(char, separator);
    QFETCH(QByteArray, expectedLeft);
    QFETCH(QByteArray, expectedRight);

    QByteArray left = leftPart(inArray, separator);
    QCOMPARE(left, expectedLeft);

    QByteArray right = rightPart(inArray, separator);
    QCOMPARE(right, expectedRight);
}

/************************************************

 ************************************************/
void TestFlacon::testByteArraySplit_data()
{
    QTest::addColumn<QByteArray>("inArray", nullptr);
    QTest::addColumn<char>("separator", nullptr);
    QTest::addColumn<QByteArray>("expectedLeft", nullptr);
    QTest::addColumn<QByteArray>("expectedRight", nullptr);

    QTest::newRow("'' split by ' '")
            << QByteArray()
            << ' '
            << QByteArray()
            << QByteArray();

    QTest::newRow("'Test' split by ' '")
            << QByteArray("Test")
            << ' '
            << QByteArray("Test")
            << QByteArray();

    QTest::newRow("'REM GENRE Pop' split by ' '")
            << QByteArray("REM GENRE Pop")
            << ' '
            << QByteArray("REM")
            << QByteArray("GENRE Pop");

    QTest::newRow("'TITLE Рок группа' split by ' '")
            << QByteArray("TITLE Рок группа")
            << ' '
            << QByteArray("TITLE")
            << QByteArray("Рок группа");

    QTest::newRow("'Название Рок группа' split by ' '")
            << QByteArray("Название Рок группа")
            << ' '
            << QByteArray("Название")
            << QByteArray("Рок группа");

    QTest::newRow("'Название=Рок группа' split by '='")
            << QByteArray("Название=Рок группа")
            << '='
            << QByteArray("Название")
            << QByteArray("Рок группа");
}

/************************************************

 ************************************************/
void TestFlacon::testSafeString()
{
    QFETCH(QString, string);
    QFETCH(QString, expected);
    QString result = safeString(string);
    QCOMPARE(result, expected);
}

/************************************************

 ************************************************/
void TestFlacon::testSafeString_data()
{
    QTest::addColumn<QString>("string", nullptr);
    QTest::addColumn<QString>("expected", nullptr);

    for (char c = 1; c <= 31; ++c) {
        if (c != '\t' && c != '\n')
            QTest::newRow(QStringLiteral("01 - \\%1").arg(int(c), 2, 16, QChar('0')).toLocal8Bit())
                    << QString(c)
                    << "";
    }

    QTest::newRow("01 - \\t") << "\t"
                              << " ";
    QTest::newRow("01 - \\n") << "\n"
                              << " ";
    QTest::newRow("01 - /") << "/"
                            << "-";
    QTest::newRow("01 - \\") << "\\"
                             << "-";
    QTest::newRow("01 - *") << "*"
                            << "_";
    QTest::newRow("01 - :") << ":"
                            << "_";
    QTest::newRow("01 - ?") << "?"
                            << "";
    QTest::newRow("01 - <") << "<"
                            << "[";
    QTest::newRow("01 - >") << ">"
                            << "]";
    QTest::newRow("01 - \"") << "\""
                             << "'";

    for (char c = 1; c <= 31; ++c) {
        if (c != '\t' && c != '\n')
            QTest::newRow(QStringLiteral("02 - A\\%1B").arg(int(c), 2, 16, QChar('0')).toLocal8Bit())
                    << "A" + QString(c) + "B"
                    << "AB";
    }

    QTest::newRow("02 - A\\tB") << "A\tB"
                                << "A B";
    QTest::newRow("02 - A\\nB") << "A\nB"
                                << "A B";
    QTest::newRow("02 - A/B") << "A/B"
                              << "A-B";
    QTest::newRow("02 - A\\B") << "A\\B"
                               << "A-B";
    QTest::newRow("02 - A*B") << "A*B"
                              << "A_B";
    QTest::newRow("02 - A:B") << "A:B"
                              << "A_B";
    QTest::newRow("02 - A?B") << "A?B"
                              << "AB";
    QTest::newRow("02 - A<B") << "A<B"
                              << "A[B";
    QTest::newRow("02 - A>B") << "A>B"
                              << "A]B";
    QTest::newRow("02 - A\"B") << "A\"B"
                               << "A'B";

    QTest::newRow("03.1 single dot")
            << "."
            << "_";

    QTest::newRow("03.2 double dot")
            << ".."
            << "__";

    QTest::newRow("03.3 ?single dot?")
            << "?.?"
            << "_";

    QTest::newRow("03.4 ?double dot?")
            << "?..?"
            << "__";

    QTest::newRow("04 UTF8")
            << "Русский текст"
            << "Русский текст";
}

/************************************************

 ************************************************/
QStringList TestFlacon::readFile(const QString &fileName)
{
    QStringList res;
    QFile       file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isOpen()) {
        FAIL(QStringLiteral("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());
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
void TestFlacon::writeFile(const QStringList &strings, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    if (!file.isOpen())
        QFAIL(QStringLiteral("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());

    foreach (const QString &string, strings) {
        file.write(string.toLocal8Bit());
    }
}

/************************************************

 ************************************************/
QString TestFlacon::stigListToString(const QStringList &strings, const QString divider)
{
    return "\n--------------------------\n" + strings.join(divider) + "\n--------------------------\n";
}

/************************************************

 ************************************************/
QStringList &operator<<(QStringList &list, int value)
{
    return list << QStringLiteral("%1").arg(value);
}

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath()
{
    QFETCH(QString, outDir);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);
    QFETCH(QString, cueFile);

    Profile profile = Profile("WAV");
    profile.setOutFileDir(outDir);
    profile.setOutFilePattern(pattern);

    if (!cueFile.isEmpty())
        QFile::copy(mDataDir + "simple.cue", cueFile);
    else
        cueFile = mDataDir + "simple.cue";

    Disc *disc = loadFromCue(cueFile);

    QString result = profile.resultFilePath(disc->tracks().first());
    if (QFileInfo(result).absoluteFilePath() != QFileInfo(expected).absoluteFilePath()) {
        QString msg = QStringLiteral("Compared values are not the same\n   Actual:   %1 [%2]\n   Expected: %3\n   CueFile: %4").arg(QFileInfo(result).absoluteFilePath(), result, expected, cueFile);
        QFAIL(msg.toLocal8Bit());
    }
    // QCOMPARE(result, expected);
    disc->deleteLater();
}

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath_data()
{
    // QTest::addColumn<QString>("cueFile");
    QTest::addColumn<QString>("outDir", nullptr);
    QTest::addColumn<QString>("pattern", nullptr);
    QTest::addColumn<QString>("expected", nullptr);
    QTest::addColumn<QString>("cueFile", nullptr);

    QTest::newRow("1. /home/user/music")
            << "/home/user/music"
            << "%a/%y - %A/%n - %t"
            << "/home/user/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("2. ~/music")
            << "~/music"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("3. /music")
            << "/music"
            << "%a/%y - %A/%n - %t"
            << "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("4. empty")
            << ""
            << "%a/%y - %A/%n - %t"
            << mDataDir + "/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("5. dot (.) with CdAudioFile")
            << "."
            << "%a/%y - %A/%n - %t"
            << dir("5. dot (.) with CdAudioFile") + "/Artist/2013 - Album/01 - Song01.wav"
            << dir("5. dot (.) with CdAudioFile") + "/simple.cue";

    QTest::newRow("6. empty with CdAudioFile")
            << ""
            << "%a/%y - %A/%n - %t"
            << dir("6. empty with CdAudioFile") + "/Artist/2013 - Album/01 - Song01.wav"
            << dir("6. empty with CdAudioFile") + "/simple.cue";

    QTest::newRow("7. ~")
            << "~"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/Artist/2013 - Album/01 - Song01.wav"
            << "";
}

/************************************************

 ************************************************/
void TestFlacon::testTrackSetCodepages()
{
    QFETCH(QString, cueFile);
    QFETCH(QString, sampleFile);
    QFETCH(QString, codepageBefore);
    QFETCH(QString, codepageAfter);

    QString testDataDir = TEST_DATA_DIR;

    QString testCueFile  = dir() + "/input.cue";
    QString expectedFile = dir() + "/expected.cue";
    QString resultFile   = dir() + "/result.cue";

    if (QFileInfo(testCueFile).exists())
        QFile(testCueFile).remove();

    if (!QFile::copy(testDataDir + cueFile, testCueFile))
        QFAIL(QStringLiteral("Can't copy file %1 to %2").arg(testDataDir + cueFile, testCueFile).toLocal8Bit().data());

    Disc *disc = loadFromCue(testCueFile);

    if (!codepageAfter.isEmpty())
        disc->setCodecName(codepageAfter);

    QStringList expected = this->readFile(TEST_DATA_DIR + sampleFile);

    QStringList result;
    // Result *************************
    // result << "GENRE:" << tracks.genre() << "\n";
    // resultSl << "ALBUM:" << tracks.album() << "\n";
    result << "DISCID:" << disc->discIdTag() << "\n";

    for (int i = 0; i < disc->tracks().count(); ++i) {
        Track *track = disc->tracks().at(i);
        result << "Track " << (i + 1) << "\n";
        result << "  "
               << "INDEX:" << i << "\n";
        result << "  "
               << "TRACKNUM:" << track->trackNumTag() << "\n";
        result << "  "
               << "ALBUM:" << track->disk()->albumTag() << "\n";
        result << "  "
               << "TITLE:" << track->titleTag() << "\n";
        result << "  "
               << "ARTIST:" << track->artistTag() << "\n";
        result << "  "
               << "GENRE:" << track->genreTag() << "\n";
        result << "  "
               << "YEAR:" << track->dateTag() << "\n";
    }

    // Result *************************

    writeFile(result, resultFile);
    writeFile(expected, expectedFile);

    if (result.join("") != expected.join("")) {
        QString msg = "The result is different from the expected. Use the following command for details:";
        QString cmd = QStringLiteral("diff %1 %2").arg(expectedFile, resultFile);
        QFAIL((msg + "\n    " + cmd).toLocal8Bit());
    }

    disc->deleteLater();
}

/************************************************

 ************************************************/
void TestFlacon::testTrackSetCodepages_data()
{
    QTest::addColumn<QString>("cueFile", nullptr);
    QTest::addColumn<QString>("sampleFile", nullptr);
    QTest::addColumn<QString>("codepageBefore", nullptr);
    QTest::addColumn<QString>("codepageAfter", nullptr);

    QTest::newRow("01 TrackSet_UTF-8")
            << "ru_utf8.cue"
            << "ru.result"
            << "Big5"
            << "UTF-8";

    QTest::newRow("02 TrackSet_UTF-8_BOM")
            << "ru_utf8_BOM.cue"
            << "ru.result"
            << "Big5"
            << "";

    QTest::newRow("03 TrackSet_CP1251")
            << "ru_cp1251.cue"
            << "ru.result"
            << "UTF-8"
            << "Windows-1251";
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCueTime()
{
    QString string = QTest::currentDataTag();
    QFETCH(QString, expected);

    bool cdQuality = string.toUpper().startsWith("CD");
    string         = string.mid(2).trimmed();

    QString id1Str = string.section("-", 0, 0).trimmed();
    CueTime idx1(id1Str);

    QString id2Str = string.section("-", 1, 1).trimmed();
    if (!id2Str.isEmpty()) {
        CueTime idx2(id2Str);
        idx1 = idx1 - idx2;
    }

    QString result = idx1.toString(cdQuality);
    QCOMPARE(result, expected);
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCueTime_data()
{
    QTest::addColumn<QString>("expected", nullptr);

    QTest::newRow("CD") << "00:00:00";
    QTest::newRow("HI") << "00:00.000";

    QTest::newRow("CD 00:00:00") << "00:00:00";
    QTest::newRow("HI 00:00:00") << "00:00.000";

    QTest::newRow("CD 00:00:000") << "00:00:00";
    QTest::newRow("HI 00:00:000") << "00:00.000";
    QTest::newRow("HI 00:00.000") << "00:00.000";

    QTest::newRow("CD 1:02:3") << "01:02:30";
    QTest::newRow("HI 1:02:3") << "01:02.300";
    QTest::newRow("HI 1:02.3") << "01:02.300";

    QTest::newRow("CD 1:02:03") << "01:02:03";
    QTest::newRow("HI 1:02:03") << "01:02.030";
    QTest::newRow("HI 1:02.03") << "01:02.030";

    QTest::newRow("HI 1:02:030") << "01:02.030";
    QTest::newRow("HI 1:02.030") << "01:02.030";

    QTest::newRow("CD 1:02:74") << "01:02:74";
    QTest::newRow("HI 1:02:999") << "01:02.999";
    QTest::newRow("HI 1:02.999") << "01:02.999";

    QTest::newRow("CD 40:50:74 - 10:20:30") << "30:30:44";
    QTest::newRow("CD 40:20:24 - 10:30:30") << "29:49:69";

    QTest::newRow("HI 40:50:740 - 10:20:300") << "30:30.440";
    QTest::newRow("HI 40:20:240 - 10:30:300") << "29:49.940";
}

QTEST_GUILESS_MAIN(TestFlacon)
