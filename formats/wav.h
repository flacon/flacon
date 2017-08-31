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


#ifndef WAV_H
#define WAV_H

#include "outformat.h"
#include "format.h"
#include "encoder.h"

class OutFormat_Wav: public OutFormat
{
public:
    OutFormat_Wav();

    virtual QString encoderProgramName() const override { return ""; }
    virtual QString gainProgramName() const override { return ""; }

    virtual QStringList encoderArgs(const Track *track, const QString &outFile) const override;
    virtual QStringList gainArgs(const QStringList &files) const override;


    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage *configPage(QWidget *parent = 0) const override;

    virtual bool hasConfigPage() const override { return false; }

};


class Format_Wav: public AudioFormat
{
public:
    virtual QString name() const override { return "WAV"; }
    virtual QString ext() const override { return "wav"; }
    virtual bool isInputFormat() const override { return true; }

    virtual QString decoderProgramName() const override { return ""; }
    virtual QStringList decoderArgs(const QString &fileName) const override;

    virtual QByteArray magic() const override { return "RIFF"; }
    virtual uint const magicOffset() const override { return 0; }
};

#endif // WAV_H
