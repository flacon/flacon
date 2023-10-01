/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2015
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

#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QList>
#include "types.h"
#include "tags.h"

class CueData;
class InputAudioFile;

class Cue
{
public:
    static constexpr const char *const EMBEDED_PREFIX = "embedded://";

public:
    Cue() = default;
    explicit Cue(const QString &fileName) noexcept(false);

    QString title() const { return mTitle; }
    QString filePath() const { return mFilePath; }
    DiscNum discCount() const { return mDiscCount; }
    DiscNum discNum() const { return mDiscNum; }

    bool isEmpty() const { return mTracks.isEmpty(); }
    bool isMutiplyAudio() const;
    bool isEmbedded() const { return mFilePath.startsWith(EMBEDED_PREFIX); }

    const DiskTags  &tracks() const { return mTracks; }
    const TrackTags &track(uint index) const { return mTracks.at(index); }

protected:
    DiskTags mTracks;
    QString  mFilePath;
    DiscNum  mDiscCount = 0;
    DiscNum  mDiscNum   = 0;
    QString  mTitle;

    void       read(const CueData &data);
    QByteArray getAlbumPerformer(const CueData &data);
    void       splitTitleTag(const CueData &data);
    void       setCodecName(const CueData &data);
    void       validate();
};

class EmbeddedCue : public Cue
{
public:
    EmbeddedCue()                         = default;
    EmbeddedCue(const EmbeddedCue &other) = default;
    EmbeddedCue &operator=(const EmbeddedCue &other) = default;

    explicit EmbeddedCue(const InputAudioFile &audioFile) noexcept(false);
};

class CueError : public FlaconError
{
public:
    explicit CueError(const QString &msg) :
        FlaconError(msg) { }

    virtual ~CueError();
};

#endif // CUE_H
