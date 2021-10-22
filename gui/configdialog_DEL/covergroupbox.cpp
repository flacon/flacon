#include "covergroupbox.h"
#include "ui_covergroupbox.h"

CoverGroupBox::CoverGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::CoverGroupBox)
{
    ui->setupUi(this);
    this->setChecked(false);
    ui->keepSizeButton->setChecked(mKeepSize);
    ui->scaleButton->setChecked(!mKeepSize);

    int h = 0;
    for (int r = 0; r < ui->coverImageLayout->rowCount(); ++r) {
        h = qMax(h, ui->coverImageLayout->cellRect(r, 0).height());
    }

    for (int r = 0; r < ui->coverImageLayout->rowCount(); ++r) {
        ui->coverImageLayout->setRowMinimumHeight(r, h);
    }
}

CoverGroupBox::~CoverGroupBox()
{
    delete ui;
}

CoverMode CoverGroupBox::coverMode() const
{
    if (!this->isChecked()) {
        return CoverMode::Disable;
    }

    if (ui->scaleButton->isChecked()) {
        return CoverMode::Scale;
    }

    return CoverMode::OrigSize;
}

void CoverGroupBox::setCoverMode(CoverMode value)
{
    if (value != CoverMode::Disable) {
        mKeepSize = (value == CoverMode::OrigSize);
    }

    this->setChecked(value != CoverMode::Disable);
    ui->keepSizeButton->setChecked(mKeepSize);
    ui->scaleButton->setChecked(!mKeepSize);
}

int CoverGroupBox::imageSize()
{
    return ui->resizeSpinBox->value();
}

void CoverGroupBox::setImageSize(int value)
{
    ui->resizeSpinBox->setValue(value);
}
