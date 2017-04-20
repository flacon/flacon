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

    virtual QString encoderProgramName() const { return ""; }
    virtual QString gainProgramName() const { return ""; }

    virtual QStringList encoderArgs(Track *track, const QString &outFile) const;
    virtual QStringList gainArgs(const QStringList &files) const;


    QHash<QString, QVariant> defaultParameters() const;
    EncoderConfigPage *configPage(QWidget *parent = 0) const;

    virtual bool hasConfigPage() const { return false; }

    virtual Encoder *createEncoder(Track *track, QObject *parent = 0) const;
    virtual Gain *createGain(Disk *disk, Track *track, QObject *parent = 0) const;
};


class Encoder_Wav: public Encoder
{
    Q_OBJECT
public:
    explicit Encoder_Wav(const OutFormat *format, Track *track, QObject *parent = 0);

protected:
    void doRun();
    virtual QStringList processArgs() const;
};

class Format_Wav: public AudioFormat
{
public:
    virtual QString name() const { return "WAV"; }
    virtual QString ext() const { return "wav"; }
    virtual bool isInputFormat() const { return true; }

    virtual QString decoderProgramName() const { return ""; }
    virtual QStringList decoderArgs(const QString &fileName) const;

    virtual QByteArray magic() const { return "RIFF"; }
    virtual uint const magicOffset() const { return 0; }
};

#endif // WAV_H
