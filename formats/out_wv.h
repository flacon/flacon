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

#ifndef OUT_WV_H
#define OUT_WV_H

#include "outformat.h"
#include "encoderconfigpage.h"
#include "ui_out_wv_config.h"

class OutFormat_Wv : public OutFormat
{
public:
    OutFormat_Wv();

    virtual QString encoderProgramName() const override { return "wavpack"; }
    virtual QString gainProgramName() const override { return "wvgain"; }

    virtual QStringList encoderArgs(const Profile &profile, const Track *track, const QString &coverFile, const QString &outFile) const override;
    virtual QStringList gainArgs(const QStringList &files, const GainType gainType) const override;

    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage *      configPage(const Profile &profile, QWidget *parent) const override;

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_32; }
    virtual SampleRate    maxSampleRate() const override { return SampleRate::Hz_768000; }
};

class ConfigPage_Wv : public EncoderConfigPage, private Ui::wvConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Wv(const Profile &profile, QWidget *parent = nullptr);

    virtual void load() override;
    virtual void save() override;
};

#endif // OUT_WV_H
