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


#include "testflacon.h"
#include "tools.h"
#include "outformat.h"
#include "disc.h"

#include <QTest>
#include <QString>
#include <QDebug>


/************************************************

 ************************************************/
void TestFlacon::testOutFormatEncoderArgs()
{
    QFETCH(QString, formatId);
    QFETCH(SettingsValues, config);
    QFETCH(QString, expected);

    for (auto prog: Settings::i()->programs()) {
        Settings::i()->setValue("Programs/" + prog, prog);
    }

    OutFormat *fmt = OutFormat::formatForId(formatId);
    if (!fmt)
        QFAIL(QString("Unknown format \"%1\"").arg(formatId).toLocal8Bit());

    Profile profile(*fmt, formatId);
    if (!profile.isValid())
        QFAIL(QString("Invalid profile for \"%1\"").arg(formatId).toLocal8Bit());

    for (auto i=config.constBegin(); i!=config.constEnd(); ++i) {
        profile.setValue(i.key(), i.value());
    }


    Disc *disk = standardDisk();
    QStringList args = profile.encoderArgs(disk->track(0), "OutFile.wav");

    QString result = args.join(" ");
    if (result != expected)
    {
        QString msg = QString("Compared values are not the same\n   Format   %1\n   Actual:   %2\n   Expected: %3").arg(
                    formatId,
                    result,
                    expected);
        QFAIL(msg.toLocal8Bit());
    }
}


/************************************************

 ************************************************/
void TestFlacon::testOutFormatEncoderArgs_data()
{
    QTest::addColumn<QString>("formatId",      nullptr);
    QTest::addColumn<SettingsValues>("config", nullptr);
    QTest::addColumn<QString>("expected",      nullptr);

    SettingsValues cfg;

    //*******************************************
    // FLAC
    //*******************************************  
    cfg.clear();
    cfg.insert("Compression", 5);
    cfg.insert("ReplayGain",  "Disable");

    QTest::newRow("Flac_1")
            << "FLAC"
            << cfg
            << "flac --force --silent "
               "--compression-level-5 "
               "--tag artist=Artist --tag album=Album --tag genre=Genre --tag date=2013 --tag title=Song01 --tag albumartist=Artist "
               "--tag comment=ExactAudioCopy v0.99pb4 --tag discId=123456789 "
               "--tag tracknumber=1 --tag totaltracks=4 --tag tracktotal=4 "
               "--tag disc=1 --tag discnumber=1 --tag disctotal=1 "
               "- -o OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Compression", 1);
    cfg.insert("ReplayGain",  "Disable");

    QTest::newRow("Flac_2")
            << "FLAC"
            << cfg
            << "flac --force --silent "
               "--compression-level-1 "
               "--tag artist=Artist --tag album=Album --tag genre=Genre --tag date=2013 --tag title=Song01 --tag albumartist=Artist "
               "--tag comment=ExactAudioCopy v0.99pb4 --tag discId=123456789 "
               "--tag tracknumber=1 --tag totaltracks=4 --tag tracktotal=4 "
               "--tag disc=1 --tag discnumber=1 --tag disctotal=1 "
               "- -o OutFile.wav";


    //*******************************************
    // AAC
    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality",   true);
    cfg.insert("Quality",      500);

    QTest::newRow("01 AAC_1")
            << "AAC"
            << cfg
            << "faac -w "
               "-q 500 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --disc 1/1 --year 2013 "
               "--comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";


    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality", true);
    cfg.insert("Quality",    10);

    QTest::newRow("02 AAC_2")
            << "AAC"
            << cfg
            << "faac -w "
               "-q 10 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --disc 1/1 --year 2013 "
               "--comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";

    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality", false);
    cfg.insert("Quality",    500);
    cfg.insert("Bitrate",    64);

    QTest::newRow("03 AAC_3")
            << "AAC"
            << cfg
            << "faac -w "
               "-b 64 "
               "--artist Artist --title Song01 --genre Genre --album Album --track 1/4 --disc 1/1 --year 2013 "
               "--comment ExactAudioCopy v0.99pb4 -o OutFile.wav -";


    //*******************************************
    // MP3
    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrMedium");

    QTest::newRow("01 MP3_vbrMedium")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset medium "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrStandard");

    QTest::newRow("02 MP3_vbrStandard")
            << "MP3"
            << cfg
            << "lame "
               "--silent "
               "--preset standard "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";



    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrStandardFast");

    QTest::newRow("04 MP3_vbrStandardFast")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset fast standard "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrExtreme");

    QTest::newRow("05 MP3_vbrExtreme")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset extreme "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrExtremeFast");

    QTest::newRow("06 MP3_vbrExtremeFast")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset fast extreme "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "cbrInsane");

    QTest::newRow("07 MP3_cbrInsane")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset insane "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "cbrKbps");
    cfg.insert("Bitrate",    64);

    QTest::newRow("08 MP3_cbrKbps64")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset cbr 64 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "cbrKbps");
    cfg.insert("Bitrate",    128);

    QTest::newRow("09 MP3_cbrKbps128")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset cbr 128 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "abrKbps");
    cfg.insert("Bitrate",    64);

    QTest::newRow("10 MP3_abrKbps64")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset 64 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "abrKbps");
    cfg.insert("Bitrate",    128);

    QTest::newRow("11 MP3_abrKbps128")
            << "MP3"
            << cfg
            << "lame --silent "
               "--preset 128 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrQuality");
    cfg.insert("Quality",    0);

    QTest::newRow("12 MP3_vbrQuality0")
            << "MP3"
            << cfg
            << "lame --silent "
               "-V 9 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrQuality");
    cfg.insert("Quality",    4);

    QTest::newRow("13 MP3_vbrQuality4")
            << "MP3"
            << cfg
            << "lame --silent "
               "-V 5 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";

    //*******************************************
    cfg.clear();
    cfg.insert("Preset",     "vbrQuality");
    cfg.insert("Quality",    9);

    QTest::newRow("14 MP3_vbrQuality9")
            << "MP3"
            << cfg
            << "lame --silent "
               "-V 0 "
               "--noreplaygain --add-id3v2 --ta Artist --tl Album --tg Genre --ty 2013 --tt Song01 --tv TPE2=Artist "
               "--tc ExactAudioCopy v0.99pb4 --tn 1/4 --tv TPOS=1 "
               "- OutFile.wav";



    //*******************************************
    // Ogg
    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality",    true);
    cfg.insert("Quality",       5);
    cfg.insert("MinBitrate",    "");
    cfg.insert("NormBitrate",   "");
    cfg.insert("MaxBitrate",    "");

    QTest::newRow("01 Ogg Quality 5")
            << "OGG"
            << cfg
            << "oggenc --quiet "
               "-q 5 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment album_artist=Artist "
               "--comment COMMENT=ExactAudioCopy v0.99pb4 --comment DISCID=123456789 "
               "--tracknum 1 --comment totaltracks=4 --comment tracktotal=4 "
               "--comment disc=1 --comment discnumber=1 --comment disctotal=1 "
               "-o OutFile.wav -";


    //*******************************************
    cfg.clear();
    cfg.insert("Quality",       10);

    QTest::newRow("02 Ogg Quality 10")
            << "OGG"
            << cfg
            << "oggenc --quiet "
               "-q 10 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment album_artist=Artist "
               "--comment COMMENT=ExactAudioCopy v0.99pb4 --comment DISCID=123456789 "
               "--tracknum 1 --comment totaltracks=4 --comment tracktotal=4 "
               "--comment disc=1 --comment discnumber=1 --comment disctotal=1 "
               "-o OutFile.wav -";


    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality",    false);
    cfg.insert("MinBitrate",    "");
    cfg.insert("NormBitrate",   "");
    cfg.insert("MaxBitrate",    "");

    QTest::newRow("03 Ogg Bitrate 0 0 0")
            << "OGG"
            << cfg
            << "oggenc --quiet "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment album_artist=Artist "
               "--comment COMMENT=ExactAudioCopy v0.99pb4 --comment DISCID=123456789 "
               "--tracknum 1 --comment totaltracks=4 --comment tracktotal=4 "
               "--comment disc=1 --comment discnumber=1 --comment disctotal=1 "
               "-o OutFile.wav -";

    //*******************************************
    cfg.clear();
    cfg.insert("UseQuality",    false);
    cfg.insert("MinBitrate",    64);
    cfg.insert("NormBitrate",   128);
    cfg.insert("MaxBitrate",    350);

    QTest::newRow("04 Ogg Bitrate 64 128 350")
            << "OGG"
            << cfg
            << "oggenc --quiet "
               "-b 128 -m 64 -M 350 "
               "--artist Artist --album Album --genre Genre --date 2013 --title Song01 --comment album_artist=Artist "
               "--comment COMMENT=ExactAudioCopy v0.99pb4 --comment DISCID=123456789 "
               "--tracknum 1 --comment totaltracks=4 --comment tracktotal=4 "
               "--comment disc=1 --comment discnumber=1 --comment disctotal=1 "
               "-o OutFile.wav -";


    //*******************************************
    // WavPack
    //*******************************************
    cfg.clear();
    cfg.insert("Compression", 0);

    QTest::newRow("01 WavPack Quality 0")
            << "WV"
            << cfg
            << "wavpack -q "
               "-f "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w Album Artist=Artist "
               "-w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 "
               "-w Track=1/4 -w Part=1 "
               "- -o OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Compression",   1);

    QTest::newRow("02 WavPack Quality 1")
            << "WV"
            << cfg
            << "wavpack -q "
               "-h "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w Album Artist=Artist "
               "-w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 "
               "-w Track=1/4 -w Part=1 "
               "- -o OutFile.wav";


    //*******************************************
    cfg.clear();
    cfg.insert("Compression",   2);

    QTest::newRow("03 WavPack Quality 2")
            << "WV"
            << cfg
            << "wavpack -q "
               "-hh "
               "-w Artist=Artist -w Album=Album -w Genre=Genre -w Year=2013 -w Title=Song01 -w Album Artist=Artist "
               "-w DiscId=123456789 -w Comment=ExactAudioCopy v0.99pb4 "
               "-w Track=1/4 -w Part=1 "
               "- -o OutFile.wav";

}
