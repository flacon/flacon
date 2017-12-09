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


#ifndef FORMAT_H
#define FORMAT_H

#include <QList>
#include <QStringList>
#include <QByteArray>
class QIODevice;

class AudioFormat;
typedef QList<const AudioFormat*> AudioFormatList;

class AudioFormat
{
public:
    AudioFormat();
    virtual ~AudioFormat();

    virtual QString name() const = 0;
    virtual QString ext() const = 0;

    virtual bool isInputFormat() const { return false; }

    virtual QString decoderProgramName() const { return ""; }
    virtual QStringList decoderArgs(const QString &fileName) const { Q_UNUSED(fileName); return QStringList(); }
    virtual QByteArray magic() const = 0;
    virtual uint magicOffset() const { return 0; }


    // Out format
    virtual bool isOutputFormat() const { return false; }

    static const AudioFormatList &allFormats();
    static const AudioFormatList &inputFormats();
    static const AudioFormatList &outFormats();

    static const AudioFormat *formatForFile(QIODevice *device);
    static const AudioFormat *formatForFile(const QString &fileName);

    static bool registerFormat(const AudioFormat &f);

    virtual QString filterDecoderStderr(const QString &stdErr) const;


protected:
    virtual bool checkMagic(const QByteArray &data) const;

};




#define REGISTER_FORMAT(FORMAT) \
    static FORMAT static_##FORMAT##_Instance; \
    static bool is_##FORMAT##_loaded = AudioFormat::registerFormat(static_##FORMAT##_Instance);

#endif // FORMAT_H
