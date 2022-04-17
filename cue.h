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

class Cue
{
public:
    static constexpr const char *const EMBEDED_PREFIX = "embedded://";

    class CueIndex : public ::CueIndex
    {
    public:
        explicit CueIndex(const QString &str = "", const QByteArray &file = {});

        QByteArray file() const { return mFile; }

    private:
        QByteArray mFile;
    };

    class Track : public ::TrackTags
    {
        friend class Cue;

    public:
        using ::TrackTags::TrackTags;

        CueIndex cueIndex00() const { return mCueIndex00; }
        CueIndex cueIndex01() const { return mCueIndex01; }

    private:
        CueIndex mCueIndex00;
        CueIndex mCueIndex01;
    };

    using Tracks = QList<Track>;

public:
    Cue();
    explicit Cue(QIODevice *device, const QString &audioFile) noexcept(false);
    explicit Cue(const QString &fileName) noexcept(false);

    QString title() const { return mTitle; }
    QString filePath() const { return mFilePath; }
    DiscNum discCount() const { return mDiscCount; }
    DiscNum discNum() const { return mDiscNum; }

    bool isEmpty() const { return mTracks.isEmpty(); }
    bool isMutiplyAudio() const;
    bool isEmbedded() const { return mFilePath.startsWith(EMBEDED_PREFIX); }

    const Tracks &tracks() const { return mTracks; }
    const Track  &track(uint index) const { return mTracks.at(index); }

private:
    Tracks  mTracks;
    QString mFilePath;
    DiscNum mDiscCount = 0;
    DiscNum mDiscNum   = 0;
    QString mTitle;

    void       read(const CueData &data);
    QByteArray getAlbumPerformer(const CueData &data);
    void       splitTitleTag(const CueData &data);
    void       setCodecName(const CueData &data);
    void       validate();
};

class CueError : public FlaconError
{
public:
    explicit CueError(const QString &msg) :
        FlaconError(msg) { }

    virtual ~CueError();
};

#endif // CUE_H
