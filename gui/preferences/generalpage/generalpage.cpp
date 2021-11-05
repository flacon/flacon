#include "generalpage.h"
#include "ui_generalpage.h"
#include "icon.h"
#include <QFileDialog>

GeneralPage::GeneralPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralPage)
{
    ui->setupUi(this);

    ui->tmpDirButton->setIcon(Icon("folder"));
    ui->tmpDirButton->setBuddy(ui->tmpDirEdit);
    connect(ui->tmpDirButton, &QToolButton::clicked, this, &GeneralPage::showTmpDirDialog);

    ui->cddbComboBox->addItem("https://gnudb.gnudb.org/");

#ifdef FLATPAK_BUNDLE
    ui->tmpDirLabel->hide();
    ui->tmpDirEdit->hide();
    ui->tmpDirButton->hide();
#endif
}

GeneralPage::~GeneralPage()
{
    delete ui;
}

void GeneralPage::showTmpDirDialog()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select temporary directory"), tmpDir());
    if (!dir.isEmpty()) {
        setTmpDir(dir);
    }
}

QString GeneralPage::tmpDir() const
{
    return ui->tmpDirEdit->text();
}

void GeneralPage::setTmpDir(const QString &value)
{
    ui->tmpDirEdit->setText(value);
}

QString GeneralPage::defaultCodepage() const
{
    return ui->codePageComboBox->currentText();
}

void GeneralPage::setDefaultCodepage(const QString &value)
{
    ui->codePageComboBox->setCurrentText(value);
}

uint GeneralPage::encoderThreadsCount() const
{
    return uint(ui->threadsCountSpin->value());
}

void GeneralPage::setEncoderThreadsCount(uint value)
{
    ui->threadsCountSpin->setValue(value);
}

QString GeneralPage::cddbHost() const
{
    return ui->cddbComboBox->currentText();
}

void GeneralPage::setCddbHost(const QString &value)
{
    ui->cddbComboBox->setCurrentText(value);
}
