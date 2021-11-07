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

#ifndef INFORMAT_H
#define INFORMAT_H

#include <QList>
#include <QStringList>
#include <QByteArray>

class QIODevice;

class InputFormat;
typedef QList<const InputFormat *> AudioFormatList;

class InputFormat
{
public:
    InputFormat();
    virtual ~InputFormat();

    virtual QString name() const = 0;
    virtual QString ext() const  = 0;

    virtual bool isInputFormat() const { return false; }

    virtual QString     decoderProgramName() const { return ""; }
    virtual QStringList decoderArgs(const QString &fileName) const
    {
        Q_UNUSED(fileName);
        return QStringList();
    }
    virtual QByteArray magic() const = 0;
    virtual uint       magicOffset() const { return 0; }

    static const AudioFormatList &allFormats();
    static const QStringList      allFileExts();

    static const InputFormat *formatForFile(QIODevice *device);
    static const InputFormat *formatForFile(const QString &fileName);

    static bool registerFormat(const InputFormat &f);

    virtual QString filterDecoderStderr(const QString &stdErr) const;

    virtual QByteArray readEmbeddedCue(const QString &fileName) const;

protected:
    virtual bool checkMagic(const QByteArray &data) const;
};

#define REGISTER_INPUT_FORMAT(FORMAT)         \
    static FORMAT static_##FORMAT##_Instance; \
    static bool   is_##FORMAT##_loaded = InputFormat::registerFormat(static_##FORMAT##_Instance);

#endif // INFORMAT_H
