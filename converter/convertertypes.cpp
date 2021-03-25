/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#include "convertertypes.h"
#include "../formats/outformat.h"
#include "../profiles.h"

using namespace Conv;

/************************************************
 *
 ************************************************/
ConvTrack::ConvTrack(const Track &other) :
    Track(other)
{
}

/************************************************
 *
 ************************************************/
EncoderFormat::EncoderFormat(const OutFormat *outFormat, const Profile *profile) :
    mOutFormat(outFormat),
    mProfile(profile)
{
}

/************************************************
 *
 ************************************************/
QString EncoderFormat::formatId() const
{
    return mOutFormat->id();
}

/************************************************
 *
 ************************************************/
QStringList EncoderFormat::encoderArgs(const ConvTrack &track, const QString &outFile) const
{
    return mOutFormat->encoderArgs(*mProfile, &track, outFile);
}

/************************************************
 *
 ************************************************/
QStringList EncoderFormat::gainArgs(const QStringList &files) const
{
    return mOutFormat->gainArgs(files, mProfile->gainType());
}

/************************************************
 *
 ************************************************/
int EncoderFormat::calcBitsPerSample(const InputAudioFile &audio) const
{
    return mOutFormat->calcBitsPerSample(audio, mOutFormat->maxBitPerSample());
}

/************************************************
 *
 ************************************************/
int EncoderFormat::calcSampleRate(const InputAudioFile &audio) const
{
    return mOutFormat->calcSampleRate(audio, mOutFormat->maxSampleRate());
}
