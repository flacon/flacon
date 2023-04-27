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

#ifndef OUTFORMAT_H
#define OUTFORMAT_H

#include <QStringList>
#include <QHash>
#include <QVariant>
#include "track.h"
#include "types.h"
#include "extprogram.h"

namespace Conv {
class Encoder;
class Gain;
}
class EncoderConfigPage;
class Profile;

class MetadataWriter;

class OutFormat
{
public:
    static QList<OutFormat *> allFormats();
    static QStringList        allFormatsId();
    static OutFormat         *formatForId(const QString &id);
    virtual ~OutFormat() { }

    QString       id() const { return mId; }
    QString       name() const { return mName; }
    QString       ext() const { return mExt; }
    FormatOptions options() const { return mOptions; }

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    virtual BitsPerSample maxBitPerSample() const = 0;
    virtual SampleRate    maxSampleRate() const   = 0;

    virtual bool check(const Profile &profile, QStringList *errors) const;

    virtual QHash<QString, QVariant> defaultParameters() const         = 0;
    virtual EncoderConfigPage       *configPage(QWidget *parent) const = 0;

    virtual ExtProgram *encoderProgram(const Profile &profile) const { return nullptr; }
    virtual QStringList encoderArgs(const Profile &profile, const QString &outFile) const { return {}; }

    virtual Conv::Encoder  *createEncoder_OLD() const { return nullptr; }
    virtual MetadataWriter *createMetadataWriter(const QString &filePath) const = 0;

protected:
    QString       mId;
    QString       mName;
    QString       mExt;
    FormatOptions mOptions;
};

#endif // OUTFORMAT_H
