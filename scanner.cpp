/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2015
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


#include "scanner.h"
#include "formats/informat.h"
#include "inputaudiofile.h"

#include "project.h"

#include <QStringList>
#include <QSet>
#include <QQueue>
#include <QDir>
#include <QApplication>


/************************************************

 ************************************************/
Scanner::Scanner(QObject *parent) :
    QObject(parent),
    mActive(false),
    mAbort(false)
{

}


/************************************************

 ************************************************/
DiscList Scanner::start(const QString &startDir)
{
    DiscList res;
    mActive = true;
    mAbort = false;

    QStringList exts;
    foreach(const InputFormat *format, InputFormat::allFormats()) {
        exts << QString("*.%1").arg(format->ext());
    }

    QQueue<QString> query;
    query << startDir;

    QSet<QString> processed;
    while (!query.isEmpty()) {
        QDir dir(query.dequeue());

        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        foreach(QFileInfo d, dirs) {
            qApp->processEvents();
            if (mAbort)
                return res;

            if (d.isSymLink())
                d = QFileInfo(d.symLinkTarget());

            if (!processed.contains(d.absoluteFilePath())) {
                processed << d.absoluteFilePath();
                query << d.absoluteFilePath();
            }
        }

        QFileInfoList files = dir.entryInfoList(exts, QDir::Files | QDir::Readable);
        foreach(QFileInfo f, files) {
            qApp->processEvents();
            if (mAbort)
                return res;

            try {
                res << project->addAudioFile(f.absoluteFilePath());
            }
            catch (FlaconError&) {
                // Silently skip corrupted files
                Q_UNUSED(startDir);
            }
        }
    }

    return res;
}


/************************************************

 ************************************************/
void Scanner::stop()
{
    mAbort = true;
}

