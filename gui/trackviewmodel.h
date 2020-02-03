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


#ifndef TRACKVIEWMODEL_H
#define TRACKVIEWMODEL_H

#include <QAbstractItemModel>
#include <QSet>

#include "types.h"
class Project;
class Disk;
class Track;
class TrackView;


class TrackViewModel : public QAbstractItemModel
{
    friend class TrackView;
    friend class TrackViewDelegate;
    Q_OBJECT
public:
    explicit TrackViewModel(TrackView *parent = nullptr);
    virtual ~TrackViewModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    TrackView *view() const { return mView; }

    QModelIndex index(const Disk &disk, int col = 0) const;
    QModelIndex index(const Track &track, int col) const;

    Disk *diskByIndex(const QModelIndex &index);
    Track *trackByIndex(const QModelIndex &index);

public slots:

protected:
    enum ItemType {
        TrackItem = 1,
        DiskItem = 2
    };

    enum Roles{
        RoleItemType   = Qt::UserRole + 1,
        RolePercent,
        RoleStatus,
        RoleTracknum,
        RoleDuration,
        RoleTitle,
        RoleArtist,
        RoleAlbum,
        RoleComment,
        RoleFileName,
        RoleAudioFileName,
        RoleTagSetTitle,
        RoleHasWarnings,
        RoleCanConvert,
        RoleIsDownloads,
        RoleItemID,
        RoleTrack,
        RoleCoverFile,
        RoleCoverImg,
        RoleCueFilePath,
        RoleAudioFilePath,
        RoleDiskWarnings,
        RoleDiskErrors,
        RoleDiskPerformer
    };



public slots:
    void downloadStarted(const Disk &disk);
    void downloadFinished(const Disk &disk);

    void trackProgressChanged(const Track &track, TrackState state, Percent percent);

private slots:
    void diskDataChanged(const Disk *disk);
    void invalidateCache(const Disk *disk);

private:
    QVariant trackData(const Track *track, const QModelIndex &index, int role) const;
    QVariant diskData(const Disk *disk, const QModelIndex &index, int role) const;
    QString trackDurationToString(uint milliseconds) const;
    class Cache;
    mutable Cache *mCache;
    TrackView *mView;
};

#endif // TRACKVIEWMODEL_H
