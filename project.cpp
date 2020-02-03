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
#include "settings.h"
#include "cue.h"
#include "inputaudiofile.h"

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDir>


/************************************************

 ************************************************/
void Project::clear()
{
    QList<Disc*> disks;
    for (int i=0; i<count(); ++i)
        disks << disk(i);

    removeDisk(&disks);
}


/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = nullptr;
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
Disc *Project::disk(int index) const
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
int Project::insertDisk(Disc *disk, int index)
{
    if (index < 0)
        index = mDisks.count();

    mDisks.insert(index, disk);

    emit layoutChanged();
    return index;
}


/************************************************

 ************************************************/
void Project::removeDisk(const QList<Disc*> *disks)
{
    for (int i=0; i<disks->count(); ++i)
    {
        Disc *disk = disks->at(i);
        emit beforeRemoveDisk(disk);
        if (mDisks.removeAll(disk))
            disk->deleteLater();

        emit afterRemoveDisk();
    }
}


/************************************************

 ************************************************/
int Project::indexOf(const Disc *disk) const
{
    return mDisks.indexOf(const_cast<Disc*>(disk));
}


/************************************************
 *
 ************************************************/
bool Project::diskExists(const QString &cueUri)
{
    foreach (const Disc *d, mDisks)
    {
        if (d->cueFile() == cueUri)
            return true;
    }
    return false;
}


/************************************************

 ************************************************/
Disc *Project::addAudioFile(const QString &fileName)
{

    QString canonicalFileName = QFileInfo(fileName).canonicalFilePath();

    for(int i=0; i<count(); ++i )
    {
        if (disk(i)->audioFileName() == canonicalFileName)
            return nullptr;
    }

    InputAudioFile audio(QFileInfo(fileName).absoluteFilePath());
    if (!audio.isValid())
    {
        throw FlaconError(audio.errorString());
    }

    Disc *disk = new Disc();
    disk->setAudioFile(audio);
    addDisk(disk);

    return disk;
}



/************************************************

 ************************************************/
DiscList Project::addCueFile(const QString &fileName)
{
    DiscList res;
    try
    {
        QVector<CueDisk> disks = CueReader().load(fileName);

        for (int i=0; i<disks.count(); ++i)
        {
            if (diskExists(disks.at(i).uri()))
                continue;

            Disc *disk = new Disc();
            disk->loadFromCue(disks.at(i));
            mDisks << disk;
            res << disk;
        }
        emit layoutChanged();
    }
    catch (FlaconError &err)
    {
        emit layoutChanged();
        qWarning() << err.what();
        throw err;
    }

    return res;
}


/************************************************

 ************************************************/
void Project::emitDiskChanged(Disc *disk) const
{
    emit diskChanged(disk);
}


/************************************************

 ************************************************/
void Project::emitLayoutChanged() const
{
    emit layoutChanged();
}
