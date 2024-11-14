/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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

#include "flacontest.h"
#include <QTest>
#include <QMap>
#include <QString>
#include "types.h"
#include "patternexpander.h"

class TestPatternExpander : public PatternExpander
{
public:
    static int lastDirSeparattor(const QString &pattern)
    {
        return PatternExpander::lastDirSeparattor(pattern);
    }
};

/************************************************
 *
 ************************************************/
static QMap<QString, QString>
parseTrackData(const QString &str)
{
    QMap<QString, QString> res;

    foreach (QString line, str.split("\n")) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        QString key = line.section(":", 0, 0).trimmed();
        QString val = line.section(":", 1).trimmed();

        res.insert(key, val);
    }

    return res;
}

/************************************************
 *
 ************************************************/
void TestFlacon::testPatternExpander()
{
    QFETCH(QString, pattern);
    QFETCH(QString, trackData);
    QFETCH(QString, expected);

    QMap<QString, QString> trackValues = parseTrackData(trackData);

    AlbumTags albumTags;
    TrackTags trackTags;

    albumTags.setTrackCount(trackValues.value("trackCount").toInt());
    trackTags.setTrackNum(trackValues.value("trackNum").toInt());
    albumTags.setDiscCount(trackValues.value("discCount").toInt());
    albumTags.setDiscNum(trackValues.value("discNum").toInt());

    albumTags.setAlbum(trackValues.value("album"));
    trackTags.setTitle(trackValues.value("title"));
    trackTags.setArtist(trackValues.value("artist"));
    trackTags.setGenre(trackValues.value("genre"));
    trackTags.setDate(trackValues.value("date"));

    PatternExpander expander(albumTags, trackTags, trackTags);

    QString result = expander.expand(pattern);

    QCOMPARE(result, expected);
}

/************************************************
 *
 ************************************************/
void TestFlacon::testPatternExpander_data()
{
    QTest::addColumn<QString>("pattern", nullptr);
    QTest::addColumn<QString>("trackData", nullptr);
    QTest::addColumn<QString>("expected", nullptr);

    QTest::newRow("01")
            << ""
            << ""
            << "";

    QTest::newRow("02")
            << "static"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "static";

    QTest::newRow("03")
            << "%a/{%y - }%A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/01 - Track title";

    QTest::newRow("04")
            << "%a -{ %y }%A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band - 1999 Hits/01 - Track title";

    QTest::newRow("05")
            << "%a/{%y - }%A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               )"
            << "Band/Hits/01 - Track title";

    QTest::newRow("06")
            << "%a/%y - %A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               )"
            << "Band/ - Hits/01 - Track title";

    QTest::newRow("07")
            << "%a/{%y - }%A/%d of %D/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   5
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/01 of 02/05 - Track title";

    QTest::newRow("08")
            << "%a/{%y - }%A/%d of %D/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   5
               discCount:  1
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/01 of 01/05 - Track title";

    QTest::newRow("09")
            << "%d of %D/%n of %N"
            << R"(
               )"
            << "00 of 00/00 of 00";

    QTest::newRow("10")
            << "{%d of %D}/{%n of %N}"
            << R"(
               trackCount: 1
               trackNum:   1
               discCount:  1
               discNum:    1
               )"
            << "/01 of 01";

    QTest::newRow("11")
            << "%a/{%y - }%A/{%d of %D/}%n - %t"
            << R"(
               trackCount: 10
               trackNum:   5
               discCount:  1
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/05 - Track title";

    QTest::newRow("12")
            << "%a/{Music}%A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/{Music}Hits/01 - Track title";

    QTest::newRow("13")
            << "%a/{Music}%%A/%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/{Music}%A/01 - Track title";

    QTest::newRow("14 - disk number")
            << "%a/{%y - }%A/{%d-}%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    2
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/02-01 - Track title";

    QTest::newRow("15 - disk number")
            << "%a/{%y - }%A{/%d/}%n - %t"
            << R"(
               trackCount: 10
               trackNum:   1
               discCount:  2
               discNum:    1
               album:      Hits
               title:      Track title
               artist:     Band
               genre:      Rock
               date:       1999
               )"
            << "Band/1999 - Hits/01/01 - Track title";
}

/**************************************
 *
 **************************************/
void TestFlacon::testPatternExpanderLastDir()
{
    QFETCH(QString, pattern);
    QFETCH(int, expected);

    int actual = TestPatternExpander::lastDirSeparattor(pattern);
    QCOMPARE(actual, expected);
}

/**************************************
 *
 **************************************/
void TestFlacon::testPatternExpanderLastDir_data()
{
    QTest::addColumn<QString>("pattern", nullptr);
    QTest::addColumn<int>("expected", nullptr);

    QTest::newRow("01") << "" << -1;
    QTest::newRow("02") << "/" << 0;
    QTest::newRow("03") << "/a" << 0;
    QTest::newRow("04") << "a/b" << 1;
    QTest::newRow("05") << "%a/{%y - }%A/%n - %t" << 12;
    QTest::newRow("06") << "%a/{%y - }{%A/}%n - %t" << 2;
    QTest::newRow("07") << "{%y - }{%A/}%n - %t" << -1;
    QTest::newRow("08") << "%a/{%y - }{%A/}%n/ - %t" << 17;
}
