/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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

#include "profilewidget.h"
#include "ui_profilewidget.h"
#include "profiles.h"
#include "formats_out/encoderconfigpage.h"
//#include "../patternexpander.h"
#include "../icon.h"

#include <QDebug>

#define DONE

ProfileWidget::ProfileWidget(const Profile &profile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWidget),
    mEncoderWidget(profile.configPage(this)),
    mProfile(mEncoderWidget->profile())
{
    ui->setupUi(this);
    ui->formatLabel->setText(tr("%1 format", "Preferences dialog: format name label, %1 is a audio format name")
                                     .arg(profile.formatName()));

DONE    ui->outDirEdit->setPlaceholderText(tr("Same directory as CUE file", "Placeholder for output direcory combobox"));
DONE    ui->outDirButton->setBuddy(ui->outDirEdit);

    ui->outPatternButton->setBuddy(ui->outPatternEdit);
    ui->outPatternButton->addStandardPatterns();

    ui->gainGroup->setVisible(profile.formatOptions().testFlag(FormatOption::SupportGain));
    ui->resampleGroup->setVisible(profile.formatOptions().testFlag(FormatOption::Lossless));

    ui->encoderGroup->setTitle(
            tr("%1 encoder settings:", "Preferences group title, %1 is a audio format name")
                    .arg(profile.formatName()));

    if (mEncoderWidget->layout())
        mEncoderWidget->layout()->setMargin(0);

    ui->encoderGroup->setLayout(new QVBoxLayout(ui->encoderGroup));
    ui->encoderGroup->layout()->addWidget(mEncoderWidget);

    ui->bitDepthComboBox->addItem(tr("Same as source", "Item in combobox"), int(BitsPerSample::AsSourcee));
    ui->bitDepthComboBox->addItem(tr("16-bit", "Item in combobox"), int(BitsPerSample::Bit_16));
    ui->bitDepthComboBox->addItem(tr("24-bit", "Item in combobox"), int(BitsPerSample::Bit_24));
    ui->bitDepthComboBox->addItem(tr("32-bit", "Item in combobox"), int(BitsPerSample::Bit_32));

    ui->sampleRateComboBox->addItem(tr("Same as source", "Item in combobox"), int(SampleRate::AsSource));
    ui->sampleRateComboBox->addItem(tr("44100 Hz", "Item in combobox"), int(SampleRate::Hz_44100));
    ui->sampleRateComboBox->addItem(tr("48000 Hz", "Item in combobox"), int(SampleRate::Hz_48000));
    ui->sampleRateComboBox->addItem(tr("96000 Hz", "Item in combobox"), int(SampleRate::Hz_96000));
    ui->sampleRateComboBox->addItem(tr("192000 Hz", "Item in combobox"), int(SampleRate::Hz_192000));

    ui->gainComboBox->clear();
    ui->gainComboBox->addItem(tr("Disabled", "ReplayGain type combobox"), gainTypeToString(GainType::Disable));
    ui->gainComboBox->addItem(tr("Per Track", "ReplayGain type combobox"), gainTypeToString(GainType::Track));
    ui->gainComboBox->addItem(tr("Per Album", "ReplayGain type combobox"), gainTypeToString(GainType::Album));
    ui->gainComboBox->setToolTip(tr("ReplayGain is a standard to normalize the perceived loudness of computer audio formats. \n\n"
                                    "The analysis can be performed on individual tracks, so that all tracks will be of equal volume on playback. \n"
                                    "Using the album-gain analysis will preserve the volume differences within an album."));

    load();
    fixLayout();
}

/************************************************
 *
 ************************************************/
void ProfileWidget::load()
{
    mEncoderWidget->load();

DONE    mEncoderWidget->loadWidget(Profile::OUT_DIRECTORY_KEY, ui->outDirEdit);
DONE    mEncoderWidget->loadWidget(Profile::OUT_PATTERN_KEY, ui->outPatternEdit);
DONE
DONE     if (mProfile.formatOptions().testFlag(FormatOption::Lossless)) {
DONE         mEncoderWidget->loadWidget(Profile::BITS_PER_SAMPLE_KEY, ui->bitDepthComboBox);
DONE         mEncoderWidget->loadWidget(Profile::SAMPLE_RATE_KEY, ui->sampleRateComboBox);
DONE     }
DONE
DONE     if (mProfile.formatOptions().testFlag(FormatOption::SupportGain)) {
DONE         mEncoderWidget->loadWidget(Profile::REPLAY_GAIN_KEY, ui->gainComboBox);
DONE     }

    ui->outCueGroup->setSupportEmbededCue(mProfile.formatOptions().testFlag(FormatOption::SupportEmbededCue));
    ui->outCueGroup->setCreateCue(mProfile.value(Profile::CREATE_CUE_KEY).toBool());
    ui->outCueGroup->setEmbededCue(mProfile.value(Profile::EMBED_CUE_KEY).toBool());
    ui->outCueGroup->setPerTrackCueFormat(mProfile.value(Profile::CUE_FILE_NAME_KEY).toString());
    ui->outCueGroup->setPregapType(strToPreGapType(mProfile.value(Profile::PREGAP_TYPE_KEY).toString()));
}

/************************************************
 * controls.h
 * profiletabwidget.h
 * using BitsPerSampleCombobox = TCombobox<int>;
using SampleRateCombobox    = TCombobox<SampleRate>;
 ************************************************/
void ProfileWidget::save() const
{
    mEncoderWidget->save();

DONE     mEncoderWidget->saveWidget(Profile::OUT_DIRECTORY_KEY, ui->outDirEdit);
DONE     mEncoderWidget->saveWidget(Profile::OUT_PATTERN_KEY, ui->outPatternEdit);
DONE
DONE     if (mProfile.formatOptions().testFlag(FormatOption::Lossless)) {
DONE         mEncoderWidget->saveWidget(Profile::BITS_PER_SAMPLE_KEY, ui->bitDepthComboBox);
DONE         mEncoderWidget->saveWidget(Profile::SAMPLE_RATE_KEY, ui->sampleRateComboBox);
DONE     }
DONE
DONE    if (mProfile.formatOptions().testFlag(FormatOption::SupportGain)) {
DONE         mEncoderWidget->saveWidget(Profile::REPLAY_GAIN_KEY, ui->gainComboBox);
DONE     }

    mProfile.setValue(Profile::CREATE_CUE_KEY, ui->outCueGroup->isCreateCue());
    mProfile.setValue(Profile::EMBED_CUE_KEY, ui->outCueGroup->isEmbededCue());
    mProfile.setValue(Profile::PREGAP_TYPE_KEY, preGapTypeToString(ui->outCueGroup->pregapType()));
    mProfile.setValue(Profile::CUE_FILE_NAME_KEY, ui->outCueGroup->perTrackCueFormat());
}

/************************************************
 *
 ************************************************/
#ifdef Q_OS_MAC
void ProfileWidget::fixLayout()
{
    QList<QLabel *> labels;
    int             width = 0;

    for (auto *layout : findChildren<QFormLayout *>()) {
        layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        for (int r = 0; r < layout->count(); ++r) {
            QLayoutItem *item = layout->itemAt(r, QFormLayout::LabelRole);
            if (!item)
                continue;

            QLabel *label = qobject_cast<QLabel *>(item->widget());
            if (label) {
                labels << label;
                width = qMax(width, label->sizeHint().width());
            }
        }
    }

    for (QLabel *label : labels) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setMinimumWidth(width);
    }

    adjustSize();
}


#else
void ProfileWidget::fixLayout()
{
}
#endif

/************************************************
 *
 ************************************************/
ProfileWidget::~ProfileWidget()
{
    delete ui;
}

/************************************************
 *
 ************************************************/
void ProfileWidget::setProfile(const Profile &value)
{
    mProfile = value;
}

void ProfileWidget::referesh()
{
    ui->outDirEdit.set
}
