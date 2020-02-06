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
#include "formats/informat.h"
#include "outformat.h"

#include "assert.h"
#include <QTextCodec>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <QQueue>
#include <QtAlgorithms>
#include <QDebug>

#define COVER_PREVIEW_SIZE 500


/************************************************

 ************************************************/
Disc::Disc(QObject *parent) :
    QObject(parent),
    mAudioFile(nullptr)
{

}


/************************************************

 ************************************************/
Disc::~Disc()
{
    delete mAudioFile;
    qDeleteAll(mTracks);
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
    if (!mTracks.isEmpty())
    {
        mPreGapTrack = *mTracks.first();
        mPreGapTrack.setCueFileName(mTracks.first()->cueFileName());
    }

    mPreGapTrack.setTag(TagId::TrackNum, QByteArray("0"));
    mPreGapTrack.setTitle("(HTOA)");
    return &mPreGapTrack;
}


/************************************************

 ************************************************/
bool Disc::canConvert(QString *description) const
{
    bool res = true;
    QStringList msg;

    if (!mAudioFile || !mAudioFile->isValid())
    {
        msg << tr("Audio file not set.");
        res = false;
    }

    if (count() < 1)
    {
        msg << tr("Cue file not set.");
        res = false;
    }

    if (res)
    {
        uint duration = 0;
        for (int i=0; i<mTracks.count()-1; ++i)
            duration += track(i)->duration();

        if (mAudioFile->duration() <= duration)
        {
            msg << tr("Audio file shorter than expected from CUE sheet.");
            res = false;
        }

    }

    if (description)
        *description = msg.join("\n");

    return res;
}



/************************************************

 ************************************************/
bool Disc::canDownloadInfo() const
{
    return !discId().isEmpty();
}


/************************************************

 ************************************************/
void Disc::loadFromCue(const CueDisc &cueDisc)
{
    QString oldDir = QFileInfo(mCueFile).dir().absolutePath();

    // If the tracks contain tags from the Internet and the
    // tags have been changed, we must save the changes.
    syncTagsFromTracks();
    mTagSets.remove(mCueFile);

    int count = cueDisc.count();
    mCueFile = cueDisc.fileName();

    // Remove all tags if number of tracks differ from loaded CUE.
    for(auto it = mTagSets.begin(); it != mTagSets.end();)
    {
        if (it.value().count() != count)
            it = mTagSets.erase(it);
        else
            ++it;
    }


    // Sync count of tracks
    for (int i=mTracks.count(); i<count; ++i)
    {
        Track *track = new Track();
        connect(track, &Track::tagChanged, this, &Disc::trackChanged);
        mTracks.append(track);

    }

    while (mTracks.count() > count)
        delete mTracks.takeLast();


    for (int t=0; t<count; ++t)
        *mTracks[t] = cueDisc.at(t);

    for (int i=0; i<count; ++i)
    {
        Track *track = mTracks[i];
        track->mDuration = this->trackDuration(i);
    }

    mCurrentTagsUri = mCueFile;
    syncTagsFromTracks();
    mTagSets[mCurrentTagsUri].setTitle(cueDisc.title());

    if (!mAudioFile)
        findAudioFile(cueDisc);


    QString dir = QFileInfo(mCueFile).dir().absolutePath();
    if (dir != oldDir)
    {
        mCoverImagePreview = QImage();
        mCoverImageFile = searchCoverImage(dir);
    }

    project->emitLayoutChanged();
}


/************************************************

 ************************************************/
QFileInfoList matchedAudioFiles(const CueDisc &cueDisc, const QFileInfoList &audioFiles)
{
    QFileInfoList res;
    QFileInfo cueFile(cueDisc.fileName());

    QStringList patterns;
    if (cueDisc.discCount() > 1)
    {
        patterns << QRegExp::escape(QFileInfo(cueDisc.first().tag(TagId::File)).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + QString("(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueDisc.discNum());
        patterns << QString(".*" "(disk|disc|side)" "(.*\\D)?" "0*" "%1" "(.*\\D)?").arg(cueDisc.discNum());
    }
    else
    {
        patterns << QRegExp::escape(QFileInfo(cueDisc.first().tag(TagId::File)).completeBaseName());
        patterns << QRegExp::escape(cueFile.completeBaseName()) + ".*";
    }

    QString audioExt;
    foreach (const InputFormat *format, InputFormat::allFormats())
        audioExt += (audioExt.isEmpty() ? "\\." : "|\\.") + format->ext();


    foreach (const QString &pattern, patterns)
    {
        QRegExp re(QString("%1(%2)+").arg(pattern).arg(audioExt), Qt::CaseInsensitive, QRegExp::RegExp2);      
        foreach (const QFileInfo &audio, audioFiles)
        {
            if (re.exactMatch(audio.fileName()))
                res << audio;
        }
    }

    return res;
}


/************************************************

 ************************************************/
void Disc::findCueFile()
{
    if (!mAudioFile)
        return;

    QFileInfo audio(mAudioFile->fileName());
    QList<Cue> cues;
    QFileInfoList files = audio.dir().entryInfoList(QStringList() << "*.cue", QDir::Files | QDir::Readable);
    foreach(QFileInfo f, files)
    {
        try
        {
            CueReader c;
            cues << c.load(f.absoluteFilePath());
        }
        catch (FlaconError&)
        {
            continue; // Just skipping the incorrect files.
        }
    }

    if (cues.count() == 1 && cues.first().count() == 1)
    {
        loadFromCue(cues.first().first());
        return;
    }

    unsigned int bestWeight = 99999;
    CueDisc bestDisc;

    foreach (const Cue &cue, cues)
    {
        foreach (const CueDisc &cueDisc, cue)
        {
            if (!matchedAudioFiles(cueDisc, QFileInfoList() << audio).isEmpty())
            {
                unsigned int weight = levenshteinDistance(QFileInfo(cueDisc.fileName()).baseName(), audio.baseName());
                if (weight < bestWeight)
                {
                    bestWeight = weight;
                    bestDisc   = cueDisc;
                }
            }
        }
    }

    if (!bestDisc.uri().isEmpty())
        loadFromCue(bestDisc);
}


/************************************************
 *
 ************************************************/
Duration Disc::trackDuration(TrackNum trackNum) const
{
    const Track *track = mTracks[trackNum];
    uint start = track->cueIndex(1).milliseconds();
    uint end = 0;

    if (trackNum < mTracks.count() - 1)
    {
        end = mTracks.at(trackNum+1)->cueIndex(1).milliseconds();
    }
    else if (audioFile() != nullptr)
    {
        end   = audioFile()->duration();
    }

    return end > start ? end - start : 0;
}


/************************************************
 *
 ************************************************/
void Disc::syncTagsFromTracks()
{
    if (mTagSets.isEmpty())
        return;

    Tracks &tags = mTagSets[mCurrentTagsUri];
    if (tags.isEmpty())
    {
        tags.resize(mTracks.count());
        tags.setUri(mCurrentTagsUri);
    }

    for (int i=0; i<qMin(tags.count(), mTracks.count()); ++i)
    {
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

    for (int i=0; i<mTracks.count(); ++i)
    {
        Track *track = mTracks[i];
        const Track &tgs = tags.at(i);

        track->blockSignals(true);
        track->setTag(TagId::Album,       tgs.tagValue(TagId::Album));
        track->setTag(TagId::Date,        tgs.tagValue(TagId::Date));
        track->setTag(TagId::Genre,       tgs.tagValue(TagId::Genre));
        track->setTag(TagId::Artist,      tgs.tagValue(TagId::Artist));
        track->setTag(TagId::SongWriter,  tgs.tagValue(TagId::SongWriter));
        track->setTag(TagId::Title,       tgs.tagValue(TagId::Title));
        track->setTag(TagId::AlbumArtist, tgs.tagValue(TagId::AlbumArtist));
        track->blockSignals(false);
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

    auto it = mTracks.constBegin();
    QByteArray value = (*it)->tagData(tagId);
    ++it;

    for (; it != mTracks.constEnd(); ++it)
    {
        if ((*it)->tagData(tagId) != value)
            return false;
    }

    return true;

}



/************************************************

 ************************************************/
QString Disc::audioFileName() const
{
    if (mAudioFile)
        return mAudioFile->fileName();
    else
        return "";
}


/************************************************

 ************************************************/
void Disc::setAudioFile(const InputAudioFile &audio)
{
    if (mAudioFile)
        delete mAudioFile;

    mAudioFile = new InputAudioFile(audio);

    if (mTracks.isEmpty())
        findCueFile();

    if (!mTracks.isEmpty())
        mTracks.last()->mDuration = trackDuration(mTracks.count() - 1);
}


/************************************************

 ************************************************/
void Disc::findAudioFile(const CueDisc &cueDisc)
{
    if (cueDisc.isEmpty())
        return;

    QStringList exts;
    foreach (const InputFormat *format, InputFormat::allFormats())
        exts << QString("*.%1").arg(format->ext());

    QFileInfo cueFile(mCueFile);
    QFileInfoList files = cueFile.dir().entryInfoList(exts, QDir::Files | QDir::Readable);

    if (cueDisc.discCount() == 1 && files.count() == 1)
    {
        InputAudioFile audio(files.first().filePath());
        if (audio.isValid())
            setAudioFile(audio);
        return;
    }

    QFileInfoList audioFiles = matchedAudioFiles(cueDisc, files);
    foreach (const QFileInfo &file, audioFiles)
    {
        InputAudioFile audio(file.filePath());
        if (audio.isValid())
        {
            setAudioFile(audio);
            return;
        }
    }
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
void Disc::setStartTrackNum(int value)
{
    foreach (auto track, mTracks)
        track->setTrackNum(value++);

    project->emitDiskChanged(this);
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

    if (codecName == CODEC_AUTODETECT)
    {
        UcharDet charDet;
        foreach (auto track, mTracks)
            charDet << *track;

        codec = charDet.textCodecName();
    }


    foreach (auto track, mTracks)
    {
        track->setCodecName(codec);
    }


    project->emitDiskChanged(this);
}


/************************************************

 ************************************************/
QString Disc::tagSetTitle() const
{
    if (mCurrentTagsUri.isEmpty())
        return "";

    const_cast<Disc*>(this)->syncTagsFromTracks();
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
QStringList Disc::warnings() const
{
    QStringList res;
    if (audioFile())
    {
        if (audioFile()->bitsPerSample() > int(Settings::i()->currentProfile().maxBitPerSample()))
            res << tr("A maximum of %1-bit per sample is supported by this format. This value will be used for encoding.", "Warning message")
                      .arg(int(Settings::i()->currentProfile().maxBitPerSample()));

        if (audioFile()->sampleRate() > int(Settings::i()->currentProfile().maxSampleRate()))
            res << tr("A maximum sample rate of %1 is supported by this format. This value will be used for encoding.", "Warning message")
                      .arg(int(Settings::i()->currentProfile().maxSampleRate()));
    }
    return res;
}


/************************************************
 *
 ************************************************/
QVector<Disc::TagSet> Disc::tagSets() const
{
    if (mTagSets.isEmpty())
        return QVector<TagSet>();

    QStringList keys = mTagSets.keys();
    keys.removeAll(mCueFile);
    std::sort(keys.begin(), keys.end());
    keys.prepend(mCueFile);

    QVector<TagSet> res;
    foreach (const QString &key, keys)
    {
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

    int minDist = std::numeric_limits<int>::max();
    int bestDisc = 0;
    for (int i=0; i<discs.count(); ++i)
    {
        const Tracks &disc = discs.at(i);
        if (disc.count() < mTracks.count())
            continue;

        addTagSet(disc, false);
        int n = distance(disc);
        if (n<minDist)
        {
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
    mCoverImageFile = fileName;
    mCoverImagePreview = QImage();
}


/************************************************

 ************************************************/
QImage Disc::coverImagePreview() const
{
    if (!mCoverImageFile.isEmpty() && mCoverImagePreview.isNull())
    {
        mCoverImagePreview = coverImage();
        if (!mCoverImagePreview.isNull())
        {
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
    foreach (auto track,  mTracks)
        track->setTag(tagId, value);
}


/************************************************
 *
 ************************************************/
void Disc::setDiscTag(TagId tagId, const QByteArray &value)
{
    foreach (auto track,  mTracks)
        track->setTag(tagId, value);
}


/************************************************

 ************************************************/
bool compareCoverImages(const QFileInfo &f1, const QFileInfo &f2)
{
    static QStringList order(QStringList()
            << "COVER"
            << "FRONT"
            << "FOLDER");

    int n1 = order.indexOf(f1.baseName().toUpper());
    if (n1 < 0)
        n1 = 9999;

    int n2 = order.indexOf(f2.baseName().toUpper());
    if (n2 < 0)
        n2 = 9999;

    if (n1 != n2)
        return n1 < n2;

    // If we have 2 files with same name but in different directories,
    // we choose the nearest (with the shorter path).
    int l1 = f1.absoluteFilePath().length();
    int l2 = f2.absoluteFilePath().length();
    if (l1 != l2)
        return l1<l2;

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
    while (!query.isEmpty())
    {
        QDir dir(query.dequeue());

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        foreach(QFileInfo d, dirs)
        {
            if (d.isSymLink())
                d = QFileInfo(d.symLinkTarget());

            if (!processed.contains(d.absoluteFilePath()))
            {
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
    QStringList l = searchCoverImages(startDir);
    foreach (QString file, l)
    {
        QImage img(file);
        if (!img.isNull())
            return file;
    }
    return "";
}


/************************************************
 *
 ************************************************/
void Disc::trackChanged(TagId tagId)
{
    if (mTracks.isEmpty())
        return;

    if (tagId == TagId::Artist && isSameTagValue(TagId::Artist))
    {
        foreach (Track *track, mTracks)
        {
            track->setTag(TagId::AlbumArtist, track->tagValue(TagId::Artist));
        }
    }

}
