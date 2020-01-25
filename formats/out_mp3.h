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


#ifndef OUT_MP3_H
#define OUT_MP3_H

#include "outformat.h"
#include "encoderconfigpage.h"
#include "ui_out_mp3_config.h"

class OutFormat_Mp3: public OutFormat
{
public:
    OutFormat_Mp3();

    virtual QString encoderProgramName() const override { return "lame"; }
    virtual QString gainProgramName() const override { return "mp3gain"; }

    virtual QStringList encoderArgs(const Profile &profile, const Track *track, const QString &outFile) const override;
    virtual QStringList gainArgs(const QStringList &files, const GainType gainType) const override;

    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage *configPage(Profile *profile, QWidget *parent) const override;

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_64; }
    virtual SampleRate    maxSampleRate()   const override { return SampleRate::Hz_768000; }
};


class ConfigPage_Mp3: public EncoderConfigPage, private Ui::mp3ConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Mp3(Profile *profile, QWidget *parent = nullptr);

    virtual void load() override;
    virtual void save() override;

private slots:
    void mp3PresetCbxCanged(int index);
};

#endif // OUT_MP3_H
