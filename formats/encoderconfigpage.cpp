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
#include "settings.h"
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>


/************************************************

 ************************************************/
EncoderConfigPage::EncoderConfigPage(QWidget *parent):
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
    return tr("Sets encoding quality, between %1 (lowest) and %2 (highest)."
              ).arg(min).arg(max);
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
              "This only affects the file size. All settings are lossless."
             ).arg(min).arg(max);
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
void EncoderConfigPage::fillReplayGainComboBox(QComboBox *comboBox)
{
    comboBox->clear();
    comboBox->addItem(tr("Disabled",  "ReplayGain type combobox"), gainTypeToString(GainType::Disable));
    comboBox->addItem(tr("Per Track", "ReplayGain type combobox"), gainTypeToString(GainType::Track));
    comboBox->addItem(tr("Per Album", "ReplayGain type combobox"), gainTypeToString(GainType::Album));
    comboBox->setToolTip(tr("ReplayGain is a standard to normalize the perceived loudness of computer audio formats. \n\n"
                            "The analysis can be performed on individual tracks, so that all tracks will be of equal volume on playback. \n"
                            "Using the album-gain analysis will preserve the volume differences within an album."));
}


/************************************************

 ************************************************/
void EncoderConfigPage::fillBitrateComboBox(QComboBox *comboBox, const QList<int> &bitrates)
{
    foreach(int bitrate, bitrates)
    {
        if (bitrate)
            comboBox->addItem(tr("%1 kbps").arg(bitrate), QVariant(bitrate));
        else
            comboBox->addItem(tr("Default"), QVariant());
    }
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QSlider *widget)
{
    bool ok;
    int value = Settings::i()->value(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QSlider *widget)
{
    Settings::i()->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QLineEdit *widget)
{
    widget->setText(Settings::i()->value(key).toString());
}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QLineEdit *widget)
{
    Settings::i()->setValue(key, widget->text());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QCheckBox *widget)
{
    widget->setChecked(Settings::i()->value(key).toBool());
}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QCheckBox *widget)
{
    Settings::i()->setValue(key, widget->isChecked());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QSpinBox *widget)
{
    bool ok;
    int value = Settings::i()->value(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QSpinBox *widget)
{
    Settings::i()->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QDoubleSpinBox *widget)
{
    bool ok;
    int value = Settings::i()->value(key).toDouble(&ok);
    if (ok)
        widget->setValue(value);

}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QDoubleSpinBox *widget)
{
    Settings::i()->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QComboBox *widget)
{
    int n = qMax(0, widget->findData(Settings::i()->value(key)));
    widget->setCurrentIndex(n);
}


/************************************************

 ************************************************/
void EncoderConfigPage::saveWidget(const QString &key, QComboBox *widget)
{
    QVariant data = widget->itemData(widget->currentIndex());
    Settings::i()->setValue(key, data);
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
