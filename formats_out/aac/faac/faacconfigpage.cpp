#include "faacconfigpage.h"
#include "ui_faacconfigpage.h"

/************************************************

 ************************************************/
FaacConfigPage::FaacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::FaacConfigPage)
{
    ui->setupUi(this);

    setLossyToolTip(ui->aacQualitySpin);
    ui->aacQualitySlider->setToolTip(ui->aacQualitySpin->toolTip());
    fillBitrateComboBox(ui->aacBitrateCbx, QList<int>() << 64 << 80 << 128 << 160 << 192 << 224 << 256 << 288 << 320);

    connect(ui->aacUseQualityCheck, &QCheckBox::toggled, this, &FaacConfigPage::useQualityChecked);
}

/************************************************

 ************************************************/
FaacConfigPage::~FaacConfigPage()
{
    delete ui;
}

/************************************************

 ************************************************/
void FaacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "UseQuality", ui->aacUseQualityCheck);
    loadWidget(profile, "Quality", ui->aacQualitySpin);
    loadWidget(profile, "Bitrate", ui->aacBitrateCbx);
}

/************************************************

 ************************************************/
void FaacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "UseQuality", ui->aacUseQualityCheck);
    saveWidget(profile, "Quality", ui->aacQualitySpin);
    saveWidget(profile, "Bitrate", ui->aacBitrateCbx);
}

/************************************************

 ************************************************/
void FaacConfigPage::useQualityChecked(bool checked)
{
    ui->qualityLabel->setEnabled(checked);
    ui->aacQualitySlider->setEnabled(checked);
    ui->aacQualitySpin->setEnabled(checked);

    ui->bitrateLabel->setEnabled(!checked);
    ui->aacBitrateCbx->setEnabled(!checked);
}
