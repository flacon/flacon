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
#include "formats/encoderconfigpage.h"
#include "../patternexpander.h"
#include "../icon.h"

#include <QDebug>

ProfileWidget::ProfileWidget(const Profile &profile, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWidget),
    mEncoderWidget(profile.configPage(this)),
    mProfile(mEncoderWidget->profile())
{
    ui->setupUi(this);
    ui->formatLabel->setText(tr("%1 encoder", "Preferences dialog: format name label, %1 is a audio format name")
                                 .arg(profile.formatName()));

    ui->gainGroup->setVisible(profile.formatOptions().testFlag(FormatOption::SupportGain));
    ui->resampleGroup->setVisible(profile.formatOptions().testFlag(FormatOption::Lossless));


    if (mEncoderWidget->layout())
        mEncoderWidget->layout()->setMargin(0);

    ui->encoderPlace->insertWidget(0, mEncoderWidget);

    ui->perTrackCueFormatBtn->addPattern("%a", tr("Insert \"Artist\""));
    ui->perTrackCueFormatBtn->addPattern("%A", tr("Insert \"Album title\""));
    ui->perTrackCueFormatBtn->addPattern("%y", tr("Insert \"Year\""));
    ui->perTrackCueFormatBtn->addPattern("%g", tr("Insert \"Genre\""));

    const QString patterns[] = {
        "%a-%A.cue",
        "%a - %A.cue",
        "%a - %y - %A.cue"};


    for (QString pattern: patterns)
    {
        ui->perTrackCueFormatBtn->addFullPattern(pattern,
                                             tr("Use \"%1\"", "Predefined CUE file name, string like 'Use \"%a/%A/%n - %t.cue\"'")
                                             .arg(pattern)
                                             + "  ( " + PatternExpander::example(pattern) + " )");
    }

    ui->perTrackCueFormatBtn->setIcon(Icon("pattern-button"));

    ui->preGapComboBox->addItem(tr("Extract to separate file"), preGapTypeToString(PreGapType::ExtractToFile));
    ui->preGapComboBox->addItem(tr("Add to first track"),       preGapTypeToString(PreGapType::AddToFirstTrack));


    ui->bitDepthComboBox->addItem(tr("Same as source", "Item in combobox"), int(BitsPerSample::AsSourcee));
    ui->bitDepthComboBox->addItem(tr("16-bit",         "Item in combobox"), int(BitsPerSample::Bit_16));
    ui->bitDepthComboBox->addItem(tr("24-bit",         "Item in combobox"), int(BitsPerSample::Bit_24));
    ui->bitDepthComboBox->addItem(tr("32-bit",         "Item in combobox"), int(BitsPerSample::Bit_32));

    ui->sampleRateComboBox->addItem(tr("Same as source", "Item in combobox"), int(SampleRate::AsSource));
    ui->sampleRateComboBox->addItem(tr("44100 Hz",       "Item in combobox"), int(SampleRate::Hz_44100));
    ui->sampleRateComboBox->addItem(tr("48000 Hz",       "Item in combobox"), int(SampleRate::Hz_48000));
    ui->sampleRateComboBox->addItem(tr("96000 Hz",       "Item in combobox"), int(SampleRate::Hz_96000));
    ui->sampleRateComboBox->addItem(tr("192000 Hz",      "Item in combobox"), int(SampleRate::Hz_192000));

    ui->gainComboBox->clear();
    ui->gainComboBox->addItem(tr("Disabled",  "ReplayGain type combobox"), gainTypeToString(GainType::Disable));
    ui->gainComboBox->addItem(tr("Per Track", "ReplayGain type combobox"), gainTypeToString(GainType::Track));
    ui->gainComboBox->addItem(tr("Per Album", "ReplayGain type combobox"), gainTypeToString(GainType::Album));
    ui->gainComboBox->setToolTip(tr("ReplayGain is a standard to normalize the perceived loudness of computer audio formats. \n\n"
                                    "The analysis can be performed on individual tracks, so that all tracks will be of equal volume on playback. \n"
                                    "Using the album-gain analysis will preserve the volume differences within an album."));



    connect(ui->perTrackCueFormatBtn, &OutPatternButton::paternSelected,
            [this](const QString &pattern){ ui->perTrackCueFormatEdit->insert(pattern);});

    connect(ui->perTrackCueFormatBtn, &OutPatternButton::fullPaternSelected,
            [this](const QString &pattern){ ui->perTrackCueFormatEdit->setText(pattern);});



    // Load
    mEncoderWidget->load();

    if (mProfile.formatOptions().testFlag(FormatOption::Lossless)) {
        mEncoderWidget->loadWidget(Profile::BITS_PER_SAMPLE_KEY, ui->bitDepthComboBox);
        mEncoderWidget->loadWidget(Profile::SAMPLE_RATE_KEY,     ui->sampleRateComboBox);
    }

    if (mProfile.formatOptions().testFlag(FormatOption::SupportGain)) {
        mEncoderWidget->loadWidget(Profile::REPLAY_GAIN_KEY, ui->gainComboBox);
    }

    mEncoderWidget->loadWidget(Profile::CREATE_CUE_KEY,    ui->perTrackCueGroup);
    mEncoderWidget->loadWidget(Profile::PREGAP_TYPE_KEY,   ui->preGapComboBox);
    mEncoderWidget->loadWidget(Profile::CUE_FILE_NAME_KEY, ui->perTrackCueFormatEdit);

    fixLayout();
}


/************************************************
 *
 ************************************************/
#ifdef Q_OS_MAC
void ProfileWidget::fixLayout()
{
    QList<QLabel*> labels;
    int width = 0;

    for (auto *layout: findChildren<QFormLayout*>()) {
        layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        for (int r=0; r<layout->count(); ++r) {
            QLayoutItem *item = layout->itemAt(r, QFormLayout::LabelRole);
            if (!item) continue;

            QLabel *label = qobject_cast<QLabel*>(item->widget());
            if (label) {
                labels << label;
                width = qMax(width, label->sizeHint().width());
            }
        }
    }

    for (QLabel *label: labels) {
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
Profile ProfileWidget::profile() const
{
    mEncoderWidget->save();

    if (mProfile.formatOptions().testFlag(FormatOption::Lossless)) {
        mEncoderWidget->saveWidget(Profile::BITS_PER_SAMPLE_KEY, ui->bitDepthComboBox);
        mEncoderWidget->saveWidget(Profile::SAMPLE_RATE_KEY,     ui->sampleRateComboBox);
    }

    if (mProfile.formatOptions().testFlag(FormatOption::SupportGain)) {
        mEncoderWidget->saveWidget(Profile::REPLAY_GAIN_KEY, ui->gainComboBox);
    }

    mEncoderWidget->saveWidget(Profile::CREATE_CUE_KEY,    ui->perTrackCueGroup);
    mEncoderWidget->saveWidget(Profile::PREGAP_TYPE_KEY,   ui->preGapComboBox);
    mEncoderWidget->saveWidget(Profile::CUE_FILE_NAME_KEY, ui->perTrackCueFormatEdit);

    return mProfile;
}
