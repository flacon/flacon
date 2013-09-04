/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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


#include "converterthread.h"
#include <QFile>
#include <QTextStream>


/************************************************

 ************************************************/
ConverterThread::ConverterThread(Disk *disk, QObject *parent):
    QThread(parent),
    mDisk(disk)
{
}


/************************************************

 ************************************************/
ConverterThread::~ConverterThread()
{
}


/************************************************

 ************************************************/
void ConverterThread::run()
{
    doRun();
}


/************************************************

 ************************************************/
void ConverterThread::error(Track *track, const QString &message)
{
    stop();
    emit trackError(track, message);
}


/************************************************

 ************************************************/
void ConverterThread::stop()
{
    doStop();
    for (int i=0; i<disk()->count(); ++i)
    {
        Track *track = disk()->track(i);

        switch (track->status())
        {
        case Track::Error:
        case Track::OK:
            break;

        case Track::NotRunning:
            emit trackProgress(track, Track::Canceled);
            break;

        default:
            emit trackProgress(track, Track::Aborted);
            break;
        }
    }
}


/************************************************

 ************************************************/
bool ConverterThread::deleteFile(const QString &fileName)
{
    QFile f(fileName);
    if (f.exists())
        return f.remove();
    else
        return true;
}


/************************************************

 ************************************************/
void ConverterThread::debugArguments(const QStringList &args)
{
    QTextStream out(stderr);
    foreach (QString arg, args)
    {
        if (arg.contains(' ') || arg.contains('\t'))
        {
            out << "'" << arg << "' ";
        }
        else
        {
            out << arg << " ";
        }

    }
    out << endl;
}
