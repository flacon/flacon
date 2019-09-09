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

/************************************************
 *
 ************************************************/
AudioFormatList &formatList()
{
    static AudioFormatList *afl = new AudioFormatList();
    return *afl;
}


/************************************************
 *
 ************************************************/
bool AudioFormat::registerFormat(const AudioFormat &f)
{
    // Some formats can be embedded as a chunk of RIFF stream.
    // So the WAV format should be last and be checked in the last turn.
    if (f.ext() == "wav")
        formatList().append(&f);
    else
        formatList().insert(0, &f);
    return true;
}


/************************************************
 *
 ************************************************/
AudioFormat::AudioFormat()
{
}


/************************************************
 *
 ************************************************/
AudioFormat::~AudioFormat()
{
}


/************************************************
 *
 ************************************************/
const AudioFormatList &AudioFormat::allFormats()
{
    return formatList();
}


/************************************************
 *
 ************************************************/
const AudioFormatList &AudioFormat::inputFormats()
{
    static AudioFormatList res;
    if (res.isEmpty())
    {
        foreach (const AudioFormat* f, allFormats())
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
const AudioFormatList &AudioFormat::outFormats()
{
    static AudioFormatList res;
    if (res.isEmpty())
    {
        foreach (const AudioFormat* f, allFormats())
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
bool AudioFormat::checkMagic(const QByteArray &data) const
{
    return data.mid(magicOffset(), magic().length()) == magic();
}


/************************************************
 *
 ************************************************/
QString AudioFormat::filterDecoderStderr(const QString &stdErr) const
{
    return stdErr;
}


/************************************************
 *
 ************************************************/
const AudioFormat *AudioFormat::formatForFile(QIODevice *device)
{
    int bufSize = 0;
    foreach (const AudioFormat *format, allFormats())
        bufSize = qMax(bufSize, int(format->magicOffset() + format->magic().length()));

    QByteArray buf = device->read(bufSize);
    if (buf.size() < bufSize)
        return nullptr;

    foreach (const AudioFormat *format, allFormats())
    {
        if (format->checkMagic(buf))
            return format;
    }

    return nullptr;
}


/************************************************
 *
 ************************************************/
const AudioFormat *AudioFormat::formatForFile(const QString &fileName)
{
    QFile file(fileName);
    if (! file.open(QFile::ReadOnly))
    {
        return nullptr;
    }

    const AudioFormat *res = formatForFile(&file);
    file.close();
    return res;
}

