/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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

#include "disc.h"
#include "track.h"
#include "project.h"
#include "inputaudiofile.h"
#include "uchardetect.h"

#include "assert.h"
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QQueue>
#include <QtAlgorithms>
#include <QDebug>
#include <QBuffer>
#include "textcodec.h"

#define COVER_PREVIEW_SIZE 500

/**************************************
 *
 **************************************/
Disc::Disc(QObject *parent) :
    QObject(parent)
{
}

/**************************************
 *
 **************************************/
Disc::~Disc()
{
    qDeleteAll(mTracks);
}

/**************************************
 *
 **************************************/
void Disc::searchCoverImage(bool replaceExisting)
{
    if (!replaceExisting && !mCoverImageFile.isEmpty()) {
        return;
    }

    if (mCue.isEmpty()) {
        return;
    }

    // Search cover ...................
    QString dir        = QFileInfo(cueFilePath()).dir().absolutePath();
    mCoverImagePreview = QImage();
    mCoverImageFile    = searchCoverImage(dir);
}

/**************************************
 *
 **************************************/
QString Disc::cueFilePath() const
{
    return mCue.isEmpty() ? "" : mCue.filePath();
}

/**************************************
 *
 **************************************/
void Disc::setCue(const Cue &cue)
{
    mCue = cue;

    if (cue.isEmpty()) {
        return;
    }

    // Remove all tags if number of tracks differ from loaded CUE.
    for (int i = mInternetTags.size() - 1; i >= 0; i--) {
        if (mInternetTags.at(i).tracks().count() != cue.tracks().count()) {
            mInternetTags.removeAt(i);
            mInternetUserTags.removeAt(i);
        }
    }

    // Sync count of tracks
    for (int i = mTracks.count(); i < cue.tracks().count(); ++i) {
        Track *track = new Track(this, i);
        mTracks.append(track);
    }

    while (mTracks.count() > cue.tracks().count()) {
        delete mTracks.takeLast();
    }

    mUserTags.resize(mTracks.count());
    mLoadedTags.resize(mTracks.count());
    mCueUserTags.resize(mTracks.count());

    syncTagsFromTracks();

    mInternetTagsIndex = -1;

    InputAudioFileList audioFiles = this->audioFiles();

    for (int i = 0; i < qMin(audioFiles.count(), mTracks.count()); ++i) {
        if (!audioFiles[i].isNull()) {
            setAudioFile(InputAudioFile(audioFiles[i]), i);
        }
    }

    int n = -1;
    for (Track *track : mTracks) {
        n++;
        track->mCueIndex00 = mCue.tracks().at(n).cueIndex00();
        track->mCueIndex01 = mCue.tracks().at(n).cueIndex01();
    }

    if (!mTextCodec.isValid()) {
        setCodecName(CODEC_AUTODETECT);
    }

    syncTagsToTracks();

    Project::instance()->emitLayoutChanged();
}

/**************************************
 *
 **************************************/
int Disc::distance(const InternetTags &other)
{
    if (mTracks.isEmpty() || other.isEmpty()) {
        return std::numeric_limits<int>::max();
    }

    int res = 0;

    QString str1 = mTracks.first()->artistTag().toUpper().replace("THE ", "");
    QString str2 = other.artist().toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2) * 3;

    str1 = albumTag().toUpper().replace("THE ", "");
    str2 = other.album().toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2);

    return res;
}

/**************************************
 *
 **************************************/
void Disc::syncTagsFromTracks()
{
    int i = -1;
    for (Track *track : mTracks) {
        i++;
        mUserTags.tracks()[i] = track->userTags();
    }

    if (mInternetTagsIndex < 0) {
        mCueUserTags = mUserTags;
    }
    else {
        mInternetUserTags[mInternetTagsIndex] = mUserTags;
    }
}

/**************************************
 *
 **************************************/
void Disc::syncTagsToTracks()
{
    mUserTags   = mInternetTagsIndex < 0 ? mCueUserTags : mInternetTags[mInternetTagsIndex];
    mLoadedTags = mCue.decode(mTextCodec);

    if (mInternetTagsIndex > -1) {
        mLoadedTags.merge(mInternetTags.at(mInternetTagsIndex));
    }

    int i = -1;
    for (Track *track : mTracks) {
        i++;
        track->setUserTags(mUserTags.tracks().at(i));
        track->setLoadedTags(mLoadedTags.tracks().at(i));
    }
}

/**************************************
 *
 **************************************/
QList<TrackPtrList> Disc::tracksByFileTag() const
{
    QList<TrackPtrList> res;
    if (mTracks.isEmpty()) {
        return res;
    }

    int b = 0;
    while (b < tracks().count()) {
        int e = b;
        res.append(TrackPtrList());
        TrackPtrList &list = res.last();

        QByteArray prev = mCue.tracks().at(b).fileTag();

        for (; e < tracks().count() && mCue.tracks().at(e).fileTag() == prev; ++e) {
            list << mTracks[e];
        }

        b = e;
    }

    return res;
}

/**************************************
 *
 **************************************/
InputAudioFileList Disc::audioFiles() const
{
    InputAudioFileList res;

    if (mTracks.isEmpty() && !mAudioFile.isNull()) {
        res << mAudioFile;
        return res;
    }

    for (const TrackPtrList &l : tracksByFileTag()) {
        res << l.first()->audioFile();
    }

    return res;
}

/**************************************
 *
 **************************************/
void Disc::setAudioFiles(const InputAudioFileList &files)
{
    this->blockSignals(true);
    for (int i = 0; i < files.size(); ++i) {
        setAudioFile(files.at(i), i);
    }
    this->blockSignals(false);
}

/**************************************
 *
 **************************************/
QStringList Disc::audioFileNames() const
{
    QStringList res;
    for (const InputAudioFile &a : audioFiles()) {
        res << a.fileName();
    }
    return res;
}

/**************************************
 *
 **************************************/
QStringList Disc::audioFilePaths() const
{
    QStringList res;
    for (const InputAudioFile &a : audioFiles()) {
        res << a.filePath();
    }
    return res;
}

/**************************************
 *
 **************************************/
void Disc::setAudioFile(const InputAudioFile &file, int fileNum)
{
    if (mTracks.isEmpty()) {
        assert(fileNum == 0);
        mAudioFile = file;
        return;
    }

    QList<TrackPtrList> tracksList = tracksByFileTag();
    if (fileNum >= tracksList.count()) {
        return;
    }

    TrackPtrList &tracks = tracksList[fileNum];
    for (Track *track : tracks) {
        track->setAudioFile(file);
    }
}

/**************************************
 *
 **************************************/
bool Disc::isMultiAudio() const
{
    return audioFiles().count() > 0;
}

/**************************************
 *
 **************************************/
int Disc::startTrackNum() const
{
    if (mTracks.isEmpty())
        return 0;

    return mTracks.first()->trackNumTag();
}

/**************************************
 *
 **************************************/
void Disc::setStartTrackNum(TrackNum value)
{
    foreach (auto track, mTracks) {
        track->setTrackNumTag(value++);
    }

    Project::instance()->emitDiscChanged(this);
}

/**************************************
 *
 **************************************/
QString Disc::codecName() const
{
    if (mTextCodec.isValid()) {
        return mTextCodec.name();
    }

    return CODEC_AUTODETECT;
}

/**************************************
 *
 **************************************/
void Disc::setCodecName(const QString &codecName)
{
    syncTagsFromTracks();
    if (codecName == CODEC_AUTODETECT) {
        mTextCodec = mCue.detectTextCodec();
    }
    else {
        mTextCodec = TextCodec::codecForName(codecName);
    }
    syncTagsToTracks();

    Project::instance()->emitDiscChanged(this);
}

/**************************************
 *
 **************************************/
DiscNum Disc::discCountTag() const
{
    return mUserTags.discCount() != 0 ? mUserTags.discCount() : mLoadedTags.discCount();
}

/**************************************
 *
 **************************************/
DiscNum Disc::discNumTag() const
{
    return mUserTags.discNum() != 0 ? mUserTags.discNum() : mLoadedTags.discNum();
}

/**************************************
 *
 **************************************/
TrackNum Disc::trackCountTag() const
{
    return tracks().count();
}

/**************************************
 *
 **************************************/
QString Disc::albumTag() const
{
    return !mUserTags.album().isEmpty() ? mUserTags.album() : mLoadedTags.album();
}

/**************************************
 *
 **************************************/
QString Disc::catalogTag() const
{
    return !mUserTags.catalog().isEmpty() ? mUserTags.catalog() : mLoadedTags.catalog();
}

/**************************************
 *
 **************************************/
QString Disc::cdTextfileTag() const
{
    return !mUserTags.cdTextfile().isEmpty() ? mUserTags.cdTextfile() : mLoadedTags.cdTextfile();
}

/**************************************
 *
 **************************************/
QString Disc::commentTag() const
{
    return !mUserTags.comment().isEmpty() ? mUserTags.comment() : mLoadedTags.comment();
}

/**************************************
 *
 **************************************/
QString Disc::dateTag() const
{
    return !mUserTags.date().isEmpty() ? mUserTags.date() : mLoadedTags.date();
}

/**************************************
 *
 **************************************/
QString Disc::discIdTag() const
{
    return !mUserTags.discId().isEmpty() ? mUserTags.discId() : mLoadedTags.discId();
}

/**************************************
 *
 **************************************/
QString Disc::genreTag() const
{
    return !mUserTags.genre().isEmpty() ? mUserTags.genre() : mLoadedTags.genre();
}

/**************************************
 *
 **************************************/
QString Disc::performerTag() const
{
    return !mUserTags.performer().isEmpty() ? mUserTags.performer() : mLoadedTags.performer();
}

/**************************************
 *
 **************************************/
QString Disc::songWriterTag() const
{
    return !mUserTags.songWriter().isEmpty() ? mUserTags.songWriter() : mLoadedTags.songWriter();
}

/**************************************
 *
 **************************************/
void Disc::setDiscCountTag(DiscNum value)
{
    mUserTags.setDiscCount(value);
}

/**************************************
 *
 **************************************/
void Disc::setDiscNumTag(DiscNum value)
{
    mUserTags.setDiscNum(value);
}

/**************************************
 *
 **************************************/
void Disc::setAlbumTag(const QString &value)
{
    mUserTags.setAlbum(value);
}

/**************************************
 *
 **************************************/
void Disc::setCatalogTag(const QString &value)
{
    mUserTags.setCatalog(value);
}

/**************************************
 *
 **************************************/
void Disc::setCdTextfileTag(const QString &value)
{
    mUserTags.setCdTextfile(value);
}

/**************************************
 *
 **************************************/
void Disc::setCommentTag(const QString &value)
{
    mUserTags.setComment(value);
}

/**************************************
 *
 **************************************/
void Disc::setDateTag(const QString &value)
{
    mUserTags.setDate(value);
}

/**************************************
 *
 **************************************/
void Disc::setDiscIdTag(const QString &value)
{
    mUserTags.setDiscId(value);
}

/**************************************
 *
 **************************************/
void Disc::setGenreTag(const QString &value)
{
    mUserTags.setGenre(value);
}

/**************************************
 *
 **************************************/
void Disc::setPerformerTag(const QString &value)
{
    mUserTags.setPerformer(value);
}

/**************************************
 *
 **************************************/
void Disc::setSongWriterTag(const QString &value)
{
    mUserTags.setSongWriter(value);
}

/**************************************
 *
 **************************************/
AlbumTags Disc::toTags() const
{
    AlbumTags res;

    res.setDiscCount(discCountTag());
    res.setDiscNum(discNumTag());
    res.setTrackCount(trackCountTag());

    res.setAlbum(albumTag());
    res.setCatalog(catalogTag());
    res.setCdTextfile(cdTextfileTag());
    res.setComment(commentTag());
    res.setDate(dateTag());
    res.setDiscId(discIdTag());
    res.setGenre(genreTag());
    res.setPerformer(performerTag());
    res.setSongWriter(songWriterTag());

    return res;
}

/**************************************
 *
 **************************************/
QList<TagsId> Disc::tagSets() const
{
    QList<TagsId> res;
    res << mCue.tagsId();

    for (const InternetTags &tags : mInternetTags) {
        res << tags.tagsId();
    }

    return res;
}

/**************************************
 *
 **************************************/
TagsId Disc::currentTagSet() const
{
    if (mInternetTagsIndex < 0) {
        return mCue.tagsId();
    }

    return mInternetTags[mInternetTagsIndex].tagsId();
}

/**************************************
 *
 **************************************/
void Disc::activateTagSet(const QString &uri)
{
    syncTagsFromTracks();

    if (uri == mCue.tagsId().uri) {
        mInternetTagsIndex = -1;
        syncTagsToTracks();
        Project::instance()->emitLayoutChanged();
        return;
    }

    for (int i = 0; i < mInternetTags.count(); ++i) {
        if (uri == mInternetTags.at(i).tagsId().uri) {
            mInternetTagsIndex = i;
            syncTagsToTracks();
            Project::instance()->emitLayoutChanged();
            break;
        }
    }
}

/**************************************
 *
 **************************************/
void Disc::addInternetTags(const QVector<InternetTags> &tags)
{
    if (tags.isEmpty()) {
        return;
    }

    int minDist  = std::numeric_limits<int>::max();
    int bestDisc = 0;
    for (int i = 0; i < tags.count(); ++i) {
        const InternetTags &t = tags.at(i);
        if (t.tracks().count() < mTracks.count()) {
            continue;
        }

        Tags userTags;
        userTags.resize(t.tracks().count());

        mInternetTags << t;
        mInternetUserTags << userTags;

        int n = distance(t);
        if (n < minDist) {
            minDist  = n;
            bestDisc = i;
        }
    }

    activateTagSet(mInternetTags.at(bestDisc).tagsId().uri);
}

/**************************************
 *
 **************************************/
void Disc::setCoverImageFile(const QString &fileName)
{
    mCoverImageFile    = fileName;
    mCoverImagePreview = QImage();
}

/**************************************
 *
 **************************************/
QImage Disc::coverImagePreview() const
{
    if (!mCoverImageFile.isEmpty() && mCoverImagePreview.isNull()) {
        mCoverImagePreview = coverImage();
        if (!mCoverImagePreview.isNull()) {
            mCoverImagePreview = mCoverImagePreview.scaled(COVER_PREVIEW_SIZE, COVER_PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    return mCoverImagePreview;
}

/**************************************
 *
 **************************************/
QImage Disc::coverImage() const
{
    if (mCoverImageFile.isEmpty())
        return QImage();

    return QImage(mCoverImageFile);
}

/**************************************
 *
 **************************************/
bool compareCoverImages(const QFileInfo &f1, const QFileInfo &f2)
{
    const static QStringList order(QStringList()
                                   << "COVER"
                                   << "FRONT"
                                   << "FOLDER");

    QString f1up = f1.baseName().toUpper();
    QString f2up = f2.baseName().toUpper();

    int n1 = 9999;
    int n2 = 9999;

    for (int i = 0; i < order.count(); ++i) {
        const QString &pattern = order.at(i);

        // complete match ..................
        if (f1up == pattern) {
            n1 = i;
        }

        if (f2up == pattern) {
            n2 = i;
        }

        // filename contains pattern .......
        if (f1up.contains(pattern)) {
            n1 = i + order.count();
        }

        if (f2up.contains(pattern)) {
            n2 = i + order.count();
        }
    }

    if (n1 != n2)
        return n1 < n2;

    // If we have 2 files with same name but in different directories,
    // we choose the nearest (with the shorter path).
    int l1 = f1.absoluteFilePath().length();
    int l2 = f2.absoluteFilePath().length();
    if (l1 != l2)
        return l1 < l2;

    return f1.absoluteFilePath() < f2.absoluteFilePath();
}

/**************************************
 *
 **************************************/
QStringList Disc::searchCoverImages(const QString &startDir)
{
    QFileInfoList files;

    QStringList exts;
    exts << "*.jpg";
    exts << "*.jpeg";
    exts << "*.png";
    exts << "*.bmp";
    exts << "*.tiff";

    QQueue<QString> query;
    query << startDir;

    QSet<QString> processed;
    while (!query.isEmpty()) {
        QDir dir(query.dequeue());

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        foreach (QFileInfo d, dirs) {
            if (d.isSymLink())
                d = QFileInfo(d.symLinkTarget());

            if (!processed.contains(d.absoluteFilePath())) {
                processed << d.absoluteFilePath();
                query << d.absoluteFilePath();
            }
        }

        files << dir.entryInfoList(exts, QDir::Files | QDir::Readable);
    }

    std::stable_sort(files.begin(), files.end(), compareCoverImages);

    QStringList res;
    foreach (QFileInfo f, files)
        res << f.absoluteFilePath();

    return res;
}

/**************************************
 *
 **************************************/
QString Disc::searchCoverImage(const QString &startDir)
{
    QString res;

    for (const QString &file : searchCoverImages(startDir)) {
        QImage img(file);

        if (img.isNull()) {
            continue;
        }

        double ratio = double(img.width()) / double(img.height());

        if (std::abs(1 - ratio) < 0.2) {
            return file;
        }

        if (res.isEmpty()) {
            res = file;
        }
    }

    return res;
}

/**************************************
 *
 **************************************/
void Disc::setState(DiskState value)
{
    mState = value;
}
