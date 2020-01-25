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


#include "out_ogg.h"
#include "disk.h"
#include "settings.h"
#include <QDebug>
#include <math.h>

/************************************************

 ************************************************/
OutFormat_Ogg::OutFormat_Ogg()
{
    mId   = "OGG";
    mExt  = "ogg";
    mName = "OGG";
}


/************************************************

 ************************************************/
QStringList OutFormat_Ogg::encoderArgs(const Profile &profile, const Track *track, const QString &outFile) const
{
    QStringList args;

    args << Settings::i()->programName(encoderProgramName());

    args << "--quiet";

    // Quality settings .........................................
    if (profile.value("UseQuality").toBool())
    {
        args << "-q" << profile.value("Quality").toString();
    }
    else
    {
        QString val = profile.value("NormBitrate").toString();
        if (!val.isEmpty())
            args << "-b" << val;

        val = profile.value("MinBitrate").toString();
        if (!val.isEmpty())
            args << "-m" << val;

        val = profile.value("MaxBitrate").toString();
        if (!val.isEmpty())
            args << "-M" << val;
    }

    // Tags .....................................................
    if (!track->artist().isEmpty())
        args << "--artist"  << track->artist();

    if (!track->album().isEmpty())
        args << "--album"   << track->album();

    if (!track->genre().isEmpty())
        args << "--genre"   << track->genre();

    if (!track->date().isEmpty())
        args << "--date"    << track->date();

    if (!track->title().isEmpty())
        args << "--title"   << track->title();

    if (!track->tag(TagId::AlbumArtist).isEmpty())
        args << "--comment" << QString("album_artist=%1").arg(track->tag(TagId::AlbumArtist));

    if (!track->comment().isEmpty())
        args << "--comment" << QString("COMMENT=%1").arg(track->comment());

    if (!track->diskId().isEmpty())
        args << "--comment" << QString("DISCID=%1").arg(track->diskId());


    args << "--tracknum" << QString("%1").arg(track->trackNum());
    args << "--comment" << QString("totaltracks=%1").arg(track->trackCount());
    args << "--comment" << QString("tracktotal=%1").arg(track->trackCount());

    args << "--comment" << QString("disc=%1").arg(track->diskNum());
    args << "--comment" << QString("discnumber=%1").arg(track->diskNum());
    args << "--comment" << QString("disctotal=%1").arg(track->diskCount());

    // Files ....................................................
    args << "-o" << outFile;
    args << "-";
    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Ogg::gainArgs(const QStringList &files, const GainType gainType) const
{
    QStringList args;
    args <<  args << Settings::i()->programName(gainProgramName());
    if (gainType ==  GainType::Album)
        args << "--album";

    args << files;

    return args;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Ogg::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality",       true);
    res.insert("Quality",          7);
    res.insert("MinBitrate",       "");
    res.insert("NormBitrate",      "");
    res.insert("MaxBitrate",       "");
    res.insert("ReplayGain",       gainTypeToString(GainType::Disable));
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Ogg::configPage(Profile *profile, QWidget *parent) const
{
    return new ConfigPage_Ogg(profile, parent);
}


/************************************************

 ************************************************/
ConfigPage_Ogg::ConfigPage_Ogg(Profile *profile, QWidget *parent):
    EncoderConfigPage(profile, parent)
{
    setupUi(this);


    setLossyToolTip(oggQualitySpin);
    oggQualitySlider->setToolTip(oggQualitySpin->toolTip());
    oggQualityLabel->setToolTip(oggQualitySpin->toolTip());

    connect(oggQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(oggQualitySliderChanged(int)));
    connect(oggQualitySpin, SIGNAL(valueChanged(double)), this, SLOT(oggQualitySpinChanged(double)));

    fillReplayGainComboBox(oggGainCbx);

    QList<int> bitrates;
    bitrates << 0 << 64 << 80 << 96 << 128 << 160 << 196 << 256 << 350;
    fillBitrateComboBox(oggMinBitrateCbx,  bitrates);
    fillBitrateComboBox(oggNormBitrateCbx, bitrates);
    fillBitrateComboBox(oggMaxBitrateCbx,  bitrates);
}


/************************************************

 ************************************************/
void ConfigPage_Ogg::load()
{
    loadWidget("UseQuality",  oggUseQualityCheck);
    loadWidget("Quality",     oggQualitySpin);
    loadWidget("MinBitrate",  oggMinBitrateCbx);
    loadWidget("NormBitrate", oggNormBitrateCbx);
    loadWidget("MaxBitrate",  oggMaxBitrateCbx);
    loadWidget("ReplayGain",  oggGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Ogg::save()
{
    saveWidget("UseQuality",  oggUseQualityCheck);
    saveWidget("Quality",     oggQualitySpin);
    saveWidget("MinBitrate",  oggMinBitrateCbx);
    saveWidget("NormBitrate", oggNormBitrateCbx);
    saveWidget("MaxBitrate",  oggMaxBitrateCbx);
    saveWidget("ReplayGain",  oggGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Ogg::oggQualitySliderChanged(int value)
{
    oggQualitySpin->setValue(value / 10.0);
}


/************************************************

 ************************************************/
void ConfigPage_Ogg::oggQualitySpinChanged(double value)
{
    oggQualitySlider->setValue(value * 10);

    //            -1  0   1   2   3    4    5    6    7    8    9    10   fake
    int kbps[] = {45, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500, 500};
    int x1 = int(value);
    double y1= kbps[x1 + 1];
    double y2= kbps[x1 + 2];
    int bitrate = round((y2 - y1) * (value - x1) + y1);
    oggQualityLabel->setText(QString("(~%1 kbps)").arg(bitrate));
}
