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


#include "format.h"

#include "flac.h"
#include <QDebug>
#include <QIODevice>
#include <QByteArray>
#include <QFile>

QList<const Format*> Format::mAllFormats;


/************************************************
 *
 ************************************************/
bool Format::registerFormat(const Format &f)
{
    // Some formats can be embedded as a chunk of RIFF stream.
    // So the WAV format should be last and be checked in the last turn.
    if (f.ext() == "wav")
        mAllFormats.append(&f);
    else
        mAllFormats.insert(0, &f);
    return true;
}


/************************************************
 *
 ************************************************/
Format::Format()
{
}


/************************************************
 *
 ************************************************/
Format::~Format()
{
}


/************************************************
 *
 ************************************************/
const FormatList &Format::allFormats()
{
    return mAllFormats;
}


/************************************************
 *
 ************************************************/
const FormatList &Format::inputFormats()
{
    static FormatList res;
    if (res.isEmpty())
    {
        foreach (const Format* f, allFormats())
        {
            if (f->isInputFormat())
                res << f;
        }
    }

    return res;
}


/************************************************
 *
 ************************************************/
const FormatList &Format::outFormats()
{
    static FormatList res;
    if (res.isEmpty())
    {
        foreach (const Format* f, allFormats())
        {
            if (f->isOutputFormat())
                res << f;
        }
    }

    return res;
}


/************************************************
 *
 ************************************************/
bool Format::checkMagic(const QByteArray &data) const
{
    return data.mid(magicOffset(), magic().length()) == magic();
}


/************************************************
 *
 ************************************************/
QString Format::filterDecoderStderr(const QString &stdErr) const
{
    return stdErr;
}


/************************************************
 *
 ************************************************/
const Format *Format::formatForFile(QIODevice *device)
{
    int bufSize = 0;
    foreach (const Format *format, allFormats())
        bufSize = qMax(bufSize, int(format->magicOffset() + format->magic().length()));

    QByteArray buf = device->read(bufSize);
    if (buf.size() < bufSize)
        return NULL;

    foreach (const Format *format, allFormats())
    {
        if (format->checkMagic(buf))
            return format;
    }

    return NULL;
}


/************************************************
 *
 ************************************************/
const Format *Format::formatForFile(const QString &fileName)
{
    QFile file(fileName);
    if (! file.open(QFile::ReadOnly))
    {
        return NULL;
    }

    const Format *res = formatForFile(&file);
    file.close();
    return res;
}

