#include "outtagspage.h"
#include "ui_outtagspage.h"

OutTagsPage::OutTagsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OutTagsPage)
{
    ui->setupUi(this);
}

OutTagsPage::~OutTagsPage()
{
    delete ui;
}

bool OutTagsPage::isWriteSingleDiskNum() const
{
    return ui->writeSingleDiskNumCheckBox->isChecked();
}

void OutTagsPage::setWriteSingleDiskNum(bool value)
{
    ui->writeSingleDiskNumCheckBox->setChecked(value);
}
