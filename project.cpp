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


#include "project.h"
//#include "disk.h"
#include "settings.h"
#include "cue.h"
#include "inputaudiofile.h"

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDir>

static void (*errorHandler)(const QString &msg);

/************************************************

 ************************************************/
QIcon Project::getIcon(const QString &iconName1, const QString &iconName2, const QString &iconName3, const QString &iconName4)
{
    if (QIcon::themeName() == "hicolor")
    {
        QStringList failback;
        failback << "oxygen";
        failback << "Tango";
        failback << "Prudence-icon";
        failback << "Humanity";
        failback << "elementary";
        failback << "gnome";


        QDir usrDir("/usr/share/icons/");
        QDir usrLocalDir("/usr/local/share/icons/");
        foreach (QString s, failback)
        {
            if (usrDir.exists(s) || usrLocalDir.exists(s))
            {
                QIcon::setThemeName(s);
                break;
            }
        }
    }

    QStringList icons;
    icons << iconName1;
    icons << iconName2;
    icons << iconName3;
    icons << iconName4;

    QIcon res;
    foreach(const QString &icon, icons)
    {
        if (icon.startsWith(':'))
            res = QIcon(icon);
        else
            res = QIcon::fromTheme(icon);

        if (!res.isNull())
            return res;
    }

    return res;
}


/************************************************

 ************************************************/
void Project::clear()
{
    QList<Disk*> disks;
    for (int i=0; i<count(); ++i)
        disks << disk(i);

    removeDisk(&disks);
}


/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = 0;
    if (!inst)
        inst = new Project();

    return inst;
}


/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent)
{
}


/************************************************

 ************************************************/
Disk *Project::disk(int index) const
{
    return mDisks.at(index);
}


/************************************************

 ************************************************/
int Project::count() const
{
    return mDisks.count();
}


/************************************************

 ************************************************/
int Project::insertDisk(Disk *disk, int index)
{
    if (index < 0)
        index = mDisks.count();

    mDisks.insert(index, disk);

    //disc.setProject(self)
    //self.connect(disc, SIGNAL("downloadStarted()"), self._discDownlaodStarted)
    //self.connect(disc, SIGNAL("downloadFinished()"), self._discDownlaodFinished)
    emit layoutChanged();

    return index;
}


/************************************************

 ************************************************/
void Project::removeDisk(const QList<Disk*> *disks)
{
    for (int i=0; i<disks->count(); ++i)
    {
        Disk *disk = disks->at(i);
        emit beforeRemoveDisk(disk);
        if (mDisks.removeAll(disk))
            disk->deleteLater();

        emit afterRemoveDisk();
    }

    //emit layoutChanged();
}


/************************************************

 ************************************************/
int Project::indexOf(const Disk *disk) const
{
    return mDisks.indexOf(const_cast<Disk*>(disk));
}


/************************************************
 *
 ************************************************/
bool Project::diskExists(const QString &cueUri)
{
    foreach (Disk *d, mDisks)
    {
        if (!d->tagSets().isEmpty() && d->tagSets().first()->uri() == cueUri)
            return true;
    }
    return false;
}


/************************************************
 *
 ************************************************/
void Project::error(const QString &msg)
{
    if (errorHandler)
        errorHandler(msg);
}


/************************************************
 *
 ************************************************/
void Project::installErrorHandler(void (*handler)(const QString &))
{
    errorHandler = handler;
}


/************************************************

 ************************************************/
Disk *Project::addAudioFile(const QString &fileName, bool showErrors)
{

    QString canonicalFileName = QFileInfo(fileName).canonicalFilePath();

    for(int i=0; i<count(); ++i )
    {
        if (disk(i)->audioFileName() == canonicalFileName)
            return 0;
    }

    InputAudioFile audio(canonicalFileName);
    if (!audio.isValid())
    {
        if (showErrors)
            Project::error(audio.errorString());

        return 0;
    }

    Disk *disk = new Disk();
    disk->setAudioFile(audio);
    addDisk(disk);

    return disk;
}



/************************************************

 ************************************************/
DiskList Project::addCueFile(const QString &fileName, bool showErrors)
{
    DiskList res;
    CueReader cueReader(fileName);
    if (cueReader.isValid())
    {
        for (int i=0; i<cueReader.diskCount(); ++i)
        {
            if (diskExists(cueReader.disk(i).uri()))
                continue;

            Disk *disk = new Disk();
            disk->loadFromCue(cueReader.disk(i));
            mDisks << disk;
            res << disk;
        }
        emit layoutChanged();
    }
    else
    {
        foreach(Disk *disk, res)
        {
            mDisks.removeAll(disk);
            disk->deleteLater();
        }

        emit layoutChanged();
        if (showErrors)
            Project::error(cueReader.errorString());
    }

    return res;
}


/************************************************

 ************************************************/
void Project::emitDiskChanged(Disk *disk) const
{
    emit diskChanged(disk);
}


/************************************************

 ************************************************/
void Project::emitTrackChanged(int disk, int track) const
{
    emit trackChanged(disk, track);
}


/************************************************

 ************************************************/
void Project::emitTrackProgress(const Track *track) const
{
    emit trackProgress(track);
}


/************************************************

 ************************************************/
void Project::emitLayoutChanged() const
{
    emit layoutChanged();
}


/************************************************
 *
 ************************************************/
void Project::emitDownloadingStarted(DataProvider *provider) const
{
    emit downloadingStarted(provider);
}


/************************************************
 *
 ************************************************/
void Project::emitDownloadingFinished(DataProvider *provider) const
{
    emit downloadingFinished(provider);
}
