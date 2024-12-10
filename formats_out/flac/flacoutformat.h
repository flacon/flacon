/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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

#ifndef FLACOITFORMAT_H
#define FLACOITFORMAT_H

#include "../outformat.h"
#include "../encoderconfigpage.h"
#include "ui_out_flac_config.h"

class OutFormat_Flac : public OutFormat
{
public:
    OutFormat_Flac();
    bool check(const Profile &profile, QStringList *errors) const override;

    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage       *configPage(QWidget *parent) const override;

    // See https://en.wikipedia.org/wiki/Comparison_of_audio_coding_formats for details
    BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_24; }
    SampleRate    maxSampleRate() const override { return SampleRate::Hz_768000; }

    MetadataWriter *createMetadataWriter(const Profile &profile, const QString &filePath) const override;

    ExtProgram *encoderProgram(const Profile &profile) const override;
    QStringList encoderArgs(const Profile &profile, const QString &outFile) const override;
};

class ConfigPage_Flac : public EncoderConfigPage, private Ui::flacConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Flac(QWidget *parent = nullptr);

    virtual void load(const Profile &profile) override;
    virtual void save(Profile *profile) override;
};

#endif // FLACOITFORMAT_H
