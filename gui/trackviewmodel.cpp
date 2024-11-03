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

#include "trackviewmodel.h"
#include "trackview.h"
#include "project.h"
#include "disc.h"

#include <QDebug>
#include <QSet>

struct CacheTrackData
{
    CacheTrackData() :
        state(TrackState::NotRunning),
        percent(0),
        discNum(-1),
        trackNum(-1)
    {
    }

    TrackState state;
    Percent    percent;
    DiscNum    discNum;
    TrackNum   trackNum;
};

class TrackViewModel::Cache
{
public:
    Cache() = default;
    QSet<DiscNum>                downloadedDiscs;
    QHash<Track, CacheTrackData> tracks;
};

class IndexData
{
public:
    explicit IndexData(const QModelIndex &index)
    {
        mDiscId  = (quint32(index.internalId()) & 0xFFFFFF);
        mTrackId = (quint32(index.internalId()) >> 16);
    }

    explicit IndexData(quint16 discNum, quint16 trackNum)
    {
        mDiscId  = discNum + 1;
        mTrackId = trackNum + 1;
    }

    explicit IndexData(quint16 discNum)
    {
        mDiscId  = discNum + 1;
        mTrackId = 0;
    }

    quintptr asPtr()
    {
        return quintptr((mTrackId << 16) | mDiscId);
    }

    bool isDisc() const { return mDiscId > 0; }
    bool isTrack() const { return mTrackId > 0; }

    int discNum() const { return mDiscId - 1; }
    int trackNum() const { return mTrackId - 1; }

    Disc *disc() const
    {
        if (mDiscId && mDiscId - 1 < Project::instance()->count())
            return Project::instance()->disc(mDiscId - 1);

        return nullptr;
    }

    Track *track() const
    {
        if (mTrackId) {
            Disc *disc = this->disc();
            if (disc && mTrackId - 1 < disc->count())
                return disc->track(mTrackId - 1);
        }
        return nullptr;
    }

private:
    quint16 mDiscId;
    quint16 mTrackId;
};

/************************************************
 *
 ************************************************/
inline uint qHash(const Track &track, uint seed)
{
    return qHash(QPair<int, intptr_t>(track.trackNum(), intptr_t(track.disc())), seed);
}

/************************************************

 ************************************************/
TrackViewModel::TrackViewModel(TrackView *parent) :
    QAbstractItemModel(parent),
    mCache(new Cache()),
    mView(parent)
{
    connect(Project::instance(), &Project::discChanged,
            this, &TrackViewModel::discDataChanged);

    connect(Project::instance(), &Project::layoutChanged,
            [this]() { this->layoutChanged(); });

    connect(Project::instance(), &Project::afterRemoveDisc,
            [this]() { this->layoutChanged(); });

    connect(Project::instance(), &Project::beforeRemoveDisc,
            this, &TrackViewModel::invalidateCache);
}

/************************************************
 *
 ************************************************/
TrackViewModel::~TrackViewModel()
{
    delete mCache;
}

/************************************************

 ************************************************/
QVariant TrackViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case TrackView::ColumnTracknum:
            return QVariant(tr("Track", "Table header."));
        case TrackView::ColumnDuration:
            return QVariant(tr("Length", "Table header."));
        case TrackView::ColumnTitle:
            return QVariant(tr("Title", "Table header."));
        case TrackView::ColumnArtist:
            return QVariant(tr("Artist", "Table header."));
        case TrackView::ColumnAlbum:
            return QVariant(tr("Album", "Table header."));
        case TrackView::ColumnComment:
            return QVariant(tr("Comment", "Table header."));
        case TrackView::ColumnFileName:
            return QVariant(tr("File", "Table header."));
    }
    return QVariant();
}

/************************************************

 ************************************************/
QModelIndex TrackViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (IndexData(parent).isTrack())
        return QModelIndex();

    if (IndexData(parent).isDisc())
        return createIndex(row, column, IndexData(parent.row(), row).asPtr());

    return createIndex(row, column, IndexData(row).asPtr());
}

/************************************************

 ************************************************/
QModelIndex TrackViewModel::index(const Disc &disc, int col) const
{
    const int discNum = Project::instance()->indexOf(&disc);
    if (discNum > -1 && discNum < rowCount(QModelIndex()))
        return index(discNum, col, QModelIndex());

    return QModelIndex();
}

/************************************************
 *
 ************************************************/
QModelIndex TrackViewModel::index(const Track &track, int col) const
{
    CacheTrackData &cache = mCache->tracks[track];

    const IndexData indexData(cache.discNum, cache.trackNum);
    if (indexData.track() && *indexData.track() == track) {
        return index(cache.trackNum, col, index(cache.discNum, 0));
    }

    // Update cache
    for (int d = 0; d < rowCount(); ++d) {
        QModelIndex discIndex = index(d, 0);
        for (int t = 0; t < rowCount(discIndex); ++t) {
            IndexData indexData(d, t);
            if (indexData.track() && *indexData.track() == track) {
                cache.discNum  = d;
                cache.trackNum = t;
                return index(cache.trackNum, col, discIndex);
            }
        }
    }

    return QModelIndex();
}

/************************************************

 ************************************************/
QModelIndex TrackViewModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    IndexData data(child);
    if (data.isTrack())
        return index(data.discNum(), 0, QModelIndex());

    return QModelIndex();
}

/************************************************

 ************************************************/
QVariant TrackViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    IndexData indexData(index);

    Track *track = indexData.track();
    if (track)
        return trackData(track, index, role);

    Disc *disc = indexData.disc();
    if (disc)
        return discData(disc, index, role);

    return QVariant();
}

/************************************************

 ************************************************/
bool TrackViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role != Qt::EditRole)
        return false;

    QList<Track *> tracks = view()->selectedTracks();
    foreach (Track *track, tracks) {
        switch (index.column()) {
            case TrackView::ColumnTitle:
                track->setTitle(value.toString());
                break;

            case TrackView::ColumnArtist:
                track->setArtist(value.toString());
                break;

            case TrackView::ColumnAlbum:
                track->setAlbum(value.toString());
                break;

            case TrackView::ColumnComment:
                track->setComment(value.toString());
                break;
        }
    }

    emit dataChanged(index, index, QVector<int>() << role);
    return true;
}

/************************************************

 ************************************************/
QVariant TrackViewModel::trackData(const Track *track, const QModelIndex &index, int role) const
{
    // Display & Edit :::::::::::::::::::::::::::::::::::
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case TrackView::ColumnTracknum:
                return QVariant(QStringLiteral("%1").arg(track->trackNum(), 2, 10, QChar('0')));

            case TrackView::ColumnDuration:
                return QVariant(trackDurationToString(track->duration()) + " ");

            case TrackView::ColumnTitle:
                return QVariant(track->title());

            case TrackView::ColumnArtist:
                return QVariant(track->artist());

            case TrackView::ColumnAlbum:
                return QVariant(track->album());

            case TrackView::ColumnComment:
                return QVariant(track->comment());

            case TrackView::ColumnFileName:
                return QVariant(Project::instance()->profile()->resultFileName(track));
        }

        return QVariant();
    }

    // ToolTip ::::::::::::::::::::::::::::::::::::::::::
    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case TrackView::ColumnFileName:
                return QVariant(Project::instance()->profile()->resultFilePath(track));

            default:
                return QVariant();
        }
    }

    // TextAlignmen :::::::::::::::::::::::::::::::::::::
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case TrackView::ColumnDuration:
                return int(Qt::AlignRight | Qt::AlignVCenter);
        }

        return QVariant();
    }

    switch (role) {
        case RoleItemType:
            return TrackItem;
        case RolePercent:
            return mCache->tracks[*track].percent;
        case RoleStatus:
            return int(mCache->tracks[*track].state);
        case RoleTracknum:
            return track->trackNum();
        case RoleDuration:
            return track->duration();
        case RoleTitle:
            return track->title();
        case RoleArtist:
            return track->tag(TagId::AlbumArtist);
        case RoleAlbum:
            return track->album();
        case RoleComment:
            return track->album();
        case RoleFileName:
            return Project::instance()->profile()->resultFileName(track);
        default:
            return QVariant();
    }

    return QVariant();
}

/************************************************

 ************************************************/
QVariant TrackViewModel::discData(const Disc *disc, const QModelIndex &index, int role) const
{
    // Display & Edit :::::::::::::::::::::::::::::::::::
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (!disc->count())
            return QVariant();

        QSet<QString> values;

        switch (index.column()) {
            case TrackView::ColumnTitle:
                for (int i = 0; i < disc->count(); ++i)
                    values << disc->track(i)->title();
                break;

            case TrackView::ColumnArtist:
                for (int i = 0; i < disc->count(); ++i)
                    values << disc->track(i)->tag(TagId::AlbumArtist);
                break;

            case TrackView::ColumnAlbum:
                for (int i = 0; i < disc->count(); ++i)
                    values << disc->track(i)->album();
                break;
        }

        if (values.count() > 1) {
            return QVariant(tr("Multiple values"));
        }
        else if (values.count() == 1) {
            return QVariant(*(values.begin()));
        }

        return QVariant();
    }

    // clang-format off
    switch (role) {
        case RoleItemType:      return DiscItem;
        case RoleTagSetTitle:   return disc->tagSetTitle();
        case RoleAudioFileName: return disc->audioFileNames();
        case RoleHasWarnings:   return Project::instance()->validator().diskHasWarnings(disc);
        case RoleHasErrors:     return Project::instance()->validator().diskHasErrors(disc);
        case RoleIsDownloads:   return mCache->downloadedDiscs.contains(index.row());
        case RoleCoverFile:     return disc->coverImageFile();
        case RoleCoverImg:      return disc->coverImagePreview();
        case RoleCueFilePath:   return disc->cueFilePath();
        case RoleAudioFilePath: return disc->audioFilePaths();
        case RoleDiscWarnings:  return Project::instance()->validator().diskWarnings(disc);
        case RoleDiscErrors:    return Project::instance()->validator().diskErrors(disc);
    }
    // clang-format on

    return QVariant();
}

/************************************************
 *
 ************************************************/
QString TrackViewModel::trackDurationToString(uint milliseconds) const
{
    if (milliseconds > 0) {
        uint l = milliseconds / 1000;
        uint h = l / 3600;
        uint m = (l % 3600) / 60;
        uint s = l % 60;

        if (h > 0)
            return tr("%1:%2:%3", "Track length, string like '01:02:56'").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
        else
            return tr("%1:%2", "Track length, string like '02:56'").arg(m).arg(s, 2, 10, QChar('0'));
    }
    else {
        return "??";
    }
}

/************************************************

 ************************************************/
int TrackViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return TrackView::ColumnCount;
}

/************************************************

 ************************************************/
int TrackViewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return Project::instance()->count();

    IndexData data(parent);
    if (data.isTrack())
        return 0;

    Disc *disc = data.disc();
    if (disc)
        return disc->count();

    return 0;
}

/************************************************

 ************************************************/
Qt::ItemFlags TrackViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (IndexData(index).isTrack()) {
        switch (index.column()) {
            case TrackView::ColumnTitle:
            case TrackView::ColumnArtist:
            case TrackView::ColumnAlbum:
            case TrackView::ColumnComment:
                res = res | Qt::ItemIsEditable;
                break;
        }
    }

    return res;
}

/************************************************

 ************************************************/
Disc *TrackViewModel::discByIndex(const QModelIndex &index)
{
    return IndexData(index).disc();
}

/************************************************

 ************************************************/
Track *TrackViewModel::trackByIndex(const QModelIndex &index)
{
    return IndexData(index).track();
}

/************************************************
 *
 ************************************************/
void TrackViewModel::downloadStarted(const Disc &disc)
{
    mCache->downloadedDiscs << index(disc).row();
    discDataChanged(&disc);
}

/************************************************
 *
 ************************************************/
void TrackViewModel::downloadFinished(const Disc &disc)
{
    mCache->downloadedDiscs.remove(index(disc).row());
    discDataChanged(&disc);
}

/************************************************
 *
 ************************************************/
void TrackViewModel::trackProgressChanged(const Track &track, TrackState state, Percent percent)
{
    CacheTrackData &cache = mCache->tracks[track];
    cache.state           = state;
    cache.percent         = percent;

    QModelIndex idx = index(track, TrackView::ColumnPercent);
    emit        dataChanged(idx, idx);
}

/************************************************

 ************************************************/
void TrackViewModel::discDataChanged(const Disc *disc)
{
    QModelIndex index1 = index(*disc, 0);
    QModelIndex index2 = index(*disc, TrackView::ColumnCount);
    emit        dataChanged(index1, index2);
}

/************************************************

 ************************************************/
void TrackViewModel::invalidateCache(const Disc *disc)
{
    mCache->downloadedDiscs.remove(index(*disc).row());

    for (int i = 0; i < disc->count(); ++i)
        mCache->tracks.remove(*(disc->track(i)));
}
