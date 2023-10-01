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
#include "inputaudiofile.h"
#include <QDebug>

namespace {
Q_LOGGING_CATEGORY(LOG, "AudioFileMatcher")

/**************************************
 *
 **************************************/
void sortByLevenshteinDistance(QFileInfoList &files, const QFileInfo &pattern)
{
    QString name = pattern.completeBaseName();
    std::sort(files.begin(), files.end(), [name](const QFileInfo &f1, const QFileInfo &f2) {
        return levenshteinDistance(f1.completeBaseName(), name) < levenshteinDistance(f2.completeBaseName(), name);
    });
}

/**************************************
 *
 **************************************/
QStringList getFileTags(const Cue &cue)
{
    QStringList res;

    QString prev;
    for (const TrackTags &track : qAsConst(cue.tracks())) {
        if (track.tag(TagId::File) != prev) {
            prev = track.tag(TagId::File);
            res << track.tag(TagId::File);
        }
    }

    qCDebug(LOG) << "CUE contains " << res.count() << " FILE tags:";
    for (const auto &f : qAsConst(res)) {
        qCDebug(LOG) << "  *" << f;
    }

    return res;
}

}

/**************************************
 *
 **************************************/
void AudioFileMatcher::matchForCue(const Cue &cue)
{
    doMatchForCue(cue, searchAudioFiles(QFileInfo(cue.filePath()).dir()));
    qCDebug(LOG) << "Result: " << *this;
}

/**************************************
 *
 **************************************/
void AudioFileMatcher::doMatchForCue(const Cue &cue, const QFileInfoList &allAudioFiles)
{
    clear();
    mCue      = cue;
    mFileTags = getFileTags(cue);

    if (mFileTags.isEmpty()) {
        return;
    }

    if (mFileTags.count() == 1) {
        mAudioFilePaths << matchSingleAudio(allAudioFiles);
        return;
    }

    if (mAudioFilePaths.isEmpty()) {
        mAudioFilePaths = tryPerTrackMatch(allAudioFiles);
    }

    if (mAudioFilePaths.isEmpty()) {
        mAudioFilePaths = tryMultiAudioPattrnMatch(allAudioFiles);
    }
}

/**************************************
 *
 **************************************/
void AudioFileMatcher::matchForAudio(const QString &audioFilePath)
{
    doMatchForAudio(audioFilePath);
    qCDebug(LOG) << "Result: " << *this;
}

/**************************************
 *
 **************************************/

void AudioFileMatcher::doMatchForAudio(const QString &audioFilePath)
{
    clear();

    QFileInfo audioFile = QFileInfo(audioFilePath);

    // Embedded CUE ........................
    try {
        InputAudioFile audio(audioFile.filePath());
        if (audio.isValid()) {
            EmbeddedCue cue(audio);
            if (!cue.isEmpty()) {
                mCue      = cue;
                mFileTags = getFileTags(mCue);
                mAudioFilePaths << audioFile;
                return;
            }
        }
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "Can't parse embedded cue:" << err.what();
    }

    QFileInfoList allCueFiles   = searchCueFiles(audioFile.dir());
    QFileInfoList allAudioFiles = searchAudioFiles(audioFile.dir());

    mAudioFilePaths << audioFile;

    if (allAudioFiles.isEmpty() || allCueFiles.isEmpty()) {
        return;
    }

    // Trivial, but frequent case. Directory contains only one disk.
    if (allAudioFiles.count() == 1 && allCueFiles.count() == 1) {
        mCue            = Cue(allCueFiles.first().filePath());
        mFileTags       = getFileTags(mCue);
        mAudioFilePaths = allAudioFiles;
        return;
    }

    sortByLevenshteinDistance(allCueFiles, audioFile);

    AudioFileMatcher matcher;

    for (const QFileInfo &cueFile : allCueFiles) {
        Cue cue(cueFile.filePath());
        matcher.doMatchForCue(cue, allAudioFiles);
        if (matcher.audioFilePaths().contains(audioFile)) {
            mCue            = cue;
            mFileTags       = matcher.fileTags();
            mAudioFilePaths = matcher.audioFilePaths();
            return;
        }
    }
}

/**************************************
 *
 **************************************/
QFileInfo AudioFileMatcher::audioFile(const QString &fileTag) const
{
    int n = mFileTags.indexOf(fileTag);
    if (n > -1 && n < mAudioFilePaths.count()) {
        return mAudioFilePaths[n];
    }

    return {};
}

/**************************************
 *
 **************************************/
QFileInfoList AudioFileMatcher::searchCueFiles(const QDir &dir)
{
    QStringList   exts = QStringList("*.cue");
    QFileInfoList res  = dir.entryInfoList(exts, QDir::Files | QDir::Readable, QDir::SortFlag::Name);

    qCDebug(LOG) << "Directory contains " << res.count() << " cue files:";
    for (const auto &fi : qAsConst(res)) {
        qCDebug(LOG) << "  *" << fi.filePath();
    }

    return res;
}

/**************************************
 *
 **************************************/
QFileInfoList AudioFileMatcher::searchAudioFiles(const QDir &dir)
{
    QStringList   exts = InputFormat::allFileExts();
    QFileInfoList res  = dir.entryInfoList(exts, QDir::Files | QDir::Readable, QDir::SortFlag::Name);

    qCDebug(LOG) << "Directory contains " << res.count() << " audio files:";
    for (const auto &fi : qAsConst(res)) {
        qCDebug(LOG) << "  *" << fi.filePath();
    }

    return res;
}

/**************************************
 *
 **************************************/
void AudioFileMatcher::clear()
{
    mFileTags.clear();
    mCue = Cue();
    mAudioFilePaths.clear();
    mAudioFiles.clear();
}

/**************************************
 *
 **************************************/
InputAudioFileList AudioFileMatcher::audioFiles() const
{
    if (mAudioFiles.isEmpty()) {
        for (auto f : mAudioFilePaths) {
            mAudioFiles << InputAudioFile(f.filePath());
        }
    }

    return mAudioFiles;
}

/**************************************
 *
 **************************************/
QFileInfo AudioFileMatcher::matchSingleAudio(const QFileInfoList &allAudioFiles)
{
    qCDebug(LOG) << Q_FUNC_INFO;

    // Trivial, but frequent case. Directory contains only one audio file.
    if (allAudioFiles.count() == 1) {
        return allAudioFiles.first();
    }

    QStringList patterns;
    patterns << QRegExp::escape(QFileInfo(mFileTags.first()).completeBaseName());
    patterns << QRegExp::escape(QFileInfo(mCue.filePath()).completeBaseName()) + ".*";

    foreach (const QString &pattern, patterns) {

        QRegExp   re(pattern, Qt::CaseInsensitive, QRegExp::RegExp2);
        QFileInfo file = searchFile(re, allAudioFiles);
        if (!file.fileName().isEmpty()) {
            return file;
        }
    }

    return {};
}

/**************************************
 * Looks like this is a per-track album
 **************************************/
QFileInfoList AudioFileMatcher::tryPerTrackMatch(const QFileInfoList &allAudioFiles) const
{
    if (mFileTags.count() != mCue.tracks().count()) {
        return {};
    }
    qCDebug(LOG) << Q_FUNC_INFO;

    if (allAudioFiles.count() == mCue.tracks().count()) {
        return allAudioFiles;
    }

    // HTOA
    if (allAudioFiles.count() == mCue.tracks().count() + 1) {
        InputAudioFile htoa(allAudioFiles.first().filePath());
        if (htoa.duration() < 10 * 1000) {
            return allAudioFiles.mid(1);
        }
    }

    return {};
}

/**************************************
 *
 **************************************/
QFileInfoList AudioFileMatcher::tryMultiAudioPattrnMatch(const QFileInfoList &allAudioFiles) const
{
    qCDebug(LOG) << Q_FUNC_INFO;

    QStringList patterns;
    patterns << "{FILE_TAG}";
    patterns << "{FILE_TAG}.*";
    patterns << ".*{FAILE_TAG_NUM}.*{FILE_TAG}.*";

    patterns << "{CUE_FILE_NAME}0*{FAILE_TAG_NUM}";
    patterns << "{CUE_FILE_NAME}0*{FAILE_TAG_NUM}\\D.*";
    patterns << "{CUE_FILE_NAME}.*\\D0*{FAILE_TAG_NUM}";
    patterns << "{CUE_FILE_NAME}.*\\D0*{FAILE_TAG_NUM}\\D.*";

    for (const QString prefix : { "disk", "disc", "side" }) {

        patterns << ".*" + prefix + "0*{FAILE_TAG_NUM}";
        patterns << ".*" + prefix + "0*{FAILE_TAG_NUM}\\D.*";
        patterns << ".*" + prefix + ".*\\D0*{FAILE_TAG_NUM}";
        patterns << ".*" + prefix + "0.*\\D0*{FAILE_TAG_NUM}\\D.*";
    }

    for (const QString prefix : { "disk", "disc", "side" }) {
        patterns << ".*" + prefix + "{FAILE_TAG_LETTER}";
        patterns << ".*" + prefix + ".*[\\[\\(]{FAILE_TAG_LETTER}[\\]\\)]";
    }

    const QString &cueFileName = QRegExp::escape(QFileInfo(mCue.filePath()).completeBaseName());

    QFileInfoList res;
    for (const QString &pattern : qAsConst(patterns)) {

        for (int i = 0; i < mFileTags.count(); ++i) {
            const QString &fileTag    = QRegExp::escape(QFileInfo(mFileTags.at(i)).completeBaseName());
            const int      fileTagNum = i;

            QString s = pattern;
            s         = s.replace("{FAILE_TAG_NUM}", QString::number(fileTagNum + 1));
            s         = s.replace("{FAILE_TAG_LETTER}", QChar('a' + (fileTagNum % ('z' - 'a'))));
            s         = s.replace("{FILE_TAG}", fileTag);
            s         = s.replace("{CUE_FILE_NAME}", cueFileName);

            QRegExp re = QRegExp(s, Qt::CaseInsensitive, QRegExp::RegExp2);

            QFileInfo file = searchFile(re, allAudioFiles);
            if (file.fileName().isEmpty()) {
                res.clear();
                break;
            }

            res << file;
        }

        if (res.count() == mFileTags.count()) {
            return res;
        }
    }

    return {};
}

QFileInfo AudioFileMatcher::searchFile(const QRegExp &pattern, const QFileInfoList &allAudioFiles) const
{
    qCDebug(LOG) << Q_FUNC_INFO << pattern;

    foreach (const QFileInfo &audio, allAudioFiles) {
        if (pattern.exactMatch(audio.completeBaseName())) {
            qCDebug(LOG) << "  - test: re=" << pattern << "file=" << audio.filePath() << " OK";
            return audio;
        }
        qCDebug(LOG) << "  - test: re=" << pattern << "file=" << audio.filePath() << " FAIL";
    }

    return {};
}

QDebug operator<<(QDebug &debug, const AudioFileMatcher &matcher)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << " {"
                    << " cue: " << (matcher.cue().isEmpty() ? "none" : matcher.cue().filePath())
                    << " audio:" << matcher.audioFilePaths()
                    << "}";
    return debug;
}
