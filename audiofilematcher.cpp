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
#include "formats_in/informat.h"
#include <QDir>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "AudioFileMatcher")
}

AudioFileMatcher::AudioFileMatcher(const QString &cueFilePath, const Cue::Tracks &tracks) :
    mCueFilePath(cueFilePath),
    mTracks(tracks)
{
    fillFileTags();

    qCDebug(LOG) << "mFileTags =" << mFileTags;

    QDir        dir  = QFileInfo(mCueFilePath).dir();
    QStringList exts = InputFormat::allFileExts();
    mAllAudioFiles   = dir.entryInfoList(exts, QDir::Files | QDir::Readable);

    for (const auto &fi : qAsConst(mAllAudioFiles)) {
        qDebug(LOG) << "mAllAudioFiles: " << fi.filePath();
    }

    if (mAllAudioFiles.isEmpty() || mTracks.isEmpty()) {
        return;
    }

    // Trivial, but frequent case. Directory contains only one audio file.
    if (mFileTags.count() == 1 && mAllAudioFiles.count() == 1) {
        mResult[mFileTags.first()] << mAllAudioFiles.first().filePath();
        qCDebug(LOG) << "Return trivial:" << mResult;
        return;
    }

    // Looks like this is a per-track album .....
    if (mFileTags.count() == mAllAudioFiles.count()) {
        for (const Cue::Track &track : qAsConst(mTracks)) {
            mResult[track.tag(TagId::File)] = matchAudioFilesByTrack(track);
        }
        qCDebug(LOG) << "Return per-track album:" << mResult;
        return;
    }

    // Common search ............................
    for (const QString &fileTag : qAsConst(mFileTags)) {
        mResult[fileTag] = matchAudioFiles(fileTag);
    }
    qCDebug(LOG) << "Return common:" << mResult;
}

QStringList AudioFileMatcher::audioFiles(int index) const
{
    QString tag = mFileTags[index];
    return audioFiles(tag);
}

bool AudioFileMatcher::containsAudioFile(const QString &audioFile) const
{
    for (auto const &files : qAsConst(mResult)) {
        if (files.contains(audioFile)) {
            return true;
        }
    }

    return false;
}

void AudioFileMatcher::fillFileTags()
{
    QString prev;
    for (const TrackTags &track : qAsConst(mTracks)) {
        if (track.tag(TagId::File) != prev) {
            prev = track.tag(TagId::File);
            mFileTags << track.tag(TagId::File);
        }
    }
}

QStringList AudioFileMatcher::matchAudioFilesByTrack(const Cue::Track &track)
{
    auto scan = [this](const QString &pattern) -> QString {
        QRegExp re(QString(pattern), Qt::CaseInsensitive, QRegExp::RegExp2);

        foreach (const QFileInfo &audio, mAllAudioFiles) {
            if (re.exactMatch(audio.fileName())) {
                return audio.filePath();
            }
        }
        return "";
    };

    {
        QString res = scan(QRegExp::escape(QFileInfo(track.tag(TagId::File)).completeBaseName()));
        if (!res.isEmpty()) {
            return QStringList(res);
        }
    }

    if (!track.title().isEmpty()) {
        QString res = scan(QString(".*%1.*%2.*").arg(track.trackNum(), 2, 10, QChar('0')).arg(QRegExp::escape(track.title())));
        if (!res.isEmpty()) {
            return QStringList(res);
        }

        res = scan(QString(".*%1.*").arg(QRegExp::escape(track.title())));
        if (!res.isEmpty()) {
            return QStringList(res);
        }
    }

    {
        QString res = scan(QString(".*%1.*").arg(track.trackNum(), 2, 10, QChar('0')));
        if (!res.isEmpty()) {
            return QStringList(res);
        }
    }

    {
        QStringList res;
        res = matchAudioFiles(track.tag(TagId::File));

        res.removeDuplicates();
        return res;
    }
}

QStringList AudioFileMatcher::matchAudioFiles(const QString &fileTag)
{
    QStringList res;

    QStringList patterns;
    if (mFileTags.count() == 1) {
        patterns << QRegExp::escape(QFileInfo(mFileTags.first()).completeBaseName());
        patterns << QRegExp::escape(QFileInfo(mCueFilePath).completeBaseName()) + ".*";
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

    qCDebug(LOG) << "matchAudioFiles: patterns=" << patterns;
    QString audioExt;
    foreach (const InputFormat *format, InputFormat::allFormats()) {
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format->ext();
    }

    foreach (const QString &pattern, patterns) {
        QRegExp re(QString("%1(%2)+").arg(pattern).arg(audioExt), Qt::CaseInsensitive, QRegExp::RegExp2);
        foreach (const QFileInfo &audio, mAllAudioFiles) {
            qCDebug(LOG) << "matchAudioFiles test: re=" << re << "file=" << audio.filePath();
            if (re.exactMatch(audio.fileName())) {
                res << audio.filePath();
                qCDebug(LOG) << "matchAudioFiles test: MATCH";
            }
        }
    }

    res.removeDuplicates();
    return res;
}
