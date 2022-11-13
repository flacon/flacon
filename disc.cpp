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
#include "settings.h"
#include "inputaudiofile.h"
#include "formats_in/informat.h"
#include "formats_out/outformat.h"
#include "audiofilematcher.h"

#include "assert.h"
#include <QTextCodec>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QQueue>
#include <QtAlgorithms>
#include <QDebug>
#include <QLoggingCategory>
#include <QBuffer>

namespace {
Q_LOGGING_CATEGORY(LOG, "Disk")
Q_LOGGING_CATEGORY(LOG_SEARCH_AUDIO_FILES, "SearchAudioFiles")
}

#define COVER_PREVIEW_SIZE 500

/************************************************

 ************************************************/
Disc::Disc(QObject *parent) :
    QObject(parent)
{
}

/************************************************

 ************************************************/
Disc::Disc(InputAudioFile &audioFile, QObject *parent) :
    QObject(parent),
    mAudioFile(audioFile)
{
}

/************************************************

************************************************/
Disc::Disc(Cue &cue, QObject *parent) :
    QObject(parent),
    mCue(cue)
{
    mTracks.reserve(cue.tracks().count());
    int i = 0;
    for (const TrackTags &cueTrack : cue.tracks()) {
        Track *track = new Track(this, i, cueTrack);
        i++;
        mTracks << track;
    }

    mCurrentTagsUri = cue.filePath();
    syncTagsFromTracks();
    mTagSets[mCurrentTagsUri].setTitle(cue.title());
}

/************************************************

 ************************************************/
Disc::~Disc()
{
    qDeleteAll(mTracks);
}

/************************************************

************************************************/
void Disc::searchCueFile(bool replaceExisting)
{
    if (!replaceExisting && !mCue.isEmpty()) {
        return;
    }

    QStringList audioFiles = audioFilePaths();
    if (audioFiles.isEmpty()) {
        return;
    }

    // Embedded CUE ........................
    try {
        QByteArray embeddedCue = this->audioFiles().first().format()->readEmbeddedCue(this->audioFilePaths().first());
        if (!embeddedCue.isEmpty()) {
            QBuffer buf(&embeddedCue);
            buf.open(QBuffer::ReadOnly);
            setCueFile(Cue(&buf, audioFilePaths().first()));
            return;
        }
    }
    catch (const FlaconError &err) {
        qCWarning(LOG) << "Can't parse embedded cue:" << err.what();
    }

    // Serarch CUE files ...................

    QFileInfo     audioFile = QFileInfo(audioFiles.first());
    QList<Cue>    cues;
    QFileInfoList files = audioFile.dir().entryInfoList(QStringList("*.cue"), QDir::Files | QDir::Readable);
    foreach (QFileInfo f, files) {
        try {
            cues << Cue(f.absoluteFilePath());
        }
        catch (FlaconError &) {
            continue; // Just skipping the incorrect files.
        }
    }

    unsigned int bestWeight = 99999;
    Cue          bestDisc;

    foreach (const Cue &cue, cues) {
        AudioFileMatcher matcher(cue.filePath(), cue.tracks());
        if (matcher.containsAudioFile(audioFile.filePath())) {
            unsigned int weight = levenshteinDistance(QFileInfo(cue.filePath()).baseName(), audioFile.baseName());
            if (weight < bestWeight) {
                bestWeight = weight;
                bestDisc   = cue;
            }
        }
    }

    if (!bestDisc.isEmpty()) {
        setCueFile(bestDisc);
    }
}

/************************************************

************************************************/
void Disc::searchAudioFiles(bool replaceExisting)
{
    QString fullPath = QFileInfo(cueFilePath()).filePath();

    qCDebug(LOG_SEARCH_AUDIO_FILES) << "Search audio for" << cueFilePath();
    qCDebug(LOG_SEARCH_AUDIO_FILES) << "fullPath=" << fullPath;
    qCDebug(LOG_SEARCH_AUDIO_FILES) << "Audio files=" << audioFileNames();

    AudioFileMatcher matcher(fullPath, mCue.tracks());
    for (int i = 0; i < audioFiles().count(); ++i) {
        if (audioFiles()[i].isNull() || replaceExisting) {

            QStringList foundAudio = matcher.audioFiles(i);
            for (const QString &file : foundAudio) {

                qCDebug(LOG_SEARCH_AUDIO_FILES) << " *" << i << " test audio file " << file;

                InputAudioFile newAudio(file);
                if (newAudio.isValid()) {
                    qCDebug(LOG_SEARCH_AUDIO_FILES) << "set audio" << newAudio.filePath();
                    setAudioFile(newAudio, i);
                    break;
                }
            }
        }
    }
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
void Disc::setCueFile(const Cue &cueDisc)
{
    mCue = cueDisc;

    InputAudioFileList audioFiles = this->audioFiles();

    // If the tracks contain tags from the Internet and the
    // tags have been changed, we must save the changes.
    syncTagsFromTracks();
    mTagSets.remove(cueFilePath());

    int count = cueDisc.tracks().count();

    // Remove all tags if number of tracks differ from loaded CUE.
    for (auto it = mTagSets.begin(); it != mTagSets.end();) {
        if (it.value().count() != count)
            it = mTagSets.erase(it);
        else
            ++it;
    }

    // Sync count of tracks
    for (int i = mTracks.count(); i < count; ++i) {
        Track *track = new Track(this, i);
        mTracks.append(track);
    }

    while (mTracks.count() > count)
        delete mTracks.takeLast();

    for (int t = 0; t < count; ++t) {
        mTracks[t]->setTags(cueDisc.tracks().at(t));
    }

    mCurrentTagsUri = cueFilePath();
    syncTagsFromTracks();
    mTagSets[mCurrentTagsUri].setTitle(cueDisc.title());

    for (int i = 0; i < qMin(audioFiles.count(), mTracks.count()); ++i) {
        if (!audioFiles[i].isNull()) {
            setAudioFile(InputAudioFile(audioFiles[i]), i);
        }
    }

    project->emitLayoutChanged();
}

/************************************************
 *
 ************************************************/
void Disc::syncTagsFromTracks()
{
    if (mTagSets.isEmpty())
        return;

    Tracks &tags = mTagSets[mCurrentTagsUri];
    if (tags.isEmpty()) {
        tags.resize(mTracks.count());
        tags.setUri(mCurrentTagsUri);
    }

    for (int i = 0; i < qMin(tags.count(), mTracks.count()); ++i) {
        tags[i] = *(mTracks.at(i));
    }
}

/************************************************
 *
 ************************************************/
void Disc::syncTagsToTracks()
{
    if (mTagSets.isEmpty())
        return;

    Tracks &tags = mTagSets[mCurrentTagsUri];
    assert(tags.count() == mTracks.count());

    for (int i = 0; i < mTracks.count(); ++i) {
        Track       *track = mTracks[i];
        const Track &tgs   = tags.at(i);

        track->setTag(TagId::Album, tgs.tagValue(TagId::Album));
        track->setTag(TagId::Date, tgs.tagValue(TagId::Date));
        track->setTag(TagId::Genre, tgs.tagValue(TagId::Genre));
        track->setTag(TagId::Artist, tgs.tagValue(TagId::Artist));
        track->setTag(TagId::SongWriter, tgs.tagValue(TagId::SongWriter));
        track->setTag(TagId::Title, tgs.tagValue(TagId::Title));
        track->setTag(TagId::AlbumArtist, tgs.tagValue(TagId::AlbumArtist));
    }
}

/************************************************
 *
 ************************************************/
int Disc::distance(const Tracks &other)
{
    if (mTracks.isEmpty() || other.empty())
        return std::numeric_limits<int>::max();

    int res = 0;

    QString str1 = mTracks.first()->artist().toUpper().replace("THE ", "");
    QString str2 = other.first().artist().toUpper().replace("THE ", "");
    res += levenshteinDistance(str1, str2) * 3;

    str1 = mTracks.first()->album().toUpper().replace("THE ", "");
    str2 = other.first().album().toUpper().replace("THE ", "");
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

    auto       it    = mTracks.constBegin();
    QByteArray value = (*it)->tagData(tagId);
    ++it;

    for (; it != mTracks.constEnd(); ++it) {
        if ((*it)->tagData(tagId) != value)
            return false;
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

    int n = 0;
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
        ++n;
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

    project->emitDiscChanged(this);
}

/************************************************

 ************************************************/
QString Disc::codecName() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->codecName();

    return "";
}

/************************************************

 ************************************************/
void Disc::setCodecName(const QString &codecName)
{

    QString codec = codecName;

    if (codecName == CODEC_AUTODETECT) {
        UcharDet charDet;
        foreach (auto track, mTracks)
            charDet << *track;

        codec = charDet.textCodecName();
    }

    foreach (auto track, mTracks) {
        track->setCodecName(codec);
    }

    project->emitDiscChanged(this);
}

/************************************************

 ************************************************/
QString Disc::tagSetTitle() const
{
    if (mCurrentTagsUri.isEmpty())
        return "";

    const_cast<Disc *>(this)->syncTagsFromTracks();
    return mTagSets[mCurrentTagsUri].title();
}

/************************************************

 ************************************************/
QString Disc::tagsUri() const
{
    return mCurrentTagsUri;
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
QString Disc::fileTag() const
{
    if (!mTracks.isEmpty())
        return mTracks.first()->tag(TagId::File);

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
QVector<Disc::TagSet> Disc::tagSets() const
{
    if (mTagSets.isEmpty())
        return QVector<TagSet>();

    QStringList keys = mTagSets.keys();
    keys.removeAll(cueFilePath());
    std::sort(keys.begin(), keys.end());
    keys.prepend(cueFilePath());

    QVector<TagSet> res;
    foreach (const QString &key, keys) {
        const Tracks &tags = mTagSets[key];

        TagSet ts;
        ts.uri  = tags.uri();
        ts.name = tags.title();
        res << ts;
    }

    return res;
}

/************************************************
 *
 ************************************************/
void Disc::addTagSet(const Tracks &tags, bool activate)
{
    mTagSets[tags.uri()] = tags;
    // Sometimes CDDB response contains an additional
    // DATA track. For example
    // http://freedb.freedb.org/~cddb/cddb.cgi?cmd=CDDB+READ+rock+A20FA70C&hello=a+127.0.0.1+f+0&proto=5
    mTagSets[tags.uri()].resize(mTracks.count());

    if (activate)
        activateTagSet(tags.uri());
}

/************************************************
 *
 ************************************************/
void Disc::activateTagSet(const QString &uri)
{
    if (!mTagSets.contains(uri))
        return;

    syncTagsFromTracks();
    mCurrentTagsUri = uri;
    syncTagsToTracks();
    project->emitLayoutChanged();
}

/************************************************
 *
 ************************************************/
void Disc::addTagSets(const QVector<Tracks> &discs)
{
    if (discs.isEmpty())
        return;

    int minDist  = std::numeric_limits<int>::max();
    int bestDisc = 0;
    for (int i = 0; i < discs.count(); ++i) {
        const Tracks &disc = discs.at(i);
        if (disc.count() < mTracks.count())
            continue;

        addTagSet(disc, false);
        int n = distance(disc);
        if (n < minDist) {
            minDist  = n;
            bestDisc = i;
        }
    }

    activateTagSet(discs.at(bestDisc).uri());
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
            mCoverImagePreview.scaled(COVER_PREVIEW_SIZE, COVER_PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
    if (isEmpty())
        return "";

    return mTracks.first()->tag(tagId);
}

/************************************************
 *
 ************************************************/
QByteArray Disc::discTagData(TagId tagId) const
{
    if (isEmpty())
        return QByteArray();

    return mTracks.first()->tagData(tagId);
}

/************************************************
 *
 ************************************************/
void Disc::setDiscTag(TagId tagId, const QString &value)
{
    foreach (auto track, mTracks)
        track->setTag(tagId, value);
}

/************************************************
 *
 ************************************************/
void Disc::setDiscTag(TagId tagId, const QByteArray &value)
{
    foreach (auto track, mTracks)
        track->setTag(tagId, value);
}

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
void Disc::trackChanged(TagId tagId)
{
    if (mTracks.isEmpty())
        return;

    if (tagId == TagId::Artist && isSameTagValue(TagId::Artist)) {
        foreach (Track *track, mTracks) {
            track->setTag(TagId::AlbumArtist, track->tagValue(TagId::Artist));
        }
    }
}

/************************************************
 *
 ************************************************/
Duration Disc::trackDuration(const Track &track) const
{
    if (mCue.isEmpty()) {
        return 0;
    }

    TrackTags cur  = mCue.tracks().at(track.index());
    TrackTags next = (track.index() < mTracks.count() - 1) ? mCue.tracks().at(track.index() + 1) : TrackTags();

    Duration trackLen = 0;
    if (cur.cueIndex(1).file() != next.cueIndex(0).file()) {
        uint start = cur.cueIndex(1).milliseconds();
        uint end   = track.audioFile().duration();
        trackLen   = end > start ? end - start : 0;
    }
    else {
        uint start = cur.cueIndex(1).milliseconds();
        uint end   = next.cueIndex(0).milliseconds();
        trackLen   = end > start ? end - start : 0;
    }

    Duration postGapLen = 0;
    if (next.cueIndex(0).file() != next.cueIndex(1).file()) {
        uint start = next.cueIndex(0).milliseconds();
        uint end   = track.audioFile().duration();
        postGapLen = end > start ? end - start : 0;
    }
    else {
        uint start = next.cueIndex(0).milliseconds();
        uint end   = next.cueIndex(1).milliseconds();
        postGapLen = end > start ? end - start : 0;
    }

    /*
    qDebug() << "***************************************";
    qDebug() << "CUR:" << track.index();
    qDebug() << "  - 00 " << cur.cueIndex(0).toString() << cur.cueIndex(0).file();
    qDebug() << "  - 01 " << cur.cueIndex(1).toString() << cur.cueIndex(1).file();
    qDebug() << ".......................................";
    qDebug() << "NEXT:";
    qDebug() << "  - 00 " << next.cueIndex(0).toString() << next.cueIndex(0).file();
    qDebug() << "  - 01 " << next.cueIndex(1).toString() << next.cueIndex(1).file();
    qDebug() << ".......................................";
    qDebug() << "trackLen: " << trackLen;
    qDebug() << "postGapLen: " << postGapLen;
    qDebug() << "***************************************";
    */
    return trackLen + postGapLen;
}
