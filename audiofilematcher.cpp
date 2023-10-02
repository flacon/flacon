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
void sortFileNameByLevenshteinDistance(QFileInfoList &files, const QFileInfo &pattern)
{
    QString name = pattern.fileName();
    std::sort(files.begin(), files.end(), [name](const QFileInfo &f1, const QFileInfo &f2) {
        return levenshteinDistance(f1.fileName(), name) < levenshteinDistance(f2.fileName(), name);
    });
}

}

/**************************************
 *
 **************************************/
void AudioFileMatcher::matchForCue(const Cue &cue)
{
    clear();
    Result res      = doMatchForCue(cue, searchAudioFiles(QFileInfo(cue.filePath()).dir()));
    mCue            = res.cue;
    mAudioFilePaths = res.audioFilePaths;

    qCDebug(LOG) << "Result: " << *this;
}

/**************************************
 *
 **************************************/
AudioFileMatcher::Result AudioFileMatcher::doMatchForCue(const Cue &cue, const QFileInfoList &allAudioFiles) const
{
    Result res;
    res.cue = cue;

    if (cue.fileTags().isEmpty()) {
        return res;
    }

    if (cue.fileTags().count() == 1) {
        res.audioFilePaths << matchSingleAudio(cue, allAudioFiles);
        return res;
    }

    if (res.audioFilePaths.isEmpty()) {
        res.audioFilePaths = tryPerTrackMatch(cue, allAudioFiles);
    }

    if (res.audioFilePaths.isEmpty()) {
        res.audioFilePaths = tryMultiAudioPattrnMatch(cue, allAudioFiles);
    }

    return res;
}

/**************************************
 *
 **************************************/
void AudioFileMatcher::matchForAudio(const QString &audioFilePath)
{
    clear();
    Result res      = doMatchForAudio(audioFilePath);
    mCue            = res.cue;
    mAudioFilePaths = res.audioFilePaths;

    qCDebug(LOG) << "Result: " << *this;
}

/**************************************
 *
 **************************************/

AudioFileMatcher::Result AudioFileMatcher::doMatchForAudio(const QString &audioFilePath) const
{
    QFileInfo audioFile = QFileInfo(audioFilePath);

    // Embedded CUE ........................
    try {
        InputAudioFile audio(audioFile.filePath());
        if (audio.isValid()) {
            EmbeddedCue cue(audio);
            if (!cue.isEmpty()) {
                Result res;
                res.cue = cue;
                res.audioFilePaths << audioFile;
                return res;
            }
        }
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "Can't parse embedded cue:" << err.what();
    }

    QFileInfoList allCueFiles   = searchCueFiles(audioFile.dir());
    QFileInfoList allAudioFiles = searchAudioFiles(audioFile.dir());

    if (allAudioFiles.isEmpty() || allCueFiles.isEmpty()) {
        Result res;
        res.audioFilePaths << audioFile;
        return res;
    }

    // Trivial, but frequent case. Directory contains only one disk.
    if (allAudioFiles.count() == 1 && allCueFiles.count() == 1) {
        Result res;
        res.cue            = Cue(allCueFiles.first().filePath());
        res.audioFilePaths = allAudioFiles;
        return res;
    }

    sortByLevenshteinDistance(allCueFiles, audioFile);

    for (const QFileInfo &cueFile : allCueFiles) {
        Cue cue(cueFile.filePath());

        if (cue.fileTags().count() == 1) {
            QFileInfoList audioFiles = tryMultiAudioPattrnMatch(cue, { audioFile });
            if (!audioFiles.isEmpty()) {
                Result res;
                res.cue            = cue;
                res.audioFilePaths = audioFiles;
                return res;
            }
        }

        sortFileNameByLevenshteinDistance(allAudioFiles, audioFile);
        QFileInfoList audioFiles = tryMultiAudioPattrnMatch(cue, allAudioFiles);

        if (audioFiles.contains(audioFile)) {
            Result res;
            res.cue            = cue;
            res.audioFilePaths = audioFiles;
            return res;
        }
    }

    return {};
}

/**************************************
 *
 **************************************/
// QFileInfo AudioFileMatcher::audioFile(const QString &fileTag) const
//{
//     int n = _mCue.fileTags().indexOf(fileTag);
//     if (n > -1 && n < _mAudioFilePaths.count()) {
//         return _mAudioFilePaths[n];
//     }

//    return {};
//}

/**************************************
 *
 **************************************/
QFileInfoList AudioFileMatcher::searchCueFiles(const QDir &dir) const
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
QFileInfoList AudioFileMatcher::searchAudioFiles(const QDir &dir) const
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
QFileInfo AudioFileMatcher::matchSingleAudio(const Cue &cue, const QFileInfoList &allAudioFiles) const
{
    qCDebug(LOG) << Q_FUNC_INFO;

    // Trivial, but frequent case. Directory contains only one audio file.
    if (allAudioFiles.count() == 1) {
        return allAudioFiles.first();
    }

    QStringList patterns;
    patterns << QRegExp::escape(QFileInfo(cue.fileTags().first()).completeBaseName());
    patterns << QRegExp::escape(QFileInfo(cue.filePath()).completeBaseName()) + ".*";

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
QFileInfoList AudioFileMatcher::tryPerTrackMatch(const Cue &cue, const QFileInfoList &allAudioFiles) const
{
    if (cue.fileTags().count() != cue.tracks().count()) {
        return {};
    }

    if (allAudioFiles.count() == cue.tracks().count()) {
        return allAudioFiles;
    }

    // HTOA
    if (allAudioFiles.count() == cue.tracks().count() + 1) {
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
QFileInfoList AudioFileMatcher::tryMultiAudioPattrnMatch(const Cue &cue, const QFileInfoList &allAudioFiles) const
{
    qCDebug(LOG) << Q_FUNC_INFO;

    QStringList patterns;
    patterns << "{FILE_TAG}";
    patterns << "{FILE_TAG}.*";
    patterns << ".*{FAILE_TAG_NUM}.*{FILE_TAG}.*";

    if (cue.fileTags().count() == 1) {
        patterns << "{CUE_FILE_NAME}";
    }

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

    const QString &cueFileName = QRegExp::escape(QFileInfo(cue.filePath()).completeBaseName());

    QFileInfoList res;
    for (const QString &pattern : qAsConst(patterns)) {

        for (int i = 0; i < cue.fileTags().count(); ++i) {
            const QString &fileTag    = QRegExp::escape(QFileInfo(cue.fileTags().at(i)).completeBaseName());
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

        if (res.count() == cue.fileTags().count()) {
            return res;
        }
    }

    return {};
}

/**************************************
 *
 **************************************/
QFileInfo AudioFileMatcher::searchFile(const QRegExp &pattern, const QFileInfoList &allFiles) const
{
    qCDebug(LOG) << Q_FUNC_INFO << pattern;

    foreach (const QFileInfo &f, allFiles) {
        if (pattern.exactMatch(f.completeBaseName())) {
            qCDebug(LOG) << "  - test: re=" << pattern << "file=" << f.filePath() << " OK";
            return f;
        }
        qCDebug(LOG) << "  - test: re=" << pattern << "file=" << f.filePath() << " FAIL";
    }

    return {};
}

/**************************************
 *
 **************************************/
QDebug operator<<(QDebug &debug, const AudioFileMatcher &matcher)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << " {"
                    << " cue: " << (matcher.cue().isEmpty() ? "none" : matcher.cue().filePath())
                    << " audio:" << matcher.audioFilePaths()
                    << "}";
    return debug;
}
