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
#include <QMetaEnum>

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
    QSet<DiscNum> downloadedDiscs;

    CacheTrackData get(const Track &track)
    {
        return mTracks.value(QPair<Disc *, int>(track.disc(), track.index()));
    }

    void set(const Track &track, const CacheTrackData &data)
    {
        mTracks[QPair<Disc *, int>(track.disc(), track.index())] = data;
    }

    void remove(const Track &track)
    {
        mTracks.remove(QPair<Disc *, int>(track.disc(), track.index()));
    }

private:
    QHash<QPair<Disk *, int>, CacheTrackData> mTracks;
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
        if (mDiscId && mDiscId - 1 < Project::instance()->count()) {
            return Project::instance()->disc(mDiscId - 1);
        }

        return nullptr;
    }

    Track *track() const
    {
        if (mTrackId) {
            Disc *disc = this->disc();
            if (disc && mTrackId - 1 < disc->tracks().count()) {
                return disc->tracks().at(mTrackId - 1);
            }
        }
        return nullptr;
    }

private:
    quint16 mDiscId;
    quint16 mTrackId;
};

/************************************************

 ************************************************/
TrackViewModel::TrackViewModel(TrackView *parent) :
    QAbstractItemModel(parent),
    mCache(new Cache()),
    mView(parent)
{
    connect(Project::instance(), &Project::discChanged,
            this, &TrackViewModel::discDataChanged);

    connect(Project::instance(), &Project::layoutChanged, this,
            [this]() { emit this->layoutChanged(); });

    connect(Project::instance(), &Project::afterRemoveDisc, this,
            [this]() { emit this->layoutChanged(); });

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

    // clang-format off
    switch (TrackView::Columns(section)) {
        case TrackView::ColumnTracknum:   return tr("Track", "Table header.");
        case TrackView::ColumnDuration:   return tr("Length", "Table header.");
        case TrackView::ColumnTitle:      return tr("Title", "Table header.");
        case TrackView::ColumnArtist:     return tr("Artist", "Table header.");
        case TrackView::ColumnAlbum:      return tr("Album", "Table header.");
        case TrackView::ColumnComment:    return tr("Comment", "Table header.");
        case TrackView::ColumnFileName:   return tr("File", "Table header.");
        case TrackView::ColumnDate:       return tr("Date", "Table header.");
        case TrackView::ColumnGenre:      return tr("Genre", "Table header.");
        case TrackView::ColumnSongWriter: return tr("Song writer", "Table header.");
        case TrackView::ColumnCatalog:    return tr("Catalog number", "Table header.");
    }
    // clang-format on

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
    QModelIndex parentIndex = index(*(track.disc()));
    if (parentIndex.isValid()) {
        return index(track.index(), col, parentIndex);
    }
    return QModelIndex();

    // CacheTrackData &cache = mCache->tracks[track];

    // const IndexData indexData(cache.discNum, cache.trackNum);
    // if (indexData.track() && *indexData.track() == track) {
    //     return index(cache.trackNum, col, index(cache.discNum, 0));
    // }

    // // Update cache
    // for (int d = 0; d < rowCount(); ++d) {
    //     QModelIndex discIndex = index(d, 0);
    //     for (int t = 0; t < rowCount(discIndex); ++t) {
    //         IndexData indexData(d, t);
    //         if (indexData.track() && *indexData.track()->index() == track.index()) {
    //             cache.discNum  = d;
    //             cache.trackNum = t;
    //             return index(cache.trackNum, col, discIndex);
    //         }
    //     }
    // }

    // return QModelIndex();
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
        // clang-format off
        switch (TrackView::Columns(index.column())) {
            case TrackView::ColumnTitle:        track->setTitleTag(value.toString());           break;
            case TrackView::ColumnArtist:       track->setPerformerTag(value.toString());       break;
            case TrackView::ColumnAlbum:        track->disk()->setAlbumTag(value.toString());   break;
            case TrackView::ColumnComment:      track->setCommentTag(value.toString());         break;
            case TrackView::ColumnDate:         track->setDateTag(value.toString());            break;
            case TrackView::ColumnGenre:        track->setGenreTag(value.toString());           break;
            case TrackView::ColumnSongWriter:   track->setSongWriterTag(value.toString());      break;
            case TrackView::ColumnCatalog:      track->disk()->setCatalogTag(value.toString()); break;

            // Read only columns
            case TrackView::ColumnTracknum: break;
            case TrackView::ColumnDuration: break;
            case TrackView::ColumnFileName: break;
        }
        // clang-format on
    }

    emit dataChanged(index, index, QVector<int>() << role);
    return true;
}

/************************************************

 ************************************************/
QVariant TrackViewModel::trackData(const Track *track, const QModelIndex &index, int role) const
{
    // Text roles :::::::::::::::::::::::::::::::
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == TrackViewModel::RolePlaceHolder) {
        // clang-format off
        switch (TrackView::Columns(index.column())) {
            case TrackView::ColumnTracknum:     return QVariant(QStringLiteral("%1").arg(track->trackNumTag(), 2, 10, QChar('0')));
            case TrackView::ColumnDuration:     return QVariant(trackDurationToString(track->duration()) + " ");
            case TrackView::ColumnTitle:        return QVariant(track->titleTag());
            case TrackView::ColumnArtist:       return track->artistTag();
            case TrackView::ColumnAlbum:        return track->disc()->albumTag();
            case TrackView::ColumnComment:      return QVariant(track->commentTag());
            case TrackView::ColumnFileName:     return QVariant(Project::instance()->profile()->resultFileName(track));
            case TrackView::ColumnDate:         return track->dateTag();
            case TrackView::ColumnGenre:        return track->genreTag();
            case TrackView::ColumnSongWriter:   return track->songWriterTag();
            case TrackView::ColumnCatalog:      return track->disc()->catalogTag();
        }
        // clang-format on

        return QVariant();
    }

    // ToolTip ::::::::::::::::::::::::::::::::::
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

    // clang-format off
    switch (role) {
        case RoleItemType:  return TrackItem;
        case RolePercent:   return mCache->get(*track).percent;
        case RoleStatus:    return int(mCache->get(*track).state);
//        case RoleTracknum:  return track->trackNumTag();
//        case RoleDuration:  return track->duration();
//        case RoleTitle:     return track->titleTag();
//        case RoleArtist:    return track->disc()->albumTag();
//        case RoleAlbum:     return track->albumTag();
//        case RoleComment:   return track->commentTag();
//        case RoleFileName:  return Project::instance()->profile()->resultFileName(track);
        default:            return QVariant();
    }
    // clang-format on

    return QVariant();
}

/************************************************

 ************************************************/
QVariant TrackViewModel::discData(const Disc *disc, const QModelIndex &index, int role) const
{
    // Display & Edit :::::::::::::::::::::::::::::::::::
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (!disc->tracks().count())
            return QVariant();

        QSet<QString> values;

        switch (index.column()) {
            case TrackView::ColumnTitle:
                for (int i = 0; i < disc->tracks().count(); ++i)
                    values << disc->tracks().at(i)->titleTag();
                break;

            case TrackView::ColumnArtist:
                for (int i = 0; i < disc->tracks().count(); ++i)
                    values << disc->albumTag();
                break;

            case TrackView::ColumnAlbum:
                values << disc->albumTag();
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
        case RoleTagSetTitle:   return disc->currentTagSet().title;
        case RoleAudioFileName: return disc->audioFileNames();
        case RoleHasWarnings:   return Project::instance()->validator().diskHasWarnings(disc);
        case RoleHasErrors:     return Project::instance()->validator().diskHasErrors(disc);
        case RoleIsDownloads:   return mCache->downloadedDiscs.contains(index.row());
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
int TrackViewModel::columnCount(const QModelIndex &) const
{
    static int columnCount = QMetaEnum::fromType<TrackView::Columns>().keyCount();
    return columnCount;
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
        return disc->tracks().count();

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
        switch (TrackView::Columns(index.column())) {
            case TrackView::ColumnTitle:
            case TrackView::ColumnArtist:
            case TrackView::ColumnAlbum:
            case TrackView::ColumnComment:
            case TrackView::ColumnDate:
            case TrackView::ColumnGenre:
            case TrackView::ColumnSongWriter:
            case TrackView::ColumnCatalog:
                res = res | Qt::ItemIsEditable;
                break;

            case TrackView::ColumnTracknum:
            case TrackView::ColumnDuration:
            case TrackView::ColumnFileName:
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

/**************************************
 *
 **************************************/
bool TrackViewModel::downloading() const
{
    return !mCache->downloadedDiscs.isEmpty();
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
    CacheTrackData cache = mCache->get(track);
    cache.state          = state;
    cache.percent        = percent;
    mCache->set(track, cache);

    QModelIndex idx = index(track, TrackView::ColumnTracknum);
    emit        dataChanged(idx, idx);
}

/************************************************

 ************************************************/
void TrackViewModel::discDataChanged(const Disc *disc)
{
    static int columnCount = QMetaEnum::fromType<TrackView::Columns>().keyCount();

    QModelIndex index1 = index(*disc, 0);
    QModelIndex index2 = index(*disc, columnCount);
    emit        dataChanged(index1, index2);
}

/************************************************

 ************************************************/
void TrackViewModel::invalidateCache(const Disc *disc)
{
    mCache->downloadedDiscs.remove(index(*disc).row());

    for (const Track *track : disc->tracks()) {
        mCache->remove(*(track));
    }
}
