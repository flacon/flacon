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


#include "out_mp3.h"
#include "settings.h"
#include <QDebug>


/************************************************

 ************************************************/
OutFormat_Mp3::OutFormat_Mp3()
{
    mId   = "MP3";
    mExt  = "mp3";
    mName = "MP3";
    mOptions = FormatOption::SupportGain;
}


/************************************************

 ************************************************/
QStringList OutFormat_Mp3::encoderArgs(const Profile &profile, const Track *track, const QString &outFile) const
{
    QStringList args;

    args << Settings::i()->programName(encoderProgramName());
    args << "--silent";

    // Settings .................................................
    QString preset = profile.value("Preset").toString();

    if (preset == "vbrMedium")
    {
        args << "--preset" << "medium";
    }

    else if (preset == "vbrStandard")
    {
        args << "--preset" << "standard";
    }

    else if (preset == "vbrStandardFast")
    {
        args << "--preset" << "fast" << "standard";
    }

    else if (preset == "vbrExtreme")
    {
        args << "--preset" << "extreme";
    }

    else if (preset == "vbrExtremeFast")
    {
        args << "--preset" << "fast" << "extreme";
    }

    else if (preset == "cbrInsane")
    {
        args << "--preset" << "insane";
    }

    else if (preset == "cbrKbps")
    {
        args << "--preset" << "cbr" << profile.value("Bitrate").toString();
    }

    else if (preset == "abrKbps")
    {
        args << "--preset" << profile.value("Bitrate").toString();
    }

    else if (preset == "vbrQuality")
    {
        int quality = profile.value("Quality").toInt();
        args << "-V" << QString("%1").arg(9 - quality);
    }

    // ReplayGain ...............................................
    if (strToGainType(profile.value("ReplayGain").toString()) != GainType::Track)
    {
        args << "--noreplaygain";
    }

    // Tags .....................................................
    args << "--add-id3v2";
    //#args << "--id3v2-only"
    if (!track->artist().isEmpty())
        args << "--ta" << track->artist();

    if (!track->album().isEmpty())
        args << "--tl" << track->album();

    if (!track->genre().isEmpty())
        args << "--tg" << track->genre();

    if (!track->date().isEmpty())
        args << "--ty" << track->date();

    if (!track->title().isEmpty())
        args << "--tt" << track->title();

    if (!track->tag(TagId::AlbumArtist).isEmpty())
        args << "--tv" << QString("TPE2=%1").arg(track->tag(TagId::AlbumArtist));

    if (!track->comment().isEmpty())
        args << "--tc" << track->comment();

    args << "--tn" << QString("%1/%2").arg(track->trackNum()).arg(track->trackCount());
    args << "--tv" << QString("TPOS=%1").arg(track->diskNum());

    // Files ....................................................
    args << "-";
    args << outFile;

    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Mp3::gainArgs(const QStringList &files, const GainType) const
{
    QStringList args;
    args <<  args << Settings::i()->programName(gainProgramName());
    args << "-a"; // Album gain
    args << "-c"; // ignore clipping warning when applying gain

    args << files;

    return args;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Mp3::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Preset",           "vbrStandardFast");
    res.insert("Bitrate",          320);
    res.insert("Quality",          4);
    res.insert("ReplayGain",       gainTypeToString(GainType::Disable));
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Mp3::configPage(const Profile &profile, QWidget *parent) const
{
    return new ConfigPage_Mp3(profile, parent);
}


/************************************************

 ************************************************/
ConfigPage_Mp3::ConfigPage_Mp3(const Profile &profile, QWidget *parent):
    EncoderConfigPage(profile, parent)
{
    setupUi(this);

    mp3PresetCbx->addItem(tr("VBR medium"),       "vbrMedium");
    mp3PresetCbx->addItem(tr("VBR standard"),      "vbrStandard");
    mp3PresetCbx->addItem(tr("VBR standard fast"), "vbrStandardFast");
    mp3PresetCbx->addItem(tr("VBR extreme"),       "vbrExtreme");
    mp3PresetCbx->addItem(tr("VBR extreme fast"),  "vbrExtremeFast");
    mp3PresetCbx->addItem(tr("VBR quality"),       "vbrQuality");
    mp3PresetCbx->addItem(tr("CBR insane"),        "cbrInsane");
    mp3PresetCbx->addItem(tr("CBR kbps"),          "cbrKbps");
    mp3PresetCbx->addItem(tr("ABR kbps"),          "abrKbps");



    fillBitrateComboBox(mp3BitrateCbx,  QList<int>() << 32 << 40 << 48 << 56
                                                     << 64 << 80 << 96 << 112
                                                     << 128 << 160 << 192
                                                     << 224 << 256 << 320);


    QString css = "<style type='text/css'>\n"
          "qbody { font-size: 9px; }\n"
          "dt { font-weight: bold; }\n"
          "dd { margin-left: 8px; margin-bottom: 8px; }\n"
          "</style>\n";

    mp3PresetCbx->setToolTip(css + mp3PresetCbx->toolTip());

    setLossyToolTip(mp3QualitySlider);
    mp3QualitySpin->setToolTip(mp3QualitySlider->toolTip());
    mp3QualityLabel->setToolTip(mp3QualitySlider->toolTip());

    connect(mp3PresetCbx, SIGNAL(currentIndexChanged(int)), this, SLOT(mp3PresetCbxCanged(int)));
    mp3PresetCbxCanged(mp3PresetCbx->currentIndex());
}


/************************************************

 ************************************************/
void ConfigPage_Mp3::load()
{
    loadWidget("Preset",      mp3PresetCbx);
    loadWidget("Bitrate",     mp3BitrateCbx);
    loadWidget("Quality",     mp3QualitySpin);
    mp3QualitySlider->setValue(mp3QualitySpin->value());
}


/************************************************

 ************************************************/
void ConfigPage_Mp3::save()
{
    saveWidget("Preset",     mp3PresetCbx);
    saveWidget("Bitrate",    mp3BitrateCbx);
    saveWidget("Quality",    mp3QualitySpin);
}


/************************************************

 ************************************************/
void ConfigPage_Mp3::mp3PresetCbxCanged(int index)
{
    QString preset = mp3PresetCbx->itemData(index).toString();

    bool enable = (preset == "abrKbps" or preset=="cbrKbps");
    mp3BitrateLabel->setEnabled(enable);
    mp3BitrateCbx->setEnabled(enable);

    enable = (preset == "vbrQuality");
    mp3QualityLabel->setEnabled(enable);
    mp3QualitySlider->setEnabled(enable);
    mp3QualitySpin->setEnabled(enable);
}
