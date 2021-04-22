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
#include <QFile>
#include <QDir>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>

using namespace Conv;

/************************************************
 *
 ************************************************/
CopyCover::CopyCover(const CoverOptions &options, const QString &outDir, const QString &outBaseName) :
    mOptions(options),
    mDir(outDir),
    mBaseName(outBaseName)
{
}

/************************************************
 *
 ************************************************/
bool CopyCover::run()
{
    if (mOptions.fileName().isEmpty())
        return true;

    QFileInfo inFile(mOptions.fileName());

    QString outName = QDir(mDir).absoluteFilePath(QString("%1.%2").arg(mBaseName).arg(inFile.suffix()));

    // Keep original size, just copy file
    if (mOptions.size() == 0)
        return copyImage(outName);

    // Resize
    return resizeImage(outName);
}

/************************************************
 *
 ************************************************/
bool CopyCover::copyImage(const QString &outFileName)
{
    QFile f(mOptions.fileName());
    bool  res = f.copy(outFileName);

    if (!res)
        mErrorString = QObject::tr("I can't copy cover file <b>%1</b>:<br>%2").arg(outFileName, f.errorString());

    // clang-format off
    const QFileDevice::Permissions perm = QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                          QFileDevice::ReadUser  | QFileDevice::WriteUser  |
                                          QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                                          QFileDevice::ReadOther ;
    // clang-format on

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
    QImageReader reader(mOptions.fileName());
    QImage       img = reader.read();
    if (img.isNull()) {
        mErrorString = QObject::tr("I can't read cover image <b>%1</b>:<br>%2",
                                   "%1 - is a file name, %2 - an error text")
                               .arg(mOptions.fileName())
                               .arg(reader.errorString());

        return false;
    }

    if (img.width() < mOptions.size() && img.height() < mOptions.size())
        return copyImage(outFileName);

    img = img.scaled(QSize(mOptions.size(), mOptions.size()), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImageWriter writer(outFileName);
    if (!writer.write(img)) {
        mErrorString = QObject::tr("I can't save cover image <b>%1</b>:<br>%2",
                                   "%1 - is file name, %2 - an error text")
                               .arg(outFileName)
                               .arg(writer.errorString());

        return false;
    }

    return true;
}
