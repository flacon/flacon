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


#include "ogg.h"
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
    mSettingsGroup = "Ogg";
}


/************************************************

 ************************************************/
QStringList OutFormat_Ogg::encoderArgs(const Track *track, const QString &outFile) const
{
    QStringList args;

    args << settings->programName(encoderProgramName());

    args << "--quiet";

    // Quality settings .........................................
    if (settings->value("Ogg/UseQuality").toBool())
    {
        args << "-q" << settings->value("Ogg/Quality").toString();
    }
    else
    {
        QString val = settings->value("Ogg/NormBitrate").toString();
        if (!val.isEmpty())
            args << "-b" << val;

        val = settings->value("Ogg/MinBitrate").toString();
        if (!val.isEmpty())
            args << "-m" << val;

        val = settings->value("Ogg/MaxBitrate").toString();
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
    args << "--comment" << QString("TOTALTRACKS=%1").arg(track->trackCount());
    args << "--comment" << QString("TRACKTOTAL=%1").arg(track->trackCount());

    // Files ....................................................
    args << "-o" << outFile;
    args << "-";
    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Ogg::gainArgs(const QStringList &files) const
{
    QStringList args;
    args <<  args << settings->programName(gainProgramName());
    if (strToGainType(settings->value("Ogg/ReplayGain").toString()) ==  GainType::Album)
        args << "--album";

    args << files;

    return args;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Ogg::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Ogg/UseQuality",       true);
    res.insert("Ogg/Quality",          7);
    res.insert("Ogg/MinBitrate",       "");
    res.insert("Ogg/NormBitrate",      "");
    res.insert("Ogg/MaxBitrate",       "");
    res.insert("Ogg/ReplayGain",       gainTypeToString(GainType::Disable));
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Ogg::configPage(QWidget *parent) const
{
    return new ConfigPage_Ogg(parent);
}


/************************************************

 ************************************************/
ConfigPage_Ogg::ConfigPage_Ogg(QWidget *parent):
    EncoderConfigPage(parent)
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
    loadWidget("Ogg/UseQuality",  oggUseQualityCheck);
    loadWidget("Ogg/Quality",     oggQualitySpin);
    loadWidget("Ogg/MinBitrate",  oggMinBitrateCbx);
    loadWidget("Ogg/NormBitrate", oggNormBitrateCbx);
    loadWidget("Ogg/MaxBitrate",  oggMaxBitrateCbx);
    loadWidget("Ogg/ReplayGain",  oggGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Ogg::write()
{
    writeWidget("Ogg/UseQuality",  oggUseQualityCheck);
    writeWidget("Ogg/Quality",     oggQualitySpin);
    writeWidget("Ogg/MinBitrate",  oggMinBitrateCbx);
    writeWidget("Ogg/NormBitrate", oggNormBitrateCbx);
    writeWidget("Ogg/MaxBitrate",  oggMaxBitrateCbx);
    writeWidget("Ogg/ReplayGain",  oggGainCbx);
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
