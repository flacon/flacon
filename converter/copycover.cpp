/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "copycover.h"
#include "disc.h"
#include <QFile>
#include <QDir>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>


/************************************************
 *
 ************************************************/
CopyCover::CopyCover(const Disc *disk, const QString &outDir, const QString &outBaseName, int newSize):
    mDisk(disk),
    mSize(newSize),
    mDir(outDir),
    mBaseName(outBaseName)
{
}


/************************************************
 *
 ************************************************/
bool CopyCover::run()
{
    if (mDisk->coverImageFile().isEmpty())
        return true;

    QFileInfo inFile(mDisk->coverImageFile());

    QString outName = QDir(mDir).absoluteFilePath(QString("%1.%2").arg(mBaseName).arg(inFile.suffix()));

    // Keep original size, just copy file
    if (mSize == 0)
        return copyImage(outName);


    // Resize
    return resizeImage(outName);
}


/************************************************
 *
 ************************************************/
bool CopyCover::copyImage(const QString &outFileName)
{
    QFile f(mDisk->coverImageFile());
    bool res = f.copy(outFileName);

    if (!res)
        mErrorString = QObject::tr("I can't copy cover file <b>%1</b>:<br>%2").arg(outFileName, f.errorString());

    const QFileDevice::Permissions perm = QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                          QFileDevice::ReadUser  | QFileDevice::WriteUser  |
                                          QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                                          QFileDevice::ReadOther ;

    res = QFile::setPermissions(outFileName, perm);
    if (!res)
        mErrorString = QObject::tr("I can't copy cover file <b>%1</b>:<br>%2").arg(outFileName, f.errorString());

    return res;
}


/************************************************
 *
 ************************************************/
bool CopyCover::resizeImage(const QString &outFileName)
{
    QImageReader reader(mDisk->coverImageFile());
    QImage img = reader.read();
    if (img.isNull())
    {
        mErrorString = QObject::tr("I can't read cover image <b>%1</b>:<br>%2",
                                   "%1 - is a file name, %2 - an error text")
                .arg(mDisk->coverImageFile())
                .arg(reader.errorString());

        return false;
    }

    if (img.width() < mSize && img.height() < mSize )
        return copyImage(outFileName);

    img = img.scaled(QSize(mSize, mSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImageWriter writer(outFileName);
    if (!writer.write(img))
    {
        mErrorString = QObject::tr("I can't save cover image <b>%1</b>:<br>%2",
                                   "%1 - is file name, %2 - an error text")
                .arg(outFileName)
                .arg(writer.errorString());

        return false;
    }

    return true;
}
