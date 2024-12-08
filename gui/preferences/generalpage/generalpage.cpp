/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

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

    initProxyTypeComboBox();

#ifdef DISABLE_TMP_DIR
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

/**************************************
 *
 **************************************/
void GeneralPage::initProxyTypeComboBox()
{
    ui->proxyTypeComboBox->addItem(tr("No proxy"), int(ProxyType::NoProxy));
    ui->proxyTypeComboBox->addItem(tr("HTTP"), int(ProxyType::HttpProxy));
    ui->proxyTypeComboBox->addItem(tr("SOCKS 5"), int(ProxyType::Socks5Proxy));
    ui->proxyTypeComboBox->setCurrentIndex(0);
}

QString GeneralPage::tmpDir() const
{
#ifndef DISABLE_TMP_DIR
    return ui->tmpDirEdit->text();
#else
    return "";
#endif
}

void GeneralPage::setTmpDir(const QString &value)
{
#ifndef DISABLE_TMP_DIR
    ui->tmpDirEdit->setText(value);
#else
    Q_UNUSED(value)
#endif
}

uint GeneralPage::encoderThreadsCount() const
{
    return uint(ui->threadsCountSpin->value());
}

void GeneralPage::setEncoderThreadsCount(uint value)
{
    ui->threadsCountSpin->setValue(value);
}

bool GeneralPage::isSplitTrackTitle() const
{
    return ui->splitTrackTitleCbx->isChecked();
}

void GeneralPage::setSplitTrackTitle(bool value)
{
    ui->splitTrackTitleCbx->setChecked(value);
}

/**************************************
 *
 **************************************/
ProxyType GeneralPage::proxyType() const
{
    return ProxyType(ui->proxyTypeComboBox->currentData().toInt());
}

/**************************************
 *
 **************************************/
void GeneralPage::setProxyType(ProxyType value)
{
    ui->proxyTypeComboBox->setCurrentIndex(qMax(0, ui->proxyTypeComboBox->findData(int(value))));
}

/**************************************
 *
 **************************************/
QString GeneralPage::proxyHost() const
{
    return ui->proxyHostEdit->text();
}

/**************************************
 *
 **************************************/
void GeneralPage::setProxyHost(const QString &value)
{
    ui->proxyHostEdit->setText(value);
}

/**************************************
 *
 **************************************/
uint GeneralPage::proxyPort() const
{
    return ui->proxyPortEdit->value();
}

/**************************************
 *
 **************************************/
void GeneralPage::setProxyPort(uint value)
{
    ui->proxyPortEdit->setValue(value);
}

/**************************************
 *
 **************************************/
QString GeneralPage::proxyUserName() const
{
    return ui->ProxyUserNameEdit->text();
}

/**************************************
 *
 **************************************/
void GeneralPage::setProxyUserName(const QString &value)
{
    ui->ProxyUserNameEdit->setText(value);
}

/**************************************
 *
 **************************************/
QString GeneralPage::proxyPassword() const
{
    return ui->proxyPasswordEdit->text();
}

/**************************************
 *
 **************************************/
void GeneralPage::setProxyPassword(const QString &value)
{
    ui->proxyPasswordEdit->setText(value);
}
