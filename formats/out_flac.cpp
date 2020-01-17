/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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


#include "out_flac.h"
#include "settings.h"
#include "project.h"
#include "inputaudiofile.h"

#include <QDebug>


/************************************************
 *
 ************************************************/
OutFormat_Flac::OutFormat_Flac()
{
    mId   = "FLAC";
    mExt  = "flac";
    mName = "Flac";
}



/************************************************

 ************************************************/
bool OutFormat_Flac::check(const Profile &profile, QStringList *errors) const
{
    bool res = OutFormat::check(profile, errors);

    if (profile.gainType() != GainType::Disable)
    {
        for (int i=0; i<project->count(); ++i)
        {
            if (project->disk(i)->audioFile()->sampleRate() > 48000)
            {
                *errors << QObject::tr("you can't use 'ReplayGain' for files with sample rates above 48kHz. Metaflac doesn't support such files.",
                                       "This string should begin with a lowercase letter. This is a part of the complex sentence.");
                res = false;
                break;
            }
        }
    }

    return res;
}


/************************************************

 ************************************************/
QStringList OutFormat_Flac::encoderArgs(const Track *track, const QString &outFile) const
{

    QStringList args;

    args << Settings::i()->programName(encoderProgramName());
    args << "--force";      // Force overwriting of output files.
    args << "--silent";     // Suppress progress indicator

    // Settings .................................................
    args << QString("--compression-level-%1").arg(Settings::i()->value("Flac/Compression").toString());


    // Tags .....................................................
    if (!track->artist().isEmpty())
        args << "--tag" << QString("artist=%1").arg(track->artist());

    if (!track->album().isEmpty())
        args << "--tag" << QString("album=%1").arg(track->album());

    if (!track->genre().isEmpty())
        args << "--tag" << QString("genre=%1").arg(track->genre());

    if (!track->date().isEmpty())
        args << "--tag" << QString("date=%1").arg(track->date());

    if (!track->title().isEmpty())
        args << "--tag" << QString("title=%1").arg(track->title());

    if (!track->tag(TagId::AlbumArtist).isEmpty())
        args << "--tag" << QString("albumartist=%1").arg(track->tag(TagId::AlbumArtist));

    if (!track->comment().isEmpty())
        args << "--tag" << QString("comment=%1").arg(track->comment());

    if (!track->diskId().isEmpty())
        args << "--tag" << QString("discId=%1").arg(track->diskId());

    args << "--tag" << QString("tracknumber=%1").arg(track->trackNum());
    args << "--tag" << QString("totaltracks=%1").arg(track->trackCount());
    args << "--tag" << QString("tracktotal=%1").arg(track->trackCount());

    args << "--tag" << QString("disc=%1").arg(track->diskNum());
    args << "--tag" << QString("discnumber=%1").arg(track->diskNum());
    args << "--tag" << QString("disctotal=%1").arg(track->diskCount());

    args << "-";
    args << "-o" << outFile;
    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Flac::gainArgs(const QStringList &files) const
{
    QStringList args;
    args << Settings::i()->programName(gainProgramName());
    args << "--add-replay-gain";
    args << files;

    return args;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Flac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Compression",  5);
    res.insert("ReplayGain",   gainTypeToString(GainType::Disable));
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Flac::configPage(Profile *profile, QWidget *parent) const
{
    return new ConfigPage_Flac(profile, parent);
}


/************************************************

 ************************************************/
ConfigPage_Flac::ConfigPage_Flac(Profile *profile, QWidget *parent):
    EncoderConfigPage(profile, parent)
{
    setupUi(this);

    setLosslessToolTip(flacCompressionSlider);
    flacCompressionSpin->setToolTip(flacCompressionSlider->toolTip());
    fillReplayGainComboBox(flacGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Flac::load()
{
    loadWidget("Compression",  flacCompressionSlider);
    loadWidget("ReplayGain", flacGainCbx);
}


/************************************************

 ************************************************/
void ConfigPage_Flac::save()
{
    saveWidget("Compression",  flacCompressionSlider);
    saveWidget("ReplayGain", flacGainCbx);
}
