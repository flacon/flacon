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

#ifndef CONVERTERTYPES_H
#define CONVERTERTYPES_H

#include <QtGlobal>
#include "../types.h"
#include "../track.h"
#include "../inputaudiofile.h"

class Track;
class OutFormat;
class Profile;

namespace Conv {

using TrackId = quint64;

class ConvTrack : public Track
{
public:
    ConvTrack()                       = default;
    ConvTrack(const ConvTrack &other) = default;
    ConvTrack(const Track &other);

    ConvTrack &operator=(const ConvTrack &other) = default;

    TrackId    id() const { return mId; }
    bool       isEnabled() const { return mEnabled; }
    bool       isPregap() const { return mPregap; }
    CueIndex   start() const { return mStart; }
    CueIndex   end() const { return mEnd; }
    bool       isNull() const { return mId == 0; }
    TrackState state() const { return mState; }

    void setId(TrackId value) { mId = value; }
    void setEnabled(bool value) { mEnabled = value; }
    void setPregap(bool value) { mPregap = value; }
    void setStart(const CueIndex &value) { mStart = value; }
    void setEnd(const CueIndex &value) { mEnd = value; }
    void setState(TrackState value) { mState = value; }

private:
    TrackId    mId    = 0;
    TrackState mState = TrackState::NotRunning;
    CueIndex   mStart;
    CueIndex   mEnd;
    bool       mEnabled = true;
    bool       mPregap  = false;
};

using ConvTracks = QList<ConvTrack>;

class EncoderOptions
{
public:
    EncoderOptions() = default;
    EncoderOptions(const OutFormat *outFormat, const Profile *profile);
    EncoderOptions(const EncoderOptions &other) = default;
    EncoderOptions &operator=(const EncoderOptions &other) = default;

    QString     formatId() const;
    QStringList encoderArgs(const ConvTrack &track, const QString &coverFile, const QString &outFile) const;

    int bitsPerSample(const InputAudioFile &audio) const;
    int sampleRate(const InputAudioFile &audio) const;

private:
    const OutFormat *mOutFormat = nullptr;
    const Profile *  mProfile   = nullptr;
};

class GainOptions
{
public:
    GainOptions() = default;
    GainOptions(const OutFormat *outFormat, const Profile *profile);
    GainOptions(const GainOptions &other) = default;
    GainOptions &operator=(const GainOptions &other) = default;

    GainType    type() const { return mType; }
    QStringList gainArgs(const QStringList &files) const;

private:
    const OutFormat *mOutFormat = nullptr;
    GainType         mType      = GainType::Disable;
};

class CoverOptions
{
public:
    CoverOptions() = default;
    CoverOptions(const QString &fileName, int size);
    CoverOptions(const CoverOptions &other) = default;
    CoverOptions &operator=(const CoverOptions &other) = default;

    QString fileName() const { return mFileName; }
    int     size() const { return mSize; }

private:
    QString mFileName;
    int     mSize = 0;
};

} // namespace

Q_DECLARE_METATYPE(Conv::ConvTrack)

inline uint qHash(const Conv::ConvTrack &track, uint seed = 0)
{
    return qHash(track.id(), seed);
}

#endif // CONVERTERTYPES_H
