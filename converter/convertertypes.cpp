#include "convertertypes.h"
#include "../formats/outformat.h"
#include "../profiles.h"

using namespace Conv;

static void registerMetaTypes()
{
    static bool registred = false;
    if (!registred) {
        qRegisterMetaType<Conv::ConvTrack>();
        registred = true;
    }
}

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
