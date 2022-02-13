#include "alacconfigpage.h"
#include "ui_alacconfigpage.h"

AlacConfigPage::AlacConfigPage(QWidget *parent) :
    EncoderConfigPage(parent),
    ui(new Ui::AlacConfigPage)
{
    ui->setupUi(this);

    setLosslessToolTip(ui->compressionSlider);
    ui->compressionSpin->setToolTip(ui->compressionSlider->toolTip());
}

AlacConfigPage::~AlacConfigPage()
{
    delete ui;
}

void AlacConfigPage::load(const Profile &profile)
{
    loadWidget(profile, "Compression", ui->compressionSlider);
}

void AlacConfigPage::save(Profile *profile)
{
    saveWidget(profile, "Compression", ui->compressionSlider);
}
