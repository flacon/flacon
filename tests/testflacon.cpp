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
#include "testflacon.h"
#include "tools.h"

#include <QDebug>
#include <QProcess>
#include <QBuffer>
#include <QFileInfo>
#include <QDir>
#include <QThreadPool>

#include "../disc.h"
#include "../settings.h"
#include "../project.h"
#include "../inputaudiofile.h"
#include "converter/converter.h"
#include "converter/wavheader.h"
#include "converter/splitter.h"
#include "../formats_out/outformat.h"
#include "converter/discpipline.h"
#include "../converter/gain.h"

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

/************************************************

 ************************************************/
void TestFlacon::checkFileExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists())
        QFAIL(QString("File not exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
}

/************************************************

 ************************************************/
void TestFlacon::checkFileNotExists(const QString &fileName)
{
    QFileInfo fi(fileName);
    if (fi.exists())
        QFAIL(QString("File exists:\n\t%1").arg(fi.absoluteFilePath()).toLocal8Bit());
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
 *
 ************************************************/
static QByteArray readCue(const QString &fileName, bool skipEmptyLines)
{

    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QByteArray res;
    res.reserve(file.size());
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.startsWith("REM COMMENT"))
            continue;

        if (skipEmptyLines && line.isEmpty())
            continue;

        res += line;
        res += "\n";
    }
    file.close();
    return res;
}

/************************************************

 ************************************************/
bool TestFlacon::compareCue(const QString &result, const QString &expected, QString *error, bool skipEmptyLines)
{
    QByteArray resData = readCue(result, skipEmptyLines);
    QByteArray expData = readCue(expected, skipEmptyLines);

    if (resData != expData) {
        QString s = "The result is different from the expected. Use the following command for details: \n diff %1 %2";
        *error    = s.arg(expected, result);
        return false;
    }

    return true;
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
            QTest::newRow(QString("01 - \\%1").arg(int(c), 2, 16, QChar('0')).toLocal8Bit())
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
            QTest::newRow(QString("02 - A\\%1B").arg(int(c), 2, 16, QChar('0')).toLocal8Bit())
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
void TestFlacon::writeFile(const QStringList &strings, const QString &fileName)
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
QString TestFlacon::stigListToString(const QStringList &strings, const QString divider)
{
    return "\n--------------------------\n" + strings.join(divider) + "\n--------------------------\n";
}

/************************************************

 ************************************************/
QStringList &operator<<(QStringList &list, int value)
{
    return list << QString("%1").arg(value);
}

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName()
{
    QFETCH(QString, cue);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    Settings::i()->selectProfile("WAV");
    Settings::i()->currentProfile().setOutFilePattern(pattern);

    project->clear();

    QString cueFile = dir() + "/input.cue";
    writeTextFile(cueFile, cue);

    Disc *disc = loadFromCue(cueFile);

    QString result = disc->track(0)->resultFileName();
    // QCOMPARE(result, expected);

    if (result != expected) {
        QString msg = QString("Compared values are not the same\n   Pattern   %1\n   Actual:   %2\n   Expected: %3").arg(pattern, result, expected);
        QFAIL(msg.toLocal8Bit());
    }
    disc->deleteLater();
}

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName_data()
{
    QTest::addColumn<QString>("cue", nullptr);
    QTest::addColumn<QString>("pattern", nullptr);
    QTest::addColumn<QString>("expected", nullptr);

    QTest::newRow("1.1")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"
            << "%a/%y - %A/%n - %t"
            << "Artist/2013 - Album/01 - Song01.wav";

    QTest::newRow("1.2")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("1.3")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "N/n/A/a/t/y/g"
            << "N/n/A/a/t/y/g.wav";

    QTest::newRow("1.4")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "/%%/%Q/%N/%n/%A/%a/%t/%y/%g/%%"
            << "/%/%Q/04/01/Album/Artist/Song01/2013/Genre/%.wav";

    QTest::newRow("1.5")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "%%Q/%%N/%%n/%%A/%%a/%%t/%%y/%%g"
            << "%Q/%N/%n/%A/%a/%t/%y/%g.wav";

    QTest::newRow("1.6")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "TITLE \"Album\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "%%%Q/%%%N/%%%n/%%%A/%%%a/%%%t/%%%y/%%%g/%%%"
            << "%%Q/%04/%01/%Album/%Artist/%Song01/%2013/%Genre/%%.wav";

    QTest::newRow("2.1")
            << "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "{}/{Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "{}/{Text}/01/01 Text//.wav";

    QTest::newRow("2.2")
            << "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "Test{Text/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Test{Text/01/01 Text//.wav";

    QTest::newRow("2.3")
            << "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM COMMENT \"ExactAudioCopy v0.99pb4\"\n"
               "PERFORMER \"Artist\"\n"
               "FILE \"en.wav\" WAVE\n"
               "  TRACK 01 AUDIO\n"
               "    TITLE \"Song01\"\n"
               "    INDEX 01 00:00:00\n"
               "  TRACK 02 AUDIO\n"
               "    TITLE \"Song02\"\n"
               "    INDEX 01 03:39:10\n"
               "  TRACK 03 AUDIO\n"
               "    TITLE \"Song03\"\n"
               "    INDEX 01 07:25:42\n"
               "  TRACK 04 AUDIO\n"
               "    TITLE \"Song04\"\n"
               "    INDEX 01 12:04:72\n"

            << "Text}/{%n}/{%n Text}/{%A}/{%A Text}"
            << "Text}/01/01 Text//.wav";

    QTest::newRow("3.1")
            << R"( REM GENRE Genre
                    REM DATE 2013
                    REM DISCID 123456789
                    REM COMMENT "ExactAudioCopy v0.99pb4"
                    PERFORMER "Artist"
                    TITLE "."
                    FILE "en.wav" WAVE
                    TRACK 01 AUDIO
                        TITLE "Song01"
                        INDEX 01 00:00:00
                    TRACK 02 AUDIO
                        TITLE "Song02"
                        INDEX 01 03:39:10
                    TRACK 03 AUDIO
                        TITLE "Song03"
                        INDEX 01 07:25:42
                    TRACK 04 AUDIO
                        TITLE "Song04"
                        INDEX 01 12:04:72)"
            << "%a/%A/%n - %t"
            << "Artist/_/01 - Song01.wav";

    QTest::newRow("3.2")
            << R"( REM GENRE Genre
                    REM DATE 2013
                    REM DISCID 123456789
                    REM COMMENT "ExactAudioCopy v0.99pb4"
                    PERFORMER "Artist"
                    TITLE ".."
                    FILE "en.wav" WAVE
                    TRACK 01 AUDIO
                        TITLE "Song01"
                        INDEX 01 00:00:00
                    TRACK 02 AUDIO
                        TITLE "Song02"
                        INDEX 01 03:39:10
                    TRACK 03 AUDIO
                        TITLE "Song03"
                        INDEX 01 07:25:42
                    TRACK 04 AUDIO
                        TITLE "Song04"
                        INDEX 01 12:04:72)"
            << "%a/%A/%n - %t"
            << "Artist/__/01 - Song01.wav";

    QTest::newRow("3.3")
            << R"( REM GENRE Genre
                    REM DATE 2013
                    REM DISCID 123456789
                    REM COMMENT "ExactAudioCopy v0.99pb4"
                    PERFORMER "."
                    TITLE ".."
                    FILE "en.wav" WAVE
                    TRACK 01 AUDIO
                        TITLE "Song01"
                        INDEX 01 00:00:00
                    TRACK 02 AUDIO
                        TITLE "Song02"
                        INDEX 01 03:39:10
                    TRACK 03 AUDIO
                        TITLE "Song03"
                        INDEX 01 07:25:42
                    TRACK 04 AUDIO
                        TITLE "Song04"
                        INDEX 01 12:04:72)"
            << "%a%A/%n - %t"
            << "___/01 - Song01.wav";

    QTest::newRow("4.1")
            << R"( REM GENRE Genre
                    REM DATE 2013
                    REM DISCID 123456789
                    REM COMMENT "ExactAudioCopy v0.99pb4"
                    PERFORMER "Автор"
                    TITLE "Альбом"
                    FILE "en.wav" WAVE
                    TRACK 01 AUDIO
                        TITLE "Песнь"
                        INDEX 01 00:00:00
                    TRACK 02 AUDIO
                        TITLE "Song02"
                        INDEX 01 03:39:10
                    TRACK 03 AUDIO
                        TITLE "Song03"
                        INDEX 01 07:25:42
                    TRACK 04 AUDIO
                        TITLE "Song04"
                        INDEX 01 12:04:72)"
            << "%a_%A/%n - %t"
            << "Автор_Альбом/01 - Песнь.wav";

    QTest::newRow("5.1")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                TITLE     "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                    TITLE "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%A%a%t"
            << "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_123456789_.wav";

    QTest::newRow("5.2")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_"
                TITLE     "album----_album----_album----_album----_album----_album----_album----_album----_album----_album----_"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                TITLE     "track----_track----_track----_track----_track----_track----_track----_track----_track----_track----_"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%y - %n - [%a] - [%A]  - %t/%n"
            << "2013 - 01 - [artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_artist---_] - "
               "[album----_album----_album----_album----_album----_album----_album----_album----_album----_album----_]  - "
               "track----_track----_track--/01.wav";

    QTest::newRow("5.3")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "Very-verry long artist name, and another long artis name. Very-verry long artist name, and another long artis name"
                TITLE "Long album title, long album title, long album title"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                    TITLE "Long first track tile, long first track tile, long first track tile, long first track tile, long first track tile"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%y_%n - [%a] - [%A]  - %t/%y_%n - [%a] - [%A]  - %t"
            << "2013_01 - [Very-verry long artist name, and another long artis name. Very-verry long artist name, and an"
               "other long artis name] - [Long album title, long album title, long album title]  - Long first track"
               " tile, long first track tile, long first track /"
               "2013_01 - [Very-verry long artist name, and another long artis name. Very-verry long artist name, and an"
               "other long artis name] - [Long album title, long album title, long album title]  - Long first track"
               " tile, long first track tile, long first track .wav";

    QTest::newRow("5.4 UTF-8")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                TITLE     "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                    TITLE "123456789_123456789_123456789_123456789_Русский"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%A%a%t"
            << "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_Русск.wav";

    QTest::newRow("5.5 UTF-8")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                TITLE     "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                    TITLE "123456789_123456789_123456789_123456789_1Русский"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%A%a%t"
            << "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_1Русс.wav";

    QTest::newRow("5.6 UTF-8")
            << R"(
                REM GENRE "Genre"
                REM DATE 2013
                REM DISCID 123456789
                REM COMMENT "ExactAudioCopy v0.99pb4"
                PERFORMER "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                TITLE     "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
                FILE "en.wav" WAVE
                  TRACK 01 AUDIO
                    TITLE "123456789_123456789_123456789_123456789_12Русский"
                    INDEX 01 00:00:00
                  TRACK 02 AUDIO
                    TITLE "Song02"
                    INDEX 01 03:39:10
                  TRACK 03 AUDIO
                    TITLE "Song03"
                    INDEX 01 07:25:42
                  TRACK 04 AUDIO
                    TITLE "Song04"
                    INDEX 01 12:04:72
                )"
            << "%A%a%t"
            << "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_"
               "123456789_123456789_123456789_123456789_12Русс.wav";
}

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFilePath()
{
    QFETCH(QString, outDir);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);
    QFETCH(QString, cueFile);

    Settings::i()->selectProfile("WAV");
    Settings::i()->currentProfile().setOutFileDir(outDir);
    Settings::i()->currentProfile().setOutFilePattern(pattern);

    if (!cueFile.isEmpty())
        QFile::copy(mDataDir + "simple.cue", cueFile);
    else
        cueFile = mDataDir + "simple.cue";

    Disc *disc = loadFromCue(cueFile);

    QString result = disc->track(0)->resultFilePath();
    if (QFileInfo(result).absoluteFilePath() != QFileInfo(expected).absoluteFilePath()) {
        QString msg = QString("Compared values are not the same\n   Actual:   %1 [%2]\n   Expected: %3\n   CueFile: %4").arg(QFileInfo(result).absoluteFilePath(), result, expected, cueFile);
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

    QTest::newRow("1: /home/user/music")
            << "/home/user/music"
            << "%a/%y - %A/%n - %t"
            << "/home/user/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("2: ~/music")
            << "~/music"
            << "%a/%y - %A/%n - %t"
            << QDir::homePath() + "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("3: /music")
            << "/music"
            << "%a/%y - %A/%n - %t"
            << "/music/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("4: empty")
            << ""
            << "%a/%y - %A/%n - %t"
            << mDataDir + "/Artist/2013 - Album/01 - Song01.wav"
            << "";

    QTest::newRow("5: dot (.) with CdAudioFile")
            << "."
            << "%a/%y - %A/%n - %t"
            << dir("5: dot (.) with CdAudioFile") + "/Artist/2013 - Album/01 - Song01.wav"
            << dir("5: dot (.) with CdAudioFile") + "/simple.cue";

    QTest::newRow("6. empty with CdAudioFile")
            << ""
            << "%a/%y - %A/%n - %t"
            << dir("6. empty with CdAudioFile") + "/Artist/2013 - Album/01 - Song01.wav"
            << dir("6. empty with CdAudioFile") + "/simple.cue";

    QTest::newRow("7: ~")
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
        QFAIL(QString("Can't copy file %1 to %2").arg(testDataDir + cueFile, testCueFile).toLocal8Bit().data());

    if (!codepageBefore.isEmpty())
        Settings::i()->setValue(Settings::Tags_DefaultCodepage, codepageBefore);
    else
        Settings::i()->setValue(Settings::Tags_DefaultCodepage, "UTF-8");

    Disc *disc = loadFromCue(testCueFile);

    if (!codepageAfter.isEmpty())
        disc->setCodecName(codepageAfter);

    QStringList expected = this->readFile(TEST_DATA_DIR + sampleFile);

    QStringList result;
    // Result *************************
    // result << "GENRE:" << tracks.genre() << "\n";
    // resultSl << "ALBUM:" << tracks.album() << "\n";
    result << "DISCID:" << disc->discId() << "\n";

    for (int i = 0; i < disc->count(); ++i) {
        Track *track = disc->track(i);
        result << "Track " << (i + 1) << "\n";
        result << "  "
               << "INDEX:" << i << "\n";
        result << "  "
               << "TRACKNUM:" << track->trackNum() << "\n";
        result << "  "
               << "ALBUM:" << track->album() << "\n";
        result << "  "
               << "TITLE:" << track->title() << "\n";
        result << "  "
               << "ARTIST:" << track->artist() << "\n";
        result << "  "
               << "GENRE:" << track->genre() << "\n";
        result << "  "
               << "YEAR:" << track->date() << "\n";
    }

    // Result *************************

    writeFile(result, resultFile);
    writeFile(expected, expectedFile);

    if (result.join("") != expected.join("")) {
        QString msg = "The result is different from the expected. Use the following command for details:";
        QString cmd = QString("diff %1 %2").arg(expectedFile, resultFile);
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

 ************************************************/
void TestFlacon::testOutFormatGainArgs()
{
    QFETCH(QString, formatId);
    QFETCH(GainType, gainType);
    QFETCH(QString, expected);

    Settings::i()->setValue("Programs/metaflac", "/opt/metaflac");
    Settings::i()->setValue("Programs/mp3gain", "/opt/mp3gain");
    Settings::i()->setValue("Programs/vorbisgain", "/opt/vorbisgain");
    Settings::i()->setValue("Programs/wvgain", "/opt/wvgain");

    OutFormat *format = OutFormat::formatForId(formatId);
    if (!format)
        QFAIL(QString("Unknown format \"%1\"").arg(formatId).toLocal8Bit());

    Profile profile;
    profile.setGainType(gainType);

    Conv::Gain *gain = format->createGain(profile);
    QStringList args = gain->programArgs(QStringList() << "OutFile_01.wav"
                                                       << "OutFile_02.wav"
                                                       << "OutFile_03.wav",
                                         gainType);

    delete gain;

    QString result = args.join(" ");
    if (result != expected) {
        QString msg = QString("Compared values are not the same\n   Format   %1, gaintype: %2\n   Actual:   %3\n   Expected: %4").arg(formatId, gainTypeToString(gainType), result, expected);
        QFAIL(msg.toLocal8Bit());
    }
}

/************************************************

 ************************************************/
void TestFlacon::testOutFormatGainArgs_data()
{
    QTest::addColumn<QString>("formatId", nullptr);
    QTest::addColumn<GainType>("gainType", nullptr);
    QTest::addColumn<QString>("expected", nullptr);

    //*******************************************
    // FLAC
    //*******************************************
    QTest::newRow("Flac Track")
            << "FLAC"
            << GainType::Track
            << "/opt/metaflac --add-replay-gain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    QTest::newRow("Flac Album")
            << "FLAC"
            << GainType::Album
            << "/opt/metaflac --add-replay-gain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    // MP3
    //*******************************************
    QTest::newRow("Mp3 Track")
            << "MP3"
            << GainType::Track
            << "/opt/mp3gain -a -c "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    QTest::newRow("Mp3 Track")
            << "MP3"
            << GainType::Album
            << "/opt/mp3gain -a -c "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    // Ogg
    //*******************************************
    QTest::newRow("Ogg Track")
            << "OGG"
            << GainType::Track
            << "/opt/vorbisgain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    QTest::newRow("Ogg Album")
            << "OGG"
            << GainType::Album
            << "/opt/vorbisgain --album "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    // WavPack
    //*******************************************
    QTest::newRow("WavPack Track")
            << "WV"
            << GainType::Track
            << "/opt/wvgain "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";

    //*******************************************
    QTest::newRow("WavPack Album")
            << "WV"
            << GainType::Album
            << "/opt/wvgain "
               "-a "
               "OutFile_01.wav OutFile_02.wav OutFile_03.wav";
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCueIndex()
{
    QString string = QTest::currentDataTag();
    QFETCH(QString, expected);

    bool cdQuality = string.toUpper().startsWith("CD");
    string         = string.mid(2).trimmed();

    QString  id1Str = string.section("-", 0, 0).trimmed();
    CueIndex idx1(id1Str);

    QString id2Str = string.section("-", 1, 1).trimmed();
    if (!id2Str.isEmpty()) {
        CueIndex idx2(id2Str);
        idx1 = idx1 - idx2;
    }

    QString result = idx1.toString(cdQuality);
    QCOMPARE(result, expected);
}

/************************************************
 *
 ************************************************/
void TestFlacon::testCueIndex_data()
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
