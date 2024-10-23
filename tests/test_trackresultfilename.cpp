/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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
#include "project.h"

#include <QDebug>

/************************************************

 ************************************************/
void TestFlacon::testTrackResultFileName()
{
    QFETCH(QString, cue);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    Profile profile = Profile("WAV");
    profile.setOutFilePattern(pattern);

    Project::instance()->clear();

    QString cueFile = dir() + "/input.cue";
    writeTextFile(cueFile, cue);

    Disc *disc = loadFromCue(cueFile);

    QString result = profile.resultFileName(disc->tracks().first());

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

    QTest::newRow("6.1 disk number")
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
            << "%a/%y - %A/{%d/}%n - %t"
            << "Artist/2013 - Album/01 - Song01.wav";

    QTest::newRow("6.2 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 1\n"
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
            << "%a/%y - %A/{%d/}%n - %t"
            << "Artist/2013 - Album/01/01 - Song01.wav";

    QTest::newRow("6.3 disk number")
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
            << "%a/%y - %A/%d/%n - %t"
            << "Artist/2013 - Album/01/01 - Song01.wav";

    QTest::newRow("6.4 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 1\n"
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
            << "%a/%y - %A/%d/%n - %t"
            << "Artist/2013 - Album/01/01 - Song01.wav";

    QTest::newRow("6.5 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 2\n"
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
            << "%a/%y - %A/%d/%n - %t"
            << "Artist/2013 - Album/02/01 - Song01.wav";

    QTest::newRow("6.6 disk number")
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
            << "%a/%y - %A/{DISK%d/}%n - %t"
            << "Artist/2013 - Album/01 - Song01.wav";

    QTest::newRow("6.7 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 1\n"
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
            << "%a/%y - %A/{DISK%d/}%n - %t"
            << "Artist/2013 - Album/DISK01/01 - Song01.wav";

    QTest::newRow("6.8 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 1\n"
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
            << "%a/{%y - }%A/{%d-}%n - %t"
            << "Artist/2013 - Album/01-01 - Song01.wav";

    QTest::newRow("6.9 disk number")
            << "REM GENRE \"Genre\"\n"
               "REM DATE 2013\n"
               "REM DISCID 123456789\n"
               "REM TOTALDISCS 2\n"
               "REM DISCNUMBER 2\n"
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
            << "%a/{%y - }%A/{%d-}%n - %t"
            << "Artist/2013 - Album/02-01 - Song01.wav";
}

/**************************************
 *
 **************************************/
void TestFlacon::testPregapTrackResultFileName()
{
    QFETCH(QString, cue);
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    Profile profile = Profile("WAV");
    profile.setOutFilePattern(pattern);

    Project::instance()->clear();

    QString cueFile = dir() + "/input.cue";
    writeTextFile(cueFile, cue);

    Disc *disc = loadFromCue(cueFile);

    PregapTrack track(*disc->tracks().first());
    QString     result = profile.resultFileName(&track);

    if (result != expected) {
        QString msg = QString("Compared values are not the same\n   Pattern   %1\n   Actual:   %2\n   Expected: %3").arg(pattern, result, expected);
        QFAIL(msg.toLocal8Bit());
    }
    disc->deleteLater();
}

/**************************************
 *
 **************************************/
void TestFlacon::testPregapTrackResultFileName_data()
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
            << "Artist/2013 - Album/00 - (HTOA).wav";
}
