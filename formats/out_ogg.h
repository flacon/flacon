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


#ifndef OUT_OGG_H
#define OUT_OGG_H

#include "outformat.h"
#include "configdialog.h"
#include "ui_out_ogg_config.h"

class OutFormat_Ogg: public OutFormat
{
public:
    OutFormat_Ogg();

    virtual QString encoderProgramName() const override { return "oggenc"; }
    virtual QString gainProgramName() const override { return "vorbisgain"; }

    virtual QStringList encoderArgs(const Track *track, const QString &outFile) const override;
    virtual QStringList gainArgs(const QStringList &files) const override;

    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage *configPage(QWidget *parent = nullptr) const override;

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_24; }
    virtual SampleRate    maxSampleRate()   const override { return SampleRate::Hz_192000; }
};


class ConfigPage_Ogg: public EncoderConfigPage, private Ui::ConfigPage_Ogg
{
    Q_OBJECT
public:
    explicit ConfigPage_Ogg(QWidget *parent = nullptr);

    virtual void load() override;
    virtual void write() override;

private slots:
    void oggQualitySliderChanged(int value);
    void oggQualitySpinChanged(double value);
};

#endif // OUT_OGG_H
