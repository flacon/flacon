/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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


#include "wav.h"
#include <QFile>


/************************************************

 ************************************************/
OutFormat_Wav::OutFormat_Wav()
{
    mId   = "WAV";
    mExt  = "wav";
    mName = "WAV";
}


/************************************************

 ************************************************/
QStringList OutFormat_Wav::encoderArgs(Track *track, const QString &outFile) const
{
    return QStringList();
}


/************************************************

 ************************************************/
QStringList OutFormat_Wav::gainArgs(const QStringList &files) const
{
    return QStringList();
}


/************************************************

 ************************************************/
Encoder *OutFormat_Wav::createEncoder(Track *track, QObject *parent) const
{
    return new Encoder_Wav(this, track, parent);
}


/************************************************

 ************************************************/
Gain *OutFormat_Wav::createGain(Disk *disk, Track *track, QObject *parent) const
{
    return 0;
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Wav::defaultParameters() const
{
    QHash<QString, QVariant> res;
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Wav::configPage(QWidget *parent) const
{
    return 0;
}


/************************************************

 ************************************************/
Encoder_Wav::Encoder_Wav(const OutFormat *format, Track *track, QObject *parent):
    Encoder(format, track, parent)
{
}


/************************************************

 ************************************************/
void Encoder_Wav::doRun()
{
    QFile srcFile(inputFile());
    QFile destFile(outFile());

    bool res = (!destFile.exists() || destFile.remove());

    if (res)
        res =  srcFile.rename(outFile());

    if (!res)
    {
        error(track(),
              tr("Can't rename file:\n%1 to %2\n%3").arg(
                  inputFile(),
                  outFile(),
                  srcFile.errorString()));
    }
}


/************************************************

 ************************************************/
QStringList Encoder_Wav::processArgs() const
{
    return QStringList();
}


