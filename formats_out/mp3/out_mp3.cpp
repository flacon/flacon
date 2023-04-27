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
#include "mp3metadatawriter.h"
#include <QDebug>

static constexpr char VBR_MEDIUM[]   = "vbrMedium";
static constexpr char VBR_STATDARD[] = "vbrStandard";
static constexpr char VBR_EXTRIME[]  = "vbrExtreme";
static constexpr char VBR_QUALITY[]  = "vbrQuality";
static constexpr char CBR_INSANE[]   = "cbrInsane";
static constexpr char CBR_KBPS[]     = "cbrKbps";
static constexpr char ABR_KBPS[]     = "abrKbps";

/************************************************

 ************************************************/
OutFormat_Mp3::OutFormat_Mp3()
{
    mId      = "MP3";
    mExt     = "mp3";
    mName    = "MP3";
    mOptions = FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Mp3::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Preset", VBR_STATDARD);
    res.insert("Bitrate", 320);
    res.insert("Quality", 4);
    res.insert("ReplayGain", gainTypeToString(GainType::Disable));
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Mp3::configPage(QWidget *parent) const
{
    return new ConfigPage_Mp3(parent);
}

/************************************************

 ************************************************/
ExtProgram *OutFormat_Mp3::encoderProgram(const Profile &) const
{
    return ExtProgram::lame();
}

/************************************************

 ************************************************/
QStringList OutFormat_Mp3::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;

    args << "--silent";

    // Settings .................................................
    QString preset = profile.encoderValue("Preset").toString();

    if (preset == VBR_MEDIUM) {
        args << "--preset"
             << "medium";
    }

    else if (preset == VBR_STATDARD) {
        args << "--preset"
             << "standard";
    }

    else if (preset == VBR_EXTRIME) {
        args << "--preset"
             << "extreme";
    }

    else if (preset == CBR_INSANE) {
        args << "--preset"
             << "insane";
    }

    else if (preset == CBR_KBPS) {
        args << "--preset"
             << "cbr" << profile.encoderValue("Bitrate").toString();
    }

    else if (preset == ABR_KBPS) {
        args << "--preset" << profile.encoderValue("Bitrate").toString();
    }

    else if (preset == VBR_QUALITY) {
        int quality = profile.encoderValue("Quality").toInt();
        args << "-V" << QString("%1").arg(9 - quality);
    }

    // ReplayGain ...............................................
    if (strToGainType(profile.encoderValue("ReplayGain").toString()) != GainType::Track) {
        args << "--noreplaygain";
    }

    // Files ....................................................
    args << "-";
    args << outFile;

    return args;
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Mp3::createMetadataWriter(const QString &filePath) const
{
    return new Mp3MetaDataWriter(filePath);
}

/************************************************

 ************************************************/
ConfigPage_Mp3::ConfigPage_Mp3(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    mp3PresetCbx->addItem(tr("VBR medium"), VBR_MEDIUM);
    mp3PresetCbx->addItem(tr("VBR standard"), VBR_STATDARD);
    mp3PresetCbx->addItem(tr("VBR extreme"), VBR_EXTRIME);
    mp3PresetCbx->addItem(tr("VBR quality"), VBR_QUALITY);
    mp3PresetCbx->addItem(tr("CBR insane"), CBR_INSANE);
    mp3PresetCbx->addItem(tr("CBR kbps"), CBR_KBPS);
    mp3PresetCbx->addItem(tr("ABR kbps"), ABR_KBPS);

    fillBitrateComboBox(mp3BitrateCbx, QList<int>() << 32 << 40 << 48 << 56 << 64 << 80 << 96 << 112 << 128 << 160 << 192 << 224 << 256 << 320);

    QString css = "<style type='text/css'>\n"
                  "qbody { font-size: 9px; }\n"
                  "dt { font-weight: bold; }\n"
                  "dd { margin-left: 8px; margin-bottom: 8px; }\n"
                  "</style>\n";

    QString toolTip = tr(
            R"(<dt>VBR medium</dt>
      <dd>By using a medium Variable BitRate, this preset should provide near transparency to most people and most music.</dd>

      <dt>VBR standard</dt>
      <dd>By using a standard Variable BitRate, this preset should generally be transparent to most people on most music and is already quite high in quality.</dd>

      <dt>VBR extreme</dt>
      <dd>By using the highest possible Variable BitRate, this preset provides slightly higher quality than the standard mode if you have extremely good hearing or high-end audio equipment.</dd>

      <dt>VBR quality</dt>
      <dd>This Variable BitRate option lets you specify the output quality.</dd>

      <dt>CBR insane</dt>
      <dd>If you must have the absolute highest quality with no regard to file size, you'll achieve it by using this Constant BitRate.</dd>

      <dt>CBR kbps</dt>
      <dd>Using this Constant BitRate preset will usually give you good quality at a specified bitrate.</dd>

      <dt>ABR kbps</dt>
      <dd>Using this Average BitRate preset will usually give you higher quality than the Constant BitRate option for a specified bitrate.</dd>
      )",
            "Tooltip for the Mp3 presets combobox on preferences dialog.");

    mp3PresetCbx->setToolTip(css + toolTip);

    setLossyToolTip(mp3QualitySlider);
    mp3QualitySpin->setToolTip(mp3QualitySlider->toolTip());
    mp3QualityLabel->setToolTip(mp3QualitySlider->toolTip());

    connect(mp3PresetCbx, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ConfigPage_Mp3::mp3PresetCbxCanged);

    mp3PresetCbxCanged(mp3PresetCbx->currentIndex());
}

/************************************************

 ************************************************/
void ConfigPage_Mp3::load(const Profile &profile)
{
    loadWidget(profile, "Preset", mp3PresetCbx);
    loadWidget(profile, "Bitrate", mp3BitrateCbx);
    loadWidget(profile, "Quality", mp3QualitySpin);
    mp3QualitySlider->setValue(mp3QualitySpin->value());
}

/************************************************

 ************************************************/
void ConfigPage_Mp3::save(Profile *profile)
{
    saveWidget(profile, "Preset", mp3PresetCbx);
    saveWidget(profile, "Bitrate", mp3BitrateCbx);
    saveWidget(profile, "Quality", mp3QualitySpin);
}

/************************************************

 ************************************************/
void ConfigPage_Mp3::mp3PresetCbxCanged(int index)
{
    QString preset = mp3PresetCbx->itemData(index).toString();

    bool enable = (preset == ABR_KBPS or preset == CBR_KBPS);
    mp3BitrateLabel->setEnabled(enable);
    mp3BitrateCbx->setEnabled(enable);

    enable = (preset == VBR_QUALITY);
    mp3QualityLabel->setEnabled(enable);
    mp3QualitySlider->setEnabled(enable);
    mp3QualitySpin->setEnabled(enable);
}
