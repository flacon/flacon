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

#ifndef OUT_AAC_H
#define OUT_AAC_H

#include "../outformat.h"
#include "../encoderconfigpage.h"
#include "ui_out_aac_config.h"
#include "../converter/encoder.h"

class OutFormat_Aac : public OutFormat
{
public:
    OutFormat_Aac();

    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage       *configPage(QWidget *parentr) const override;

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_32; }
    virtual SampleRate    maxSampleRate() const override { return SampleRate::Hz_192000; }

    ExtProgram *encoderProgram(const Profile &profile) const override;
    QStringList encoderArgs(const Profile &profile, const QString &outFile) const override;

    MetadataWriter *createMetadataWriter(const Profile &profile, const QString &filePath) const override;
};

class ConfigPage_Acc : public EncoderConfigPage, private Ui::aacConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Acc(QWidget *parent = nullptr);

    virtual void load(const Profile &profile) override;
    virtual void save(Profile *profile) override;

private slots:
    void useQualityChecked(bool checked);
};

#endif // OUT_AAC_H
