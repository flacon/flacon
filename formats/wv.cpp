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


#include "wv.h"
#include "disk.h"
#include "settings.h"
#include <QDebug>

REGISTER_FORMAT(Format_Wv)

/************************************************
 * As I understand WavPack can be embedded as a chunk of a RIFF stream.
 * I have not such a file, if anyone has one, please send me.
 ************************************************/
bool Format_Wv::checkMagic(const QByteArray &data) const
{
    return data.contains(magic());
}


/************************************************
 *
 ************************************************/
QStringList Format_Wv::decoderArgs(const QString &fileName) const
{
    QStringList args;
    args << "-q";
    args << "-y";
    args << fileName;
    args << "-o" << "-";

    return args;
}


/************************************************

 ************************************************/
OutFormat_Wv::OutFormat_Wv()
{
    mId   = "WV";
    mExt  = "wv";
    mName = "WavPack";
}


/************************************************

 ************************************************/
QStringList OutFormat_Wv::encoderArgs(const Track *track, const QString &outFile) const
{
    QStringList args;

    args << settings->programName(encoderProgramName());

    args << "-q";            // Suppress progress indicator

    // Settings .................................................
    int compression = settings->value("WV/Compression").toInt();
    if (compression == 0)    args << "-f";
    if (compression == 1)    args << "-h";
    if (compression == 2)    args << "-hh";

    // Tags .....................................................
    if (!track->artist().isEmpty())  args << "-w" << QString("Artist=%1").arg(track->artist());
    if (!track->album().isEmpty())   args << "-w" << QString("Album=%1").arg(track->album());
    if (!track->genre().isEmpty())   args << "-w" << QString("Genre=%1").arg(track->genre());
    if (!track->date().isEmpty())    args << "-w" << QString("Year=%1").arg(track->date());
    if (!track->title().isEmpty())   args << "-w" << QString("Title=%1").arg(track->title());
    if (!track->disk()->discId().isEmpty())  args << "-w" << QString("DiscId=%1").arg(track->disk()->discId());
    if (!track->comment().isEmpty()) args << "-w" << QString("Comment=%1").arg(track->comment());
    args << "-w" << QString("Track=%1/%2").arg(track->trackNum()).arg(track->disk()->count());

    args << "-";
    args << "-o" << outFile;

    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Wv::gainArgs(const QStringList &files) const
{
    QStringList args;
    args <<  args << settings->programName(gainProgramName());
    args << "-a";
    args << files;

    return args;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Wv::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("WV/Compression",       1);
    res.insert("WV/ReplayGain",        gainTypeToString(GainType::Disable));
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Wv::configPage(QWidget *parent) const
{
    return new ConfigPage_Wv(parent);
}


/************************************************

 ************************************************/
ConfigPage_Wv::ConfigPage_Wv(QWidget *parent):
    EncoderConfigPage(parent)
{
    setupUi(this);

    setLosslessToolTip(wvCompressionSlider);
    wvCompressionSpin->setToolTip(wvCompressionSlider->toolTip());
    fillReplayGainComboBox(wvGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Wv::load()
{
    loadWidget("WV/Compression", wvCompressionSlider);
    loadWidget("WV/ReplayGain",  wvGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Wv::write()
{
    writeWidget("WV/Compression", wvCompressionSlider);
    writeWidget("WV/ReplayGain",  wvGainCbx);
}
