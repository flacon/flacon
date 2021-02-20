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
#include <QFileInfoList>

class AudioFileMatcher
{
public:
    AudioFileMatcher(const QString &cueFilePath, const Tracks &tracks);

    const Tracks &tracks() const { return mTracks; }

    const QStringList &fileTags() const { return mFileTags; }
    const QStringList &result() const { return mResult; }

private:
    QString       mCueFilePath;
    Tracks        mTracks;
    QStringList   mFileTags;
    QFileInfoList mAudioFiles;
    QStringList   mResult;

    void                   fillFileTags();
    QMap<QString, QString> run();
    QFileInfoList          matchAudioFilesByTrack(const QString &fileTag, const QString &trackTitle);
    QFileInfoList          matchAudioFiles(const QString &fileTag);
};

#endif // AUDIOFILEMATCHER_H
