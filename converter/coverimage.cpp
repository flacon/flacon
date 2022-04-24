/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include "coverimage.h"
#include <QObject>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include "types.h"
#include <QDebug>

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(LOG, "CoverImage")
}

/*******************************************
 https://doc.qt.io/qt-5/qimagereader.html#supportedImageFormats
 *******************************************/
static QString formatToMimeType(const QByteArray &format)
{
    QByteArray fmt = format.toUpper();
    // clang-format off
    if (fmt =="BMP") return "image/bmp";                // Windows Bitmap
    if (fmt =="GIF") return "image/gif";                // Graphic Interchange Format (optional)
    if (fmt =="JPG") return "image/jpeg";               // Joint Photographic Experts Group
    if (fmt =="PNG") return "image/png";                // Portable Network Graphics
    if (fmt =="PBM") return "image/x-portable-bitmap";  // Portable Bitmap
    if (fmt =="PGM") return "image/x-portable-graymap"; // Portable Graymap
    if (fmt =="PPM") return "image/x-portable-pixmap";  // Portable Pixmap
    if (fmt =="XBM") return "image/x-xbitmap";          // X11 Bitmap
    if (fmt =="XPM") return "image/x-xpixmap";          // X11 Pixmap
    if (fmt =="SVG") return "image/svg+xml";            // Scalable Vector Graphics
    // clang-format on

    return "";
}

static CoverImage::Format formatStrToFormat(const QByteArray &format)
{
    QByteArray fmt = format.toUpper();
    // clang-format off
    if (fmt =="BMP") return CoverImage::Format::BMP;    // Windows Bitmap
    if (fmt =="GIF") return CoverImage::Format::GIF;    // Graphic Interchange Format (optional)
    if (fmt =="JPG") return CoverImage::Format::JPG;    // Joint Photographic Experts Group
    if (fmt =="PNG") return CoverImage::Format::PNG;    // Portable Network Graphics
    if (fmt =="PBM") return CoverImage::Format::PBM;    // Portable Bitmap
    if (fmt =="PGM") return CoverImage::Format::PGM;    // Portable Graymap
    if (fmt =="PPM") return CoverImage::Format::PPM;    // Portable Pixmap
    if (fmt =="XBM") return CoverImage::Format::XBM;    // X11 Bitmap
    if (fmt =="XPM") return CoverImage::Format::XPM;    // X11 Pixmap
    if (fmt =="SVG") return CoverImage::Format::SVG;    // Scalable Vector Graphics
    // clang-format on

    return CoverImage::Format::Unknown;
}

CoverImage::CoverImage(const QString &inFilePath, uint size)
{

    try {
        if (inFilePath.isEmpty()) {
            qCCritical(LOG) << "Input file name is empty";
            throw QObject::tr("file name is empty", "error message text");
        }

        QImageReader reader(inFilePath);
        QByteArray   format = reader.format();
        mFormat             = formatStrToFormat(format);
        mMimeType           = formatToMimeType(format);

        QImage image = reader.read();
        if (image.isNull()) {
            qCCritical(LOG) << "Can't read cover file" << inFilePath << ":" << reader.errorString();
            throw reader.errorString();
        }

        if (size > 0) {
            if (image.width() > int(size) || image.height() > int(size)) {
                image.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }

        mSize  = image.size();
        mDepth = image.depth();

        QBuffer out(&mData);
        out.open(QIODevice::WriteOnly);
        QImageWriter writer(&out, format);
        if (!writer.write(image)) {
            qCCritical(LOG) << "Can't write cover file to memory" << inFilePath << ":" << writer.errorString();
            throw writer.errorString();
        }
    }
    catch (const QString &err) {
        throw FlaconError(QObject::tr(
                                  "I can't read cover image <b>%1</b>:<br>%2",
                                  "%1 - is a file name, %2 - an error text")
                                  .arg(inFilePath, err));
    }
}

void CoverImage::saveAs(const QString &filePath) const
{
    QFile f(filePath);
    if (!f.open(QFile::WriteOnly)) {
        qCCritical(LOG) << "Can't write cover file to" << filePath << ":" << f.errorString();
        throw FlaconError(QObject::tr("I can't save cover image <b>%1</b>:<br>%2",
                                      "%1 - is file name, %2 - an error text")
                                  .arg(filePath, f.errorString()));
    }
    f.write(mData);
    f.close();
}
