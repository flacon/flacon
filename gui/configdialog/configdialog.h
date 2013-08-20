/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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


#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "ui_configdialog.h"

class OutFormat;
class EncoderConfigPage;
class ProgramEdit;

class ConfigDialog : public QDialog, private Ui::ConfigDialog
{
    Q_OBJECT
public:
    static ConfigDialog *createAndShow(const OutFormat *format, QWidget *parent = 0);

signals:
    
public slots:
    void setPage(int pageIndex);
    void setPage(const OutFormat *format);

    void done(int res);
    void tmpDirShowDialog();

private:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

    void pagesListInit();
    void programsInit();

    void load();
    void write();

    QList<EncoderConfigPage*> mEncodersPages;
    QList<ProgramEdit*> mProgramEdits;
};


class EncoderConfigPage: public QWidget
{
    Q_OBJECT
public:
    explicit EncoderConfigPage(QWidget *parent = 0);
    virtual ~EncoderConfigPage();

    virtual void load() = 0;
    virtual void write() = 0;

    static QString losslessCompressionToolTip(int min, int max);
    static void setLosslessToolTip(QSlider *widget);
    static void setLosslessToolTip(QSpinBox *widget);

    static QString lossyCompressionToolTip(int min, int max);
    static void setLossyToolTip(QSlider *widget);
    static void setLossyToolTip(QSpinBox *widget);
    static void setLossyToolTip(QDoubleSpinBox *widget);

    static void fillReplayGainComboBox(QComboBox *comboBox);
    static void fillBitrateComboBox(QComboBox *comboBox, const QList<int> &bitrates);


    static void loadWidget(const QString &key, QSlider *widget);
    static void writeWidget(const QString &key, QSlider *widget);

    static void loadWidget(const QString &key, QLineEdit *widget);
    static void writeWidget(const QString &key, QLineEdit *widget);

    static void loadWidget(const QString &key, QCheckBox *widget);
    static void writeWidget(const QString &key, QCheckBox *widget);

    static void loadWidget(const QString &key, QSpinBox *widget);
    static void writeWidget(const QString &key, QSpinBox *widget);

    static void loadWidget(const QString &key, QDoubleSpinBox *widget);
    static void writeWidget(const QString &key, QDoubleSpinBox *widget);

    static void loadWidget(const QString &key, QComboBox *widget);
    static void writeWidget(const QString &key, QComboBox *widget);

};


#endif // CONFIGDIALOG_H
