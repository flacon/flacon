#include "covergroupbox.h"
#include "ui_covergroupbox.h"
#include <QDebug>

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

    connect(ui->scaleButton, &QRadioButton::toggled, this, &CoverGroupBox::refresh);
    connect(this, &QGroupBox::toggled, this, &CoverGroupBox::refresh);
}

CoverGroupBox::~CoverGroupBox()
{
    delete ui;
}

CoverOptions CoverGroupBox::coverOptions() const
{
    CoverOptions res;
    res.mode = CoverMode::Disable;

    if (this->isChecked()) {
        res.mode = (ui->scaleButton->isChecked()) ? CoverMode::Scale : CoverMode::OrigSize;
    }

    res.size = ui->resizeSpinBox->value();
    return res;
}

void CoverGroupBox::setCoverOptions(const CoverOptions &value)
{
    if (value.mode != CoverMode::Disable) {
        mKeepSize = (value.mode == CoverMode::OrigSize);
    }

    this->setChecked(value.mode != CoverMode::Disable);
    ui->keepSizeButton->setChecked(mKeepSize);
    ui->scaleButton->setChecked(!mKeepSize);

    ui->resizeSpinBox->setValue(value.size);
}

void CoverGroupBox::refresh()
{
    ui->resizeSpinBox->setEnabled(ui->scaleButton->isChecked() && this->isChecked());
}
