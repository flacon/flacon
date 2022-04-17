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

#ifndef AUDIOFILEMATCHER_H
#define AUDIOFILEMATCHER_H

#include <QString>
#include "track.h"
#include "cue.h"
#include <QFileInfoList>

class AudioFileMatcher
{
public:
    AudioFileMatcher(const QString &cueFilePath, const Cue::Tracks &tracks);

    const Cue::Tracks &tracks() const { return mTracks; }

    const QStringList &fileTags() const { return mFileTags; }
    QStringList        audioFiles(const QString &fileTag) const { return mResult[fileTag]; }
    QStringList        audioFiles(int index) const;

    bool containsAudioFile(const QString &audioFile) const;

private:
    QString                    mCueFilePath;
    Cue::Tracks                mTracks;
    QStringList                mFileTags;
    QFileInfoList              mAllAudioFiles;
    QMap<QString, QStringList> mResult;

    void        fillFileTags();
    QStringList matchAudioFilesByTrack(const QString &fileTag, const QString &trackTitle);
    QStringList matchAudioFiles(const QString &fileTag);
};

#endif // AUDIOFILEMATCHER_H
