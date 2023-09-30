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
#include "cue.h"
#include <QFileInfoList>
#include "inputaudiofile.h"

class QDebug;

class AudioFileMatcher
{
public:
    void matchForCue(const Cue &cue);
    void matchForAudio(const QString &audioFilePath);

    void clear();

    QStringList        fileTags() const { return mFileTags; }
    Cue                cue() const { return mCue; }
    QFileInfoList      audioFilePaths() const { return mAudioFilePaths; }
    InputAudioFileList audioFiles() const;

    QFileInfo
    audioFile(const QString &fileTag) const;

private:
    QStringList                mFileTags;
    Cue                        mCue;
    QFileInfoList              mAudioFilePaths;
    mutable InputAudioFileList mAudioFiles;

    QFileInfoList searchCueFiles(const QDir &dir);
    QFileInfoList searchAudioFiles(const QDir &dir);

    void doMatchForCue(const Cue &cue, const QFileInfoList &allAudioFiles);
    void doMatchForAudio(const QString &audioFilePath);

    QFileInfo matchSingleAudio(const QFileInfoList &allAudioFiles);

    QFileInfoList tryPerTrackMatch(const QFileInfoList &allAudioFiles) const;
    QFileInfoList tryMultiAudioPattrnMatch(const QFileInfoList &allAudioFiles) const;

    QFileInfo searchFile(const QRegExp &pattern, const QFileInfoList &allAudioFiles) const;
};

QDebug operator<<(QDebug &debug, const AudioFileMatcher &matcher);

#endif // AUDIOFILEMATCHER_H
