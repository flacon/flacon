#include "outcuegroupbox.h"
#include "ui_outcuegroupbox.h"
#include "../patternexpander.h"
#include "../icon.h"

OutCueGroupBox::OutCueGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::OutCueGroupBox)
{
    ui->setupUi(this);

    setEmbededCue(false);

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

    ui->preGapComboBox->addItem(tr("Extract to separate file"), int(PreGapType::ExtractToFile));
    ui->preGapComboBox->addItem(tr("Add to first track"), int(PreGapType::AddToFirstTrack));

    connect(ui->perTrackCueFormatBtn, &OutPatternButton::paternSelected,
            [this](const QString &pattern) { ui->perTrackCueFormatEdit->insert(pattern); });

    connect(ui->perTrackCueFormatBtn, &OutPatternButton::fullPaternSelected,
            [this](const QString &pattern) { ui->perTrackCueFormatEdit->setText(pattern); });

    connect(ui->writeToFileButton, &QRadioButton::clicked, this, &OutCueGroupBox::refresh);
    connect(ui->embedButton, &QRadioButton::clicked, this, &OutCueGroupBox::refresh);
}

OutCueGroupBox::~OutCueGroupBox()
{
    delete ui;
}

QString OutCueGroupBox::perTrackCueFormat() const
{
    return ui->perTrackCueFormatEdit->text();
}

void OutCueGroupBox::setPerTrackCueFormat(const QString &value)
{
    ui->perTrackCueFormatEdit->setText(value);
}

bool OutCueGroupBox::isCreateCue() const
{
    return this->isChecked();
}

void OutCueGroupBox::setCreateCue(bool value)
{
    this->setChecked(value);
}

bool OutCueGroupBox::isEmbededCue() const
{
    return ui->embedButton->isChecked();
}

void OutCueGroupBox::setEmbededCue(bool value)
{
    bool emb = mSupportEmbededCue && value;
    ui->embedButton->setChecked(emb);
    ui->writeToFileButton->setChecked(!emb);
    refresh();
}

PreGapType OutCueGroupBox::pregapType() const
{
    return PreGapType(ui->preGapComboBox->currentData().toInt());
}

void OutCueGroupBox::setPregapType(PreGapType value)
{
    int n = ui->preGapComboBox->findData(int(value));
    if (n > -1) {
        ui->preGapComboBox->setCurrentIndex(n);
    }
}

void OutCueGroupBox::setSupportEmbededCue(bool value)
{
    mSupportEmbededCue = value;
    refresh();
}

void OutCueGroupBox::refresh()
{
    ui->embedButton->setEnabled(mSupportEmbededCue);
    ui->perTrackCueFormatEdit->setEnabled(ui->writeToFileButton->isChecked());
    ui->perTrackCueFormatBtn->setEnabled(ui->writeToFileButton->isChecked());
}
