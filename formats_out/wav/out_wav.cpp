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

#include "out_wav.h"

/************************************************

 ************************************************/
OutFormat_Wav::OutFormat_Wav()
{
    mId      = "WAV";
    mExt     = "wav";
    mName    = "WAV";
    mOptions = FormatOption::Lossless;
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
    return new ConfigPage_Wav(parent);
}

/************************************************

 ************************************************/
Conv::Encoder *OutFormat_Wav::createEncoder() const
{
    return new Encoder_Wav();
}

/************************************************

 ************************************************/
ConfigPage_Wav::ConfigPage_Wav(QWidget *parent) :
    EncoderConfigPage(parent)
{
}

/************************************************
 *
 ************************************************/
QStringList Encoder_Wav::programArgs() const
{
    return QStringList();
}
