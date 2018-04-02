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
#include <QSpinBox>

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

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

    tmpDirButton->setIcon(loadIcon("folder"));
    connect(tmpDirButton, SIGNAL(clicked()), this, SLOT(tmpDirShowDialog()));


    connect(coverDisableButton,  &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::Disable);  });
    connect(coverKeepSizeButton, &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::OrigSize); });
    connect(coverScaleButton,    &QRadioButton::clicked,  [=](){this->setCoverMode(CoverMode::Scale);    });

    preGapComboBox->addItem(tr("Extract to separate file"), preGapTypeToString(PreGapType::ExtractToFile));
    preGapComboBox->addItem(tr("Add to first track"),       preGapTypeToString(PreGapType::AddToFirstTrack));

    initTabPages();

#ifdef MAC_BUNDLE
    pages->removeTab(pages->indexOf(programsPage));
#else
    initPrograms();
#endif

#ifdef MAC_UPDATER
    initUpdatePage();
#else
    pages->removeTab(pages->indexOf(updatePage));
#endif

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
void ConfigDialog::initTabPages()
{
    int n = 1;
    foreach(OutFormat *format, OutFormat::allFormats())
    {
        EncoderConfigPage *page = format->configPage(this);
        if (!page)
            continue;

        mEncodersPages << page;

        page->setObjectName(format->id());
        pages->insertTab(n, page, format->name());
        n++;
    }
}


/************************************************

 ************************************************/
void ConfigDialog::initPrograms()
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
 *
 ************************************************/
void ConfigDialog::initUpdatePage()
{
#ifdef MAC_UPDATER
    connect(updateNowBtn, &QPushButton::clicked,
            [this]() {
                Updater::sharedUpdater().checkForUpdatesInBackground();
                updateLastUpdateLbl();
            });

    updateLastUpdateLbl();

#endif
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
    pages->setCurrentIndex(pageIndex);
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
 *
 ************************************************/
void ConfigDialog::setCoverMode(CoverMode mode)
{
    switch (mode)
    {
    case CoverMode::Disable:
        coverDisableButton->setChecked(true);
        coverResizeSpinBox->setEnabled(false);
        break;

    case CoverMode::OrigSize:
        coverKeepSizeButton->setChecked(true);
        coverResizeSpinBox->setEnabled(false);
        break;

    case CoverMode::Scale:
        coverScaleButton->setChecked(true);
        coverResizeSpinBox->setEnabled(true);
        break;
    }
}


/************************************************
 *
 ************************************************/
void ConfigDialog::updateLastUpdateLbl()
{
#ifdef MAC_UPDATER
    QDateTime date = Updater::sharedUpdater().lastUpdateCheckDate();
    QString s;
    if (!date.isNull())
        s = tr("Last check was %1", "Information about last update")
                .arg(date.toString(Qt::DefaultLocaleLongDate));
    else
        s = tr("Never checked", "Information about last update");

    lastUpdateLbl->setText(s);
#endif
}


/************************************************
 *
 ************************************************/
void loadWidget(Settings::Key key, QCheckBox *widget)
{
    widget->setChecked(settings->value(key).toBool());
}


/************************************************
 *
 ************************************************/
void loadWidget(Settings::Key key, QSpinBox *widget)
{
    bool ok;
    int value = settings->value(key).toInt(&ok);
    if (ok)
        widget->setValue(value);
}


/************************************************
 *
 ************************************************/
void writeWidget(Settings::Key key, QCheckBox *widget)
{
    settings->setValue(key, widget->isChecked());
}


/************************************************

 ************************************************/
void writeWidget(Settings::Key key, QSpinBox *widget)
{
    settings->setValue(key, widget->value());
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

    setCoverMode(settings->coverMode());
    loadWidget(Settings::Cover_ResizeSize,   coverResizeSpinBox);

    foreach(EncoderConfigPage *page, mEncodersPages)
        page->load();

    foreach(ProgramEdit *edit, mProgramEdits)
        edit->setText(settings->value("Programs/" + edit->programName()).toString());

#ifdef MAC_UPDATER
   autoUpdateCbk->setChecked(Updater::sharedUpdater().automaticallyChecksForUpdates());
#endif
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

    settings->setValue(Settings::Cover_Mode, coverModeToString(coverMode()));
    writeWidget(Settings::Cover_ResizeSize,   coverResizeSpinBox);

    foreach(EncoderConfigPage *page, mEncodersPages)
        page->write();

    foreach(ProgramEdit *edit, mProgramEdits)
        settings->setValue("Programs/" + edit->programName(), edit->text());

#ifdef MAC_UPDATER
    Updater::sharedUpdater().setAutomaticallyChecksForUpdates(
        autoUpdateCbk->isChecked());
#endif
}



/************************************************

 ************************************************/
CoverMode ConfigDialog::coverMode() const
{
    if (coverDisableButton->isChecked())  return CoverMode::Disable;
    if (coverKeepSizeButton->isChecked()) return CoverMode::OrigSize;
    if (coverScaleButton->isChecked())    return CoverMode::Scale;

    return CoverMode::Disable;
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
