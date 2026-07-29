#include "configpage_acc.h"
#include <QBoxLayout>
#include <QFormLayout>
#include "faac/faacconfigpage.h"
#include "fdkaac/fdkaacconfigpage.h"
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include "../extprogram.h"

/************************************************

 ************************************************/
ConfigPage_Acc::ConfigPage_Acc(QWidget *parent) :
    EncoderConfigPage(parent)
{
    if (ExtProgram::fdkaac()->check()) {
        initDouble();
    }
    else {
        initSingle();
    }
}

/************************************************

 ************************************************/
void ConfigPage_Acc::initSingle()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mFaacPage           = new FaacConfigPage(this);
    layout->addWidget(mFaacPage);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::initDouble()
{

    mProgLabel = new QLabel(this);
    mProgLabel->setText(tr("Program:"));

    mProgCombo = new QComboBox(this);
    mProgCombo->addItem("faac", "faac");
    mProgCombo->addItem("fdkaac", "fdkaac");
    connect(mProgCombo, &QComboBox::currentIndexChanged, this, &ConfigPage_Acc::progComboChanged);

    mFaacPage   = new FaacConfigPage(this);
    mFdkaacPage = new FdkaacConfigPage(this);

    mPages = new QStackedWidget(this);
    mPages->addWidget(mFaacPage);

    mPages->addWidget(mFdkaacPage);

    QFormLayout *hLayout = new QFormLayout();
    hLayout->setWidget(0, QFormLayout::LabelRole, mProgLabel);
    hLayout->setWidget(0, QFormLayout::FieldRole, mProgCombo);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(mPages);
}

/************************************************

 ************************************************/
void ConfigPage_Acc::progComboChanged()
{
    mPages->setCurrentIndex(mProgCombo->currentIndex());
}

/************************************************

 ************************************************/
void ConfigPage_Acc::load(const Profile &profile)
{
    if (mFaacPage) {
        mFaacPage->load(profile);
    }

    if (mFdkaacPage) {
        mFdkaacPage->load(profile);
    }

    if (mProgCombo) {
        loadWidget(profile, "Program", mProgCombo);
    }
}

/************************************************

 ************************************************/
void ConfigPage_Acc::save(Profile *profile)
{
    if (mFaacPage) {
        mFaacPage->save(profile);
    }

    if (mFdkaacPage) {
        mFdkaacPage->save(profile);
    }

    if (mProgCombo) {
        saveWidget(profile, "Program", mProgCombo);
    }
}
