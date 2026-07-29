#include "fdkaacconfigpage.h"
#include "ui_fdkaacconfigpage.h"

/************************************************

 ************************************************/
FdkaacConfigPage::FdkaacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::FdkaacConfigPage)
{
    ui->setupUi(this);

    initQualitySlider();
    initCbrBitrateComboBox();

    connect(ui->useVbrCheckBox, &QCheckBox::clicked, this, &FdkaacConfigPage::refresh);

    refresh();
}

/************************************************

 ************************************************/
FdkaacConfigPage::~FdkaacConfigPage()
{
    delete ui;
}

/************************************************

 ************************************************/
bool FdkaacConfigPage::useVbr() const
{
    return ui->useVbrCheckBox->isChecked();
}

/************************************************

 ************************************************/
int FdkaacConfigPage::vbrQuality() const
{
    return ui->qualitySlider->value();
}

/************************************************

 ************************************************/
int FdkaacConfigPage::cbrBitrate() const
{
    return ui->cbrBitrateComboBox->currentData().toInt();
}

/************************************************

 ************************************************/
void FdkaacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "Fdkaac/UseVBR", ui->useVbrCheckBox);
    loadWidget(profile, "Fdkaac/Quality", ui->qualitySlider);
    loadWidget(profile, "Fdkaac/Bitrate", ui->cbrBitrateComboBox);
    refresh();
}

/************************************************

 ************************************************/
void FdkaacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "Fdkaac/UseVBR", ui->useVbrCheckBox);
    saveWidget(profile, "Fdkaac/Quality", ui->qualitySlider);
    saveWidget(profile, "Fdkaac/Bitrate", ui->cbrBitrateComboBox);
}

/************************************************

 ************************************************/
void FdkaacConfigPage::initQualitySlider()
{
    QLabel   *label   = ui->qualityLabel;
    QSlider  *slider  = ui->qualitySlider;
    QSpinBox *spinBox = ui->qualitySpin;

    label->setBuddy(slider);

    slider->setMinimum(0);
    slider->setMaximum(5);
    slider->setSingleStep(1);
    slider->setPageStep(1);
    slider->setTracking(true);
    slider->setTickPosition(QSlider::TicksAbove);
    slider->setTickInterval(1);

    spinBox->setMinimum(slider->minimum());
    spinBox->setMaximum(slider->maximum());

    connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
    connect(spinBox, &QSpinBox::valueChanged, slider, &QSlider::setValue);

    slider->setToolTip(tr("VBR (higher value -> higher bitrate)"));
    spinBox->setToolTip(slider->toolTip());
}

void FdkaacConfigPage::initCbrBitrateComboBox()
{
    QLabel    *label    = ui->cbrBitrateLabel;
    QComboBox *comboBox = ui->cbrBitrateComboBox;

    fillBitrateComboBox(comboBox, { 64, 80, 128, 160, 192, 224, 256, 288, 320 });

    label->setBuddy(comboBox);

    comboBox->setToolTip(tr("Target bitrate (for CBR)"));
}

/************************************************

 ************************************************/
void FdkaacConfigPage::refresh()
{
    ui->qualityLabel->setEnabled(useVbr());
    ui->qualitySlider->setEnabled(useVbr());
    ui->qualitySpin->setEnabled(useVbr());

    ui->cbrBitrateComboBox->setEnabled(!useVbr());
    ui->cbrBitrateLabel->setEnabled(ui->cbrBitrateComboBox->isEnabled());
}
