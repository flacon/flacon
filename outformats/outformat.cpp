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


#include "outformat.h"
#include "settings.h"
#include "encoder.h"
#include "gain.h"

#include "wav.h"
#include "flac.h"
#include "aac.h"
#include "mp3.h"
#include "ogg.h"
#include "wv.h"

#include <QDebug>


/************************************************

 ************************************************/
void initOutFormats(QList<OutFormat*> *formats)
{
    *formats << new OutFormat_Wav();
    *formats << new OutFormat_Flac();
    *formats << new OutFormat_Aac();
    *formats << new OutFormat_Mp3();
    *formats << new OutFormat_Ogg();
    *formats << new OutFormat_Wv();
}


/************************************************

 ************************************************/
QList<OutFormat *> OutFormat::allFormats()
{
    static QList<OutFormat*> res;
    if (!res.count())
        initOutFormats(&res);

    return res;
}


/************************************************

 ************************************************/
OutFormat *OutFormat::currentFormat()
{
    QString formatId = settings->value(Settings::OutFiles_Format).toString();
    foreach (OutFormat *format, allFormats())
    {
        if (format->id() == formatId)
            return format;
    }

    return allFormats().first();
}


/************************************************

 ************************************************/
OutFormat::GainType OutFormat::gainType() const
{
    QString s = settings->value(id() + "/ReplayGain").toString();
    return strToGainType(s);
}


/************************************************

 ************************************************/
OutFormat::PreGapType OutFormat::preGapType() const
{
    QString s = settings->value(Settings::PerTrackCue_Pregap).toString();
    return strToPreGapType(s);
}


/************************************************

 ************************************************/
bool OutFormat::createCue() const
{
    return settings->value(Settings::PerTrackCue_Create).toBool();
}


/************************************************

 ************************************************/
bool OutFormat::checkProgram(const QString &program, QStringList *errors) const
{
    if (program.isEmpty())
        return true;

    if (!settings->checkProgram(program))
    {
        *errors << QObject::tr("I can't find program <b>%1</b>.").arg(program);
        return false;
    }

    return true;
}



/************************************************

 ************************************************/
bool OutFormat::check(QStringList *errors) const
{
    bool res = checkProgram(encoderProgramName(), errors);

    if (gainType() != GainDisable)
        checkProgram(gainProgramName(), errors);

    return res;
}


/************************************************

 ************************************************/
QString OutFormat::gainTypeToString(OutFormat::GainType type)
{
    switch(type)
    {
    case GainDisable: return "Disable";
    case GainTrack:   return "Track";
    case GainAlbum:   return "Album";
    }

    return "Disable";
}


/************************************************

 ************************************************/
OutFormat::GainType OutFormat::strToGainType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "TRACK")   return GainTrack;
    if (s == "ALBUM")   return GainAlbum;

    return GainDisable;
}


/************************************************

 ************************************************/
QString OutFormat::preGapTypeToString(OutFormat::PreGapType type)
{
    switch(type)
    {
    case PreGapExtractToFile:   return "Extract";
    case PreGapAddToFirstTrack: return "AddToFirst";
    }

    return "Disable";
}


/************************************************

 ************************************************/
OutFormat::PreGapType OutFormat::strToPreGapType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "EXTRACT")     return PreGapExtractToFile;
    if (s == "ADDTOFIRST")  return PreGapAddToFirstTrack;

    return PreGapAddToFirstTrack;
}


/************************************************

 ************************************************/
Encoder *OutFormat::createEncoder(Track *track, QObject *parent) const
{
    return new Encoder(this, track, parent);
}


/************************************************

 ************************************************/
Gain *OutFormat::createGain(Disk *disk, Track *track, QObject *parent) const
{
    if (!gainProgramName().isEmpty())
        return new Gain(this, disk, track, parent);
    else
        return 0;
}





