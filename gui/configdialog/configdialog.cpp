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


#include "configdialog.h"
#include "outformat.h"
#include "../types.h"
#include "settings.h"
#include "project.h"
#include "../controls.h"

#include <QStringList>
#include <QSet>
#include <QFileDialog>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>


/************************************************

 ************************************************/
ConfigDialog *ConfigDialog::createAndShow(const OutFormat *format, QWidget *parent)
{
    ConfigDialog *instance = parent->findChild<ConfigDialog*>();

    if (!instance)
        instance = new ConfigDialog(parent);

    instance->setPage(format);
    instance->show();
    instance->raise();
    instance->activateWindow();
    instance->setAttribute(Qt::WA_DeleteOnClose);

    return instance;
}



/************************************************

 ************************************************/
ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    int width = settings->value(Settings::ConfigureDialog_Width).toInt();
    int height = settings->value(Settings::ConfigureDialog_Height).toInt();
    resize(width, height);


    generalPage->setWindowTitle(tr("General configuration"));
    programsPage->setWindowTitle(tr("Full path of the external applications"));

    connect(pages, SIGNAL(currentChanged(int)), this, SLOT(setPage(int)));

    tmpDirButton->setIcon(Project::getIcon("document-open-folder", "document-open", "folder_open", ":/icons/16/select-folder"));
    connect(tmpDirButton, SIGNAL(clicked()), this, SLOT(tmpDirShowDialog()));

    preGapComboBox->addItem(tr("Extract to separate file"), preGapTypeToString(PreGapType::ExtractToFile));
    preGapComboBox->addItem(tr("Add to first track"),       preGapTypeToString(PreGapType::AddToFirstTrack));

    pagesListInit();
    programsInit();
    pages->setCurrentIndex(0);
    load();

    preGapComboBox->setEnabled(perTrackCueCheck->isChecked());
}


/************************************************

 ************************************************/
ConfigDialog::~ConfigDialog()
{

}


/************************************************

 ************************************************/
void ConfigDialog::pagesListInit()
{
    QListWidgetItem *item = new QListWidgetItem(pagesList);
    item->setText(tr("General"));
    item->setIcon(Project::getIcon("go-home", "gohome", ":/icons/32/settings-main"));
    pagesList->addItem(item);

    int n = 1;
    foreach(OutFormat *format, OutFormat::allFormats())
    {
        EncoderConfigPage *page = format->configPage(this);
        if (!page)
            continue;

        mEncodersPages << page;

        page->setObjectName(format->id());
        pages->insertWidget(n, page);
        n++;

        item = new QListWidgetItem(pagesList);
        item->setText(format->name());
        item->setIcon(Project::getIcon("audio-x-generic", "sound", ":/icons/32/settings-encoder"));
        pagesList->addItem(item);
    }

    item = new QListWidgetItem(pagesList);
    item->setText(tr("Programs"));
    item->setIcon(Project::getIcon("applications-system", "gear", ":/icons/32/settings-programs"));
    pagesList->addItem(item);


    // Set item width ........................
    int width = 0;
    for(int i=0; i<pagesList->count(); ++i)
    {
        item = pagesList->item(i);
        width = qMax(width, pagesList->fontMetrics().width(item->text()));
    }

    width += 42;

    for(int i=0; i<pagesList->count(); ++i)
    {
        item = pagesList->item(i);
        int h = pagesList->height() + pagesList->fontMetrics().height() + 8;
        item->setSizeHint(QSize(width, h));
    }

    pagesList->setMaximumWidth(width + 2 * pagesList->frameWidth());
}


/************************************************

 ************************************************/
void ConfigDialog::programsInit()
{
    QStringList progs = QStringList::fromSet(settings->programs());
    progs.sort();

    int row = 0;
    foreach (QString prog, progs)
    {
        ProgramEdit *edit = new ProgramEdit(prog, programsPage);
        mProgramEdits << edit;

        QLabel *label = new QLabel(prog + ": ");
        label->setBuddy(edit);
        progsLayout->addWidget(label, row, 0);
        progsLayout->addWidget(edit,  row, 1);
        connect(progScanButton, SIGNAL(clicked()), edit, SLOT(find()));
        row++;
    }
}


/************************************************

 ************************************************/
void ConfigDialog::setPage(const OutFormat *format)
{
    int n = 0;
    if (format)
    {
        EncoderConfigPage *page = pages->findChild<EncoderConfigPage*>(format->id());
        if (page)
            n = pages->indexOf(page);
    }

    setPage(n);
}


/************************************************

 ************************************************/
void ConfigDialog::setPage(int pageIndex)
{
    pageTitle->setText(pages->currentWidget()->windowTitle());
    pagesList->setCurrentRow(pageIndex);
}


/************************************************

 ************************************************/
void ConfigDialog::done(int res)
{
    settings->setValue(Settings::ConfigureDialog_Width,  size().width());
    settings->setValue(Settings::ConfigureDialog_Height, size().height());

    if (res)
    {
        write();
        settings->sync();
    }

    QDialog::done(res);
}


/************************************************

 ************************************************/
void ConfigDialog::tmpDirShowDialog()
{
    QString tmpDir = QFileDialog::getExistingDirectory(this, tr("Select temporary directory"), tmpDirEdit->text());
    if (!tmpDir.isEmpty())
        tmpDirEdit->setText(tmpDir);
}


/************************************************

 ************************************************/
void ConfigDialog::load()
{
    EncoderConfigPage::loadWidget("Tags/DefaultCodepage",  codePageComboBox);
    EncoderConfigPage::loadWidget("Encoder/ThreadCount",   threadsCountSpin);
    EncoderConfigPage::loadWidget("Encoder/TmpDir",        tmpDirEdit);
    EncoderConfigPage::loadWidget("PerTrackCue/Create",    perTrackCueCheck);
    EncoderConfigPage::loadWidget("PerTrackCue/Pregap",    preGapComboBox);

    foreach(EncoderConfigPage *page, mEncodersPages)
        page->load();

    foreach(ProgramEdit *edit, mProgramEdits)
        edit->setText(settings->value("Programs/" + edit->programName()).toString());
}


/************************************************

 ************************************************/
void ConfigDialog::write()
{
    EncoderConfigPage::writeWidget("Tags/DefaultCodepage",  codePageComboBox);
    EncoderConfigPage::writeWidget("Encoder/ThreadCount",   threadsCountSpin);
    EncoderConfigPage::writeWidget("Encoder/TmpDir",        tmpDirEdit);
    EncoderConfigPage::writeWidget("PerTrackCue/Create",    perTrackCueCheck);
    EncoderConfigPage::writeWidget("PerTrackCue/Pregap",    preGapComboBox);

    foreach(EncoderConfigPage *page, mEncodersPages)
        page->write();

    foreach(ProgramEdit *edit, mProgramEdits)
        settings->setValue("Programs/" + edit->programName(), edit->text());
}



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
    int value = settings->value(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QSlider *widget)
{
    settings->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QLineEdit *widget)
{
    widget->setText(settings->value(key).toString());
}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QLineEdit *widget)
{
    settings->setValue(key, widget->text());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QCheckBox *widget)
{
    widget->setChecked(settings->value(key).toBool());
}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QCheckBox *widget)
{
    settings->setValue(key, widget->isChecked());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QSpinBox *widget)
{
    bool ok;
    int value = settings->value(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QSpinBox *widget)
{
    settings->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QDoubleSpinBox *widget)
{
    bool ok;
    int value = settings->value(key).toDouble(&ok);
    if (ok)
        widget->setValue(value);

}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QDoubleSpinBox *widget)
{
    settings->setValue(key, widget->value());
}


/************************************************

 ************************************************/
void EncoderConfigPage::loadWidget(const QString &key, QComboBox *widget)
{
    int n = qMax(0, widget->findData(settings->value(key)));
    widget->setCurrentIndex(n);
}


/************************************************

 ************************************************/
void EncoderConfigPage::writeWidget(const QString &key, QComboBox *widget)
{
    QVariant data = widget->itemData(widget->currentIndex());
    settings->setValue(key, data);
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
