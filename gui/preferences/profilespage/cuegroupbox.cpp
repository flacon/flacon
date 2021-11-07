#include "cuegroupbox.h"
#include "ui_cuegroupbox.h"
#include "../patternexpander.h"
#include "../icon.h"

CueGroupBox::CueGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::CueGroupBox)
{
    ui->setupUi(this);

    ui->perTrackCueFormatBtn->addPattern("%a", tr("Insert \"Artist\""));
    ui->perTrackCueFormatBtn->addPattern("%A", tr("Insert \"Album title\""));
    ui->perTrackCueFormatBtn->addPattern("%y", tr("Insert \"Year\""));
    ui->perTrackCueFormatBtn->addPattern("%g", tr("Insert \"Genre\""));

    const QString patterns[] = {
        "%a-%A.cue",
        "%a - %A.cue",
        "%a - %y - %A.cue"
    };

    for (QString pattern : patterns) {
        ui->perTrackCueFormatBtn->addFullPattern(pattern,
                                                 tr("Use \"%1\"", "Predefined CUE file name, string like 'Use \"%a/%A/%n - %t.cue\"'")
                                                                 .arg(pattern)
                                                         + "  ( " + PatternExpander::example(pattern) + " )");
    }

    ui->perTrackCueFormatBtn->setIcon(Icon("pattern-button"));

    ui->preGapComboBox->addItem(tr("Extract to separate file"), PreGapType::ExtractToFile);
    ui->preGapComboBox->addItem(tr("Add to first track"), PreGapType::AddToFirstTrack);

    connect(ui->perTrackCueFormatBtn, &OutPatternButton::paternSelected,
            [this](const QString &pattern) { ui->perTrackCueFormatEdit->insert(pattern); });

    connect(ui->perTrackCueFormatBtn, &OutPatternButton::fullPaternSelected,
            [this](const QString &pattern) { ui->perTrackCueFormatEdit->setText(pattern); });

    connect(ui->writeToFileButton, &QCheckBox::clicked, this, &CueGroupBox::refresh);
    connect(ui->embedButton, &QCheckBox::clicked, this, &CueGroupBox::refresh);

    refresh();
}

CueGroupBox::~CueGroupBox()
{
    delete ui;
}

void CueGroupBox::fromProfile(const Profile &profile)
{
    mSupportEmbeddedCue = profile.formatOptions().testFlag(FormatOption::SupportEmbeddedCue);

    ui->writeToFileButton->setChecked(profile.isCreateCue());
    ui->embedButton->setChecked(profile.isEmbedCue());
    ui->perTrackCueFormatEdit->setText(profile.cueFileName());
    ui->preGapComboBox->setValue(profile.preGapType());
    refresh();
}

void CueGroupBox::toProfile(Profile *profile) const
{
    profile->setCreateCue(ui->writeToFileButton->isChecked());
    profile->setEmbedCue(ui->embedButton->isChecked());
    profile->setCueFileName(ui->perTrackCueFormatEdit->text());
    profile->setPregapType(ui->preGapComboBox->value());
}

void CueGroupBox::refresh()
{
    ui->embedButton->setEnabled(mSupportEmbeddedCue);
    ui->perTrackCueFormatEdit->setEnabled(ui->writeToFileButton->isChecked());
    ui->perTrackCueFormatBtn->setEnabled(ui->writeToFileButton->isChecked());
    ui->preGapComboBox->setEnabled(ui->writeToFileButton->isChecked() || ui->embedButton->isChecked());
    ui->preGapLabel->setEnabled(ui->preGapComboBox->isEnabled());
}
