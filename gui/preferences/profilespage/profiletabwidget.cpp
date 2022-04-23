/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#include "profiletabwidget.h"
#include "ui_profiletabwidget.h"
#include "controls.h"
#include "../preferencesdialog.h"

class NoEncoderConfigPage : public EncoderConfigPage
{
public:
    using EncoderConfigPage::EncoderConfigPage;
    void load(const Profile &) override {};
    void save(Profile *) override {};
};

/************************************************
 *
 ************************************************/
ProfileTabWidget::ProfileTabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::ProfileTabWidget)
{
    ui->setupUi(this);
    setCurrentIndex(0);

    ui->outDirEdit->setPlaceholderText(tr("Same directory as CUE file", "Placeholder for output direcory combobox"));
    ui->outDirButton->setBuddy(ui->outDirEdit);

    ui->outPatternButton->setBuddy(ui->outPatternEdit);
    ui->outPatternButton->addStandardPatterns();

    ui->encoderGroup->setLayout(new QVBoxLayout(ui->encoderGroup));

    ui->bitDepthComboBox->addItem(tr("Same as source", "Item in combobox"), int(BitsPerSample::AsSourcee));
    ui->bitDepthComboBox->addItem(tr("16-bit", "Item in combobox"), int(BitsPerSample::Bit_16));
    ui->bitDepthComboBox->addItem(tr("24-bit", "Item in combobox"), int(BitsPerSample::Bit_24));
    ui->bitDepthComboBox->addItem(tr("32-bit", "Item in combobox"), int(BitsPerSample::Bit_32));

    ui->sampleRateComboBox->addItem(tr("Same as source", "Item in combobox"), SampleRate::AsSource);
    ui->sampleRateComboBox->addItem(tr("44100 Hz", "Item in combobox"), SampleRate::Hz_44100);
    ui->sampleRateComboBox->addItem(tr("48000 Hz", "Item in combobox"), SampleRate::Hz_48000);
    ui->sampleRateComboBox->addItem(tr("96000 Hz", "Item in combobox"), SampleRate::Hz_96000);
    ui->sampleRateComboBox->addItem(tr("192000 Hz", "Item in combobox"), SampleRate::Hz_192000);

    ui->gainComboBox->clear();
    ui->gainComboBox->addItem(tr("Disabled", "ReplayGain type combobox"), GainType::Disable);
    ui->gainComboBox->addItem(tr("Per Track", "ReplayGain type combobox"), GainType::Track);
    ui->gainComboBox->addItem(tr("Per Album", "ReplayGain type combobox"), GainType::Album);
    ui->gainComboBox->setToolTip(tr("ReplayGain is a standard to normalize the perceived loudness of computer audio formats. \n\n"
                                    "The analysis can be performed on individual tracks, so that all tracks will be of equal volume on playback. \n"
                                    "Using the album-gain analysis will preserve the volume differences within an album."));
}

/************************************************
 *
 ************************************************/
ProfileTabWidget::~ProfileTabWidget()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
void ProfileTabWidget::fromProfile(const Profile &profile)
{
    // Create EncoderWidget ................
    recreateEncoderWidget(profile);
    mEncoderWidget->load(profile);

    // Result files options ................
    ui->outDirEdit->setText(profile.outFileDir());
    ui->outPatternEdit->setText(profile.outFilePattern());

    // Resample options ....................
    ui->resampleGroup->setVisible(profile.formatOptions().testFlag(FormatOption::Lossless));
    ui->resampleGroupLine->setVisible(ui->resampleGroup->isVisible());

    if (profile.formatOptions().testFlag(FormatOption::Lossless)) {
        ui->bitDepthComboBox->setValue(profile.bitsPerSample());
        ui->sampleRateComboBox->setValue(profile.sampleRate());
    }

    // Replay Gain options ................
    ui->gainGroup->setVisible(profile.formatOptions().testFlag(FormatOption::SupportGain));
    if (profile.formatOptions().testFlag(FormatOption::SupportGain)) {
        ui->gainComboBox->setValue(profile.gainType());
    }

    // Cover options ......................
    ui->copyCoverGroupBox->setCoverOptions(profile.copyCoverOptions());
    ui->embeddedCoverGroupBox->setCoverOptions(profile.embedCoverOptions());
    ui->embeddedCoverGroupBox->setVisible(profile.formatOptions().testFlag(FormatOption::SupportEmbeddedImage));

    // Out CUE options ....................
    ui->cueGroup->fromProfile(profile);
}

/************************************************
 *
 ************************************************/
void ProfileTabWidget::toProfile(Profile *profile) const
{
    mEncoderWidget->save(profile);

    // Result files options ................
    profile->setOutFileDir(ui->outDirEdit->text());
    profile->setOutFilePattern(ui->outPatternEdit->text());

    // Resample options ....................
    if (profile->formatOptions().testFlag(FormatOption::Lossless)) {

        profile->setBitsPerSample(ui->bitDepthComboBox->value());
        profile->setSampleRate(ui->sampleRateComboBox->value());
    }

    // Replay Gain options ................
    if (profile->formatOptions().testFlag(FormatOption::SupportGain)) {
        profile->setGainType(ui->gainComboBox->value());
    }

    // Cover options ......................
    profile->setCopyCoverOptions(ui->copyCoverGroupBox->coverOptions());
    profile->setEmbedCoverOptions(ui->embeddedCoverGroupBox->coverOptions());

    // Out CUE options ....................
    ui->cueGroup->toProfile(profile);
}

/************************************************
 *
 ************************************************/
void ProfileTabWidget::recreateEncoderWidget(const Profile &profile)
{
    mEncoderWidget.reset(profile.configPage(ui->encoderGroup));
    if (mEncoderWidget) {
        ui->encoderGroup->setTitle(
                tr("%1 encoder settings:", "Preferences group title, %1 is a audio format name")
                        .arg(profile.formatName()));

        ui->encoderGroup->setVisible(true);
        ui->encoderGroupLine->setVisible(true);

        if (mEncoderWidget->layout()) {
            mEncoderWidget->layout()->setMargin(0);
        }
        ui->encoderGroup->layout()->addWidget(mEncoderWidget.get());
        PreferencesDialog::fixLayout(this);
    }
    else {
        ui->encoderGroup->setVisible(false);
        ui->encoderGroupLine->setVisible(false);

        mEncoderWidget.reset(new NoEncoderConfigPage(ui->encoderGroup));
    }
}
