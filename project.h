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


#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QList>
#include <QIcon>
#include "disk.h"

class Disk;
class Track;
class DataProvider;

class Project : public QObject
{
    Q_OBJECT
public:
    static Project* instance();
    
    Disk *disk(int index) const;
    int count() const;
    int indexOf(const Disk *disk) const;


    void addDisk(Disk *disk) { insertDisk(disk); }
    int insertDisk(Disk *disk, int index=-1);
    void removeDisk(const QList<Disk*> *disks);

    void emitDiskChanged(Disk *disk);
    void emitTrackChanged(int disk, int track);
    void emitTrackProgress(const Track *track);
    void emitLayoutChanged();
    void emitDownloadingStarted(DataProvider *provider) { emit downloadingStarted(provider); }
    void emitDownloadingFinished(DataProvider *provider) { emit downloadingFinished(provider);}

    static QIcon getIcon(const QString &iconName1, const QString &iconName2="", const QString &iconName3="", const QString &iconName4="");

    bool diskExists(const QString &cueUri);

    static void error(const QString &msg);
    static void installErrorHandler(void (*handler)(const QString &msg));

public slots:
    void clear();
    Disk *addAudioFile(const QString &fileName, bool showErrors);
    DiskList addCueFile(const QString &fileName, bool showErrors);

signals:
    void diskChanged(Disk *disk);
    void trackChanged(int disk, int track);
    void trackProgress(const Track *track);
    void layoutChanged();
    void beforeRemoveDisk(Disk *disk);
    void afterRemoveDisk();
    void downloadingStarted(DataProvider *provider);
    void downloadingFinished(DataProvider *provider);

private:
    explicit Project(QObject *parent = 0);

    QList<Disk*> mDisks;
};

#define project Project::instance()

#endif // PROJECT_H
