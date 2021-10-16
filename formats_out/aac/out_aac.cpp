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

#include "out_aac.h"
#include "inputaudiofile.h"

/************************************************

 ************************************************/
OutFormat_Aac::OutFormat_Aac()
{
    mId      = "AAC";
    mExt     = "m4a";
    mName    = "AAC";
    mOptions = FormatOption::NoOptions;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Aac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality", true);
    res.insert("Quality", 100);
    res.insert("Bitrate", 256);
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Aac::configPage(const Profile &profile, QWidget *parentr) const
{
    return new ConfigPage_Acc(profile, parentr);
}

/************************************************

 ************************************************/
Conv::Encoder *OutFormat_Aac::createEncoder() const
{
    return new Encoder_Aac();
}

/************************************************

 ************************************************/
ConfigPage_Acc::ConfigPage_Acc(const Profile &profile, QWidget *parent) :
    EncoderConfigPage(profile, parent)
{
    setupUi(this);

    setLossyToolTip(aacQualitySpin);
    aacQualitySlider->setToolTip(aacQualitySpin->toolTip());
    fillBitrateComboBox(aacBitrateCbx, QList<int>() << 64 << 80 << 128 << 160 << 192 << 224 << 256 << 288 << 320);

    connect(aacUseQualityCheck, &QCheckBox::toggled,
            this, &ConfigPage_Acc::useQualityChecked);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::load()
{
    loadWidget("UseQuality", aacUseQualityCheck);
    loadWidget("Quality", aacQualitySpin);
    loadWidget("Bitrate", aacBitrateCbx);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::save()
{
    saveWidget("UseQuality", aacUseQualityCheck);
    saveWidget("Quality", aacQualitySpin);
    saveWidget("Bitrate", aacBitrateCbx);
}

/************************************************
 *
 ************************************************/
void ConfigPage_Acc::useQualityChecked(bool checked)
{
    qualityLabel->setEnabled(checked);
    aacQualitySlider->setEnabled(checked);
    aacQualitySpin->setEnabled(checked);

    bitrateLabel->setEnabled(!checked);
    aacBitrateCbx->setEnabled(!checked);
}

/************************************************

 ************************************************/
QStringList Encoder_Aac::programArgs() const
{
    QStringList args;

    args << programPath();

    args << "-w"; // Wrap  AAC  data  in  an MP4 container.

    // Quality settings .........................................
    if (profile().value("UseQuality").toBool())
        args << "-q" << profile().value("Quality").toString();
    else
        args << "-b" << profile().value("Bitrate").toString();

    // Tags .....................................................
    // --artist artist
    //     Set artist to artist
    if (!track().artist().isEmpty())
        args << "--artist" << track().artist();

    // --writer writer
    //     Set writer to writer

    // --title title
    //     Set title to title
    if (!track().title().isEmpty())
        args << "--title" << track().title();

    // --genre genre
    //     Set genre to genre
    if (!track().genre().isEmpty())
        args << "--genre" << track().genre();

    // --album album
    //     Set album to album
    if (!track().album().isEmpty())
        args << "--album" << track().album();

    // --compilation
    //     Set compilation

    // --track track
    //     Set track to track in the format “number/total”
    args << "--track" << QString("%1/%2").arg(track().trackNum()).arg(track().trackCount());

    // --disc disc
    //     Set disc to disc in the format “number/total”
    args << "--disc" << QString("%1/%2").arg(track().discNum()).arg(track().discCount());

    // --year year
    //     Set year to year
    if (!track().date().isEmpty())
        args << "--year" << track().date();

    // --cover-art file
    //     Set cover art to image in file; supported formats are GIF, JPEG, and PNG.

    // --comment comment
    //     Set comment to comment
    if (!track().comment().isEmpty())
        args << "--comment" << track().comment();

    if (!coverFile().isEmpty()) {
        args << "--cover-art" << coverFile();
    }

    args << "-o" << outFile();
    args << "-";
    return args;
}
