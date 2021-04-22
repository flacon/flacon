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
EncoderOptions::EncoderOptions(const OutFormat *outFormat, const Profile *profile) :
    mOutFormat(outFormat),
    mProfile(profile)
{
}

/************************************************
 *
 ************************************************/
QString EncoderOptions::formatId() const
{
    return mOutFormat->id();
}

/************************************************
 *
 ************************************************/
QStringList EncoderOptions::encoderArgs(const ConvTrack &track, const QString &outFile) const
{
    return mOutFormat->encoderArgs(*mProfile, &track, outFile);
}

/************************************************
 *
 ************************************************/
int EncoderOptions::bitsPerSample(const InputAudioFile &audio) const
{
    return calcQuality(audio.bitsPerSample(), mProfile->bitsPerSample(), mOutFormat->maxBitPerSample());
}

/************************************************
 *
 ************************************************/
int EncoderOptions::sampleRate(const InputAudioFile &audio) const
{
    return calcQuality(audio.sampleRate(), mProfile->sampleRate(), mOutFormat->maxSampleRate());
}

/************************************************
 *
 ************************************************/
GainOptions::GainOptions(const OutFormat *outFormat, const Profile *profile) :
    mOutFormat(outFormat),
    mType(profile->gainType())
{
}

/************************************************
 *
 ************************************************/
QStringList GainOptions::gainArgs(const QStringList &files) const
{
    return mOutFormat->gainArgs(files, type());
}

/************************************************
 *
 ************************************************/
CoverOptions::CoverOptions(const QString &fileName, int size) :
    mFileName(fileName),
    mSize(size)
{
}
