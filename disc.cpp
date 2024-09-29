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

/************************************************

 ************************************************/
static TagSet toTagSet(const Cue &cue)
{
    return TagSet { cue.filePath(), cue.title() };
}

/************************************************

 ************************************************/
static TagSet toTagSet(const InternetTags &tags)
{
    return TagSet { tags.uri(), tags.name() };
}

/************************************************

 ************************************************/
Disc::Disc(QObject *parent) :
    QObject(parent)
{
}

/************************************************

 ************************************************/
Disc::~Disc()
{
    qDeleteAll(mTracks);
}

/************************************************

************************************************/
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

/************************************************

 ************************************************/
Track *Disc::track(int index) const
{
    return mTracks.at(index);
}

/************************************************
 *
 ************************************************/
const Track *Disc::preGapTrack() const
{
    if (!mTracks.isEmpty()) {
        mPreGapTrack = *mTracks.first();
    }

    mPreGapTrack.setTag(TagId::TrackNum, QByteArray("0"));
    mPreGapTrack.setTitle("(HTOA)");
    return &mPreGapTrack;
}

/************************************************
 *
 ************************************************/
QString Disc::cueFilePath() const
{
    return mCue.isEmpty() ? "" : mCue.filePath();
}

/************************************************

 ************************************************/
void Disc::setCue(const Cue &cue)
{
    mCue = cue;

    if (cue.isEmpty()) {
        return;
    }

    InputAudioFileList audioFiles = this->audioFiles();

    mCueUserTags.resize(cue.trackCount());

    // Remove all tags if number of tracks differ from loaded CUE.
    for (int i = mInternetTags.size() - 1; i >= 0; i--) {
        if (mInternetTags.at(i).trackCount() != cue.trackCount()) {
            mInternetTags.removeAt(i);
        }
    }

    // Sync count of tracks
    for (int i = mTracks.count(); i < cue.trackCount(); ++i) {
        Track *track = new Track(this, i);
        mTracks.append(track);
    }

    while (mTracks.count() > cue.trackCount()) {
        delete mTracks.takeLast();
    }

    mInternetTagsIndex = -1;

    for (int i = 0; i < qMin(audioFiles.count(), mTracks.count()); ++i) {
        if (!audioFiles[i].isNull()) {
            setAudioFile(InputAudioFile(audioFiles[i]), i);
        }
    }

    Project::instance()->emitLayoutChanged();
    emit revalidateRequested();
}

/************************************************
 *
 ************************************************/
int Disc::distance(const Tags &other)
{
    if (mTracks.isEmpty() || other.isEmpty()) {
        return std::numeric_limits<int>::max();
    }

    int res = 0;

    QString str1 = mTracks.first()->artist().toUpper().replace("THE ", "");
    QString str2 = other.albumTag(TagId::Artist).toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2) * 3;

    str1 = mTracks.first()->album().toUpper().replace("THE ", "");
    str2 = other.albumTag(TagId::Album).toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2);

    return res;
}

/************************************************
 *
 ************************************************/
bool Disc::isSameTagValue(TagId tagId)
{
    if (mTracks.isEmpty())
        return false;

    QString value = mTracks.first()->tag(tagId);

    for (const Track *t : mTracks) {
        if (t->tag(tagId) != value) {
            return false;
        }
    }

    return true;
}

/************************************************

************************************************/
QList<TrackPtrList> Disc::tracksByFileTag() const
{
    QList<TrackPtrList> res;
    if (mTracks.isEmpty()) {
        return res;
    }

    int b = 0;
    while (b < mTracks.count()) {
        int e = b;
        res.append(TrackPtrList());
        TrackPtrList &list = res.last();

        QString prev = mTracks[b]->tag(TagId::File);
        for (; e < count() && mTracks[e]->tag(TagId::File) == prev; ++e) {
            list << mTracks[e];
        }

        b = e;
    }

    return res;
}

/************************************************

************************************************/
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

/************************************************

************************************************/
void Disc::setAudioFiles(const InputAudioFileList &files)
{
    this->blockSignals(true);
    for (int i = 0; i < files.size(); ++i) {
        setAudioFile(files.at(i), i);
    }
    this->blockSignals(false);
    emit revalidateRequested();
}

/************************************************

************************************************/
QStringList Disc::audioFileNames() const
{
    QStringList res;
    for (const InputAudioFile &a : audioFiles()) {
        res << a.fileName();
    }
    return res;
}

/************************************************

************************************************/
QStringList Disc::audioFilePaths() const
{
    QStringList res;
    for (const InputAudioFile &a : audioFiles()) {
        res << a.filePath();
    }
    return res;
}

/************************************************

************************************************/
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

    emit revalidateRequested();
}

/************************************************

************************************************/
bool Disc::isMultiAudio() const
{
    return audioFiles().count() > 0;
}

/************************************************
 *
 ************************************************/
int Disc::startTrackNum() const
{
    if (mTracks.isEmpty())
        return 0;

    return mTracks.first()->trackNum();
}

/************************************************

 ************************************************/
void Disc::setStartTrackNum(TrackNum value)
{
    foreach (auto track, mTracks) {
        track->setTrackNum(value++);
    }

    Project::instance()->emitDiscChanged(this);
}

/************************************************

 ************************************************/
QString Disc::codecName() const
{
    if (mTextCodec.isValid()) {
        return mTextCodec.name();
    }

    return CODEC_AUTODETECT;
}

/************************************************

 ************************************************/
void Disc::setCodecName(const QString &codecName)
{
    if (codecName == CODEC_AUTODETECT) {
        mTextCodec = mCue.detectTextCodec();
    }
    else {
        mTextCodec = TextCodec::codecForName(codecName);
    }

    Project::instance()->emitDiscChanged(this);
}

/************************************************
 *
 ************************************************/
QString Disc::discId() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->tag(TagId::DiscId);

    return "";
}

/************************************************
 *
 ************************************************/
DiscNum Disc::discNum() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->discNum();

    return 0;
}

/************************************************
 *
 ************************************************/
DiscNum Disc::discCount() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->discCount();

    return 0;
}

/************************************************
 *
 ************************************************/
QList<TagSet> Disc::tagSets() const
{
    QList<TagSet> res;
    res << toTagSet(mCue);

    for (const InternetTags &tags : mInternetTags) {
        res << toTagSet(tags);
    }

    return res;
}

/************************************************
 *
 ************************************************/
TagSet Disc::currentTagSet() const
{
    if (mInternetTagsIndex < 0) {
        return toTagSet(mCue);
    }

    return toTagSet(mInternetTags[mInternetTagsIndex]);
}

/************************************************
 *
 ************************************************/
void Disc::activateTagSet(const QString &uri)
{
    if (uri == toTagSet(mCue).uri) {
        mInternetTagsIndex = -1;
        Project::instance()->emitLayoutChanged();
        return;
    }

    for (int i = 0; i < mInternetTags.count(); ++i) {
        if (uri == mInternetTags.at(i).uri()) {
            mInternetTagsIndex = i;
            Project::instance()->emitLayoutChanged();
            return;
        }
    }
}

/************************************************
 *
 ************************************************/
void Disc::addInternetTags(const QVector<InternetTags> &tags)
{
    if (tags.isEmpty())
        return;

    int minDist  = std::numeric_limits<int>::max();
    int bestDisc = 0;
    for (int i = 0; i < tags.count(); ++i) {
        const InternetTags &t = tags.at(i);
        if (t.trackCount() < mTracks.count()) {
            continue;
        }

        mInternetTags << t;
        int n = distance(t);
        if (n < minDist) {
            minDist  = n;
            bestDisc = i;
        }
    }

    activateTagSet(mInternetTags.at(bestDisc).uri());
}

/************************************************

 ************************************************/
void Disc::setCoverImageFile(const QString &fileName)
{
    mCoverImageFile    = fileName;
    mCoverImagePreview = QImage();
}

/************************************************

 ************************************************/
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

/************************************************

 ************************************************/
QImage Disc::coverImage() const
{
    if (mCoverImageFile.isEmpty())
        return QImage();

    return QImage(mCoverImageFile);
}

/************************************************
 *
 ************************************************/
QString Disc::discTag(TagId tagId) const
{
    if (isEmpty()) {
        return "";
    }

    return mTracks.first()->tag(tagId);
}

/************************************************
 *
 ************************************************/
// void Disc::setDiscTag(TagId tagId, const QString &value)
// {
//     foreach (auto track, mTracks)
//         track->setTag(tagId, value);
// }

/************************************************

 ************************************************/
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

/************************************************

 ************************************************/
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

/************************************************

 ************************************************/
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

/************************************************
 *
 ************************************************/
void Disc::setState(DiskState value)
{
    mState = value;
}

/************************************************
 *
 ************************************************/
void Disc::trackChanged(TagId tagId)
{
    if (mTracks.isEmpty())
        return;

    if (tagId == TagId::Artist && isSameTagValue(TagId::Artist)) {
        foreach (Track *track, mTracks) {
            track->setTag(TagId::AlbumArtist, track->tag(TagId::Artist));
        }
    }

    emit tagChanged();
}

/************************************************
 *
 ************************************************/
Duration Disc::trackDuration(const Track &track) const
{
    if (mCue.isEmpty()) {
        return 0;
    }

    Cue::Track cur  = mCue.tracks().at(track.index());
    Cue::Track next = (track.index() < mTracks.count() - 1) ? mCue.tracks().at(track.index() + 1) : Cue::Track();

    Duration trackLen = 0;
    if (cur.cueIndex01.file() != next.cueIndex00.file()) {
        uint start = cur.cueIndex01.milliseconds();
        uint end   = track.audioFile().duration();
        trackLen   = end > start ? end - start : 0;
    }
    else {
        uint start = cur.cueIndex01.milliseconds();
        uint end   = next.cueIndex00.milliseconds();
        trackLen   = end > start ? end - start : 0;
    }

    Duration postGapLen = 0;
    if (next.cueIndex00.file() != next.cueIndex01.file()) {
        uint start = next.cueIndex00.milliseconds();
        uint end   = track.audioFile().duration();
        postGapLen = end > start ? end - start : 0;
    }
    else {
        uint start = next.cueIndex00.milliseconds();
        uint end   = next.cueIndex01.milliseconds();
        postGapLen = end > start ? end - start : 0;
    }

    /*
    qDebug() << "***************************************";
    qDebug() << "CUR:" << track.index();
    qDebug() << "  - 00 " << cur.cueIndex00.toString() << cur.cueIndex00.file();
    qDebug() << "  - 01 " << cur.cueIndex01.toString() << cur.cueIndex01.file();
    qDebug() << ".......................................";
    qDebug() << "NEXT:";
    qDebug() << "  - 00 " << next.cueIndex00.toString() << next.cueIndex00.file();
    qDebug() << "  - 01 " << next.cueIndex01.toString() << next.cueIndex01.file();
    qDebug() << ".......................................";
    qDebug() << "trackLen: " << trackLen;
    qDebug() << "postGapLen: " << postGapLen;
    qDebug() << "***************************************";
    */
    return trackLen + postGapLen;
}

QString Disc::trackTag(int trackIndex, TagId tagId)
{
    if (mInternetTagsIndex > -1) {
        return mInternetTags[mInternetTagsIndex].trackTag(trackIndex, tagId);
    }

    if (mCueUserTags.containsTrackTag(trackIndex, tagId)) {
        return mCueUserTags.trackTag(trackIndex, tagId);
    }

    return mTextCodec.decode(mCue.tracks().at(trackIndex).tags.value(tagId));
}

void Disc::setTrackTag(int trackIndex, TagId tagId, const QString &value)
{
    if (mInternetTagsIndex > -1) {
        mInternetTags[mInternetTagsIndex].setTrackTag(trackIndex, tagId, value);
        return;
    }

    mCueUserTags.setTrackTag(trackIndex, tagId, value);
}

CueIndex Disc::trackCueIndex00(int trackIndex)
{
    return mCue.tracks().at(trackIndex).cueIndex00;
}

CueIndex Disc::trackCueIndex01(int trackIndex)
{
    return mCue.tracks().at(trackIndex).cueIndex01;
}
