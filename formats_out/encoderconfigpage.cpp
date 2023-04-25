/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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

#include "encoderconfigpage.h"
#include "types.h"

#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>

/************************************************

 ************************************************/
EncoderConfigPage::EncoderConfigPage(QWidget *parent) :
    QWidget(parent)
{
}

/************************************************

 ************************************************/
EncoderConfigPage::~EncoderConfigPage()
{
}

/************************************************

 ************************************************/
QString EncoderConfigPage::lossyCompressionToolTip(int min, int max)
{
    return tr("Sets encoding quality, between %1 (lowest) and %2 (highest).").arg(min).arg(max);
}

/************************************************

 ************************************************/
void EncoderConfigPage::setLossyToolTip(QSlider *widget)
{
    widget->setToolTip(lossyCompressionToolTip(widget->minimum(), widget->maximum()));
}

/************************************************

 ************************************************/
void EncoderConfigPage::setLossyToolTip(QSpinBox *widget)
{
    widget->setToolTip(lossyCompressionToolTip(widget->minimum(), widget->maximum()));
}

/************************************************

 ************************************************/
void EncoderConfigPage::setLossyToolTip(QDoubleSpinBox *widget)
{
    widget->setToolTip(lossyCompressionToolTip(widget->minimum(), widget->maximum()));
}

/************************************************

 ************************************************/
QString EncoderConfigPage::losslessCompressionToolTip(int min, int max)
{
    return tr("Sets compression level, between %1 (fastest) and %2 (highest compression).\n"
              "This only affects the file size. All settings are lossless.")
            .arg(min)
            .arg(max);
}

/************************************************

 ************************************************/
void EncoderConfigPage::setLosslessToolTip(QSlider *widget)
{
    widget->setToolTip(losslessCompressionToolTip(widget->minimum(), widget->maximum()));
}

/************************************************

 ************************************************/
void EncoderConfigPage::setLosslessToolTip(QSpinBox *widget)
{
    widget->setToolTip(losslessCompressionToolTip(widget->minimum(), widget->maximum()));
}

/************************************************

 ************************************************/
void EncoderConfigPage::fillBitrateComboBox(QComboBox *comboBox, const QList<int> &bitrates)
{
    foreach (int bitrate, bitrates) {
        if (bitrate)
            comboBox->addItem(tr("%1 kbps").arg(bitrate), QVariant(bitrate));
        else
            comboBox->addItem(tr("Default"), QVariant());
    }
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QSlider *widget) const
{
    bool ok;
    int  value = profile.encoderValue(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QSlider *widget)
{
    profile->setEncoderValue(key, widget->value());
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QLineEdit *widget) const
{
    widget->setText(profile.encoderValue(key).toString());
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QLineEdit *widget)
{
    profile->setEncoderValue(key, widget->text());
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QCheckBox *widget) const
{
    bool wasChecked = widget->isChecked();
    widget->setChecked(profile.encoderValue(key).toBool());

    if (widget->isChecked() == wasChecked) {
        emit widget->toggled(widget->isChecked());
        emit widget->clicked(widget->isChecked());
    }
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QCheckBox *widget)
{
    profile->setEncoderValue(key, widget->isChecked());
}

/************************************************
 *
 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QGroupBox *widget) const
{
    bool wasChecked = widget->isChecked();
    widget->setChecked(profile.encoderValue(key).toBool());

    if (widget->isChecked() == wasChecked) {
        emit widget->toggled(widget->isChecked());
        emit widget->clicked(widget->isChecked());
    }
}

/************************************************
 *
 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QGroupBox *widget)
{
    profile->setEncoderValue(key, widget->isChecked());
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QSpinBox *widget) const
{
    bool ok;
    int  value = profile.encoderValue(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QSpinBox *widget)
{
    profile->setEncoderValue(key, widget->value());
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QDoubleSpinBox *widget) const
{
    bool ok;
    int  value = profile.encoderValue(key).toDouble(&ok);
    if (ok)
        widget->setValue(value);
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QDoubleSpinBox *widget)
{
    profile->setEncoderValue(key, widget->value());
}

/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const Profile &profile, const QString &key, QComboBox *widget) const
{
    if (widget->isEditable()) {
        widget->setEditText(profile.encoderValue(key).toString());
    }
    else {
        int n = qMax(0, widget->findData(profile.encoderValue(key)));
        widget->setCurrentIndex(n);
    }
}

/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(Profile *profile, const QString &key, const QComboBox *widget)
{
    if (widget->isEditable()) {
        profile->setEncoderValue(key, widget->currentText());
    }
    else {
        QVariant data = widget->itemData(widget->currentIndex());
        profile->setEncoderValue(key, data);
    }
}

/************************************************

 ************************************************/
QString EncoderConfigPage::toolTipCss()
{
    return "<style type='text/css'>\n"
           "qbody { font-size: 9px; }\n"
           "dt { font-weight: bold; }\n"
           "dd { margin-left: 8px; margin-bottom: 8px; }\n"
           "</style>\n";
}
