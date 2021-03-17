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

#include "audiofilematcher.h"
#include "formats/informat.h"
#include <QDir>

AudioFileMatcher::AudioFileMatcher(const QString &cueFilePath, const Tracks &tracks) :
    mCueFilePath(cueFilePath),
    mTracks(tracks)
{
    fillFileTags();

    QDir        dir  = QFileInfo(mCueFilePath).dir();
    QStringList exts = InputFormat::allFileExts();
    mAudioFiles      = dir.entryInfoList(exts, QDir::Files | QDir::Readable);

    QMap<QString, QString> res = run();
    mResult.reserve(mFileTags.count());
    for (const QString &tag : mFileTags) {
        mResult << res[tag];
    }
}

void AudioFileMatcher::fillFileTags()
{
    QString prev;
    for (const Track &track : mTracks) {
        if (track.tag(TagId::File) != prev) {
            prev = track.tag(TagId::File);
            mFileTags << track.tag(TagId::File);
        }
    }
}

QMap<QString, QString> AudioFileMatcher::run()
{
    QMap<QString, QString> res;
    if (mAudioFiles.isEmpty() || mTracks.isEmpty()) {
        return res;
    }

    // Trivial, but frequent case. Directory contains only one audio file.
    if (mFileTags.count() == 1 && mAudioFiles.count() == 1) {
        res[mFileTags.first()] = mAudioFiles.first().filePath();
        return res;
    }

    // Looks like this is a per-track album .....
    if (mFileTags.count() == mAudioFiles.count()) {
        for (const Track &track : mTracks) {
            const QFileInfoList files = matchAudioFilesByTrack(track.tag(TagId::File), track.tag(TagId::Title));
            if (!files.isEmpty()) {
                res[track.tag(TagId::File)] = files.first().filePath();
            }
        }
        return res;
    }

    // Common search ............................
    for (const QString &fileTag : mFileTags) {
        QFileInfoList files = matchAudioFiles(fileTag);

        if (!files.isEmpty()) {
            res[fileTag] = files.first().filePath();
        }
    }
    return res;
}

QFileInfoList AudioFileMatcher::matchAudioFilesByTrack(const QString &fileTag, const QString &trackTitle)
{
    QFileInfoList res;
    QStringList   patterns;

    patterns << QRegExp::escape(QFileInfo(fileTag).completeBaseName());
    patterns << QString(".*%1.*").arg(QRegExp::escape(trackTitle));

    QString audioExt;
    foreach (const InputFormat *format, InputFormat::allFormats()) {
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format->ext();
    }

    foreach (const QString &pattern, patterns) {
        QRegExp re(QString("%1(%2)+").arg(pattern).arg(audioExt), Qt::CaseInsensitive, QRegExp::RegExp2);

        foreach (const QFileInfo &audio, mAudioFiles) {
            if (re.exactMatch(audio.fileName())) {
                res << audio;
            }
        }
    }

    if (res.isEmpty()) {
        res = matchAudioFiles(fileTag);
    }
    return res;
}

QFileInfoList AudioFileMatcher::matchAudioFiles(const QString &fileTag)
{
    QFileInfoList res;

    QStringList patterns;
    if (mFileTags.count() == 1) {
        patterns << QRegExp::escape(QFileInfo(mFileTags.first()).completeBaseName());
        patterns << QRegExp::escape(mCueFilePath) + ".*";
    }
    else {
        int fileTagNum = mFileTags.indexOf(fileTag) + 1; // Disks are indexed from 1, not from 0!
        patterns << QRegExp::escape(QFileInfo(fileTag).completeBaseName());
        patterns << QRegExp::escape(QFileInfo(mCueFilePath).completeBaseName()) + QString("(.*\\D)?"
                                                                                          "0*"
                                                                                          "%1"
                                                                                          "(.*\\D)?")
                                                                                          .arg(fileTagNum);
        patterns << QString(".*"
                            "(disk|disc|side)"
                            "(.*\\D)?"
                            "0*"
                            "%1"
                            "(.*\\D)?")
                            .arg(fileTagNum);
    }

    QString audioExt;
    foreach (const InputFormat *format, InputFormat::allFormats()) {
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format->ext();
    }

    foreach (const QString &pattern, patterns) {
        QRegExp re(QString("%1(%2)+").arg(pattern).arg(audioExt), Qt::CaseInsensitive, QRegExp::RegExp2);
        foreach (const QFileInfo &audio, mAudioFiles) {
            if (re.exactMatch(audio.fileName())) {
                res << audio;
            }
        }
    }

    return res;
}
