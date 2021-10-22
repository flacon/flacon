#include "profiletabwidget.h"
#include "ui_profiletabwidget.h"

ProfileTabWidget::ProfileTabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::ProfileTabWidget)
{
    ui->setupUi(this);

    ui->outDirEdit->setPlaceholderText(tr("Same directory as CUE file", "Placeholder for output direcory combobox"));
    ui->outDirButton->setBuddy(ui->outDirEdit);

    ui->outPatternButton->setBuddy(ui->outPatternEdit);
    ui->outPatternButton->addStandardPatterns();

    ui->encoderGroup->setLayout(new QVBoxLayout(ui->encoderGroup));

    ui->bitDepthComboBox->addItem(tr("Same as source", "Item in combobox"), int(BitsPerSample::AsSourcee));
    ui->bitDepthComboBox->addItem(tr("16-bit", "Item in combobox"), int(BitsPerSample::Bit_16));
    ui->bitDepthComboBox->addItem(tr("24-bit", "Item in combobox"), int(BitsPerSample::Bit_24));
    ui->bitDepthComboBox->addItem(tr("32-bit", "Item in combobox"), int(BitsPerSample::Bit_32));

    ui->sampleRateComboBox->addItem(tr("Same as source", "Item in combobox"), int(SampleRate::AsSource));
    ui->sampleRateComboBox->addItem(tr("44100 Hz", "Item in combobox"), int(SampleRate::Hz_44100));
    ui->sampleRateComboBox->addItem(tr("48000 Hz", "Item in combobox"), int(SampleRate::Hz_48000));
    ui->sampleRateComboBox->addItem(tr("96000 Hz", "Item in combobox"), int(SampleRate::Hz_96000));
    ui->sampleRateComboBox->addItem(tr("192000 Hz", "Item in combobox"), int(SampleRate::Hz_192000));

    ui->gainComboBox->clear();
    ui->gainComboBox->addItem(tr("Disabled", "ReplayGain type combobox"), gainTypeToString(GainType::Disable));
    ui->gainComboBox->addItem(tr("Per Track", "ReplayGain type combobox"), gainTypeToString(GainType::Track));
    ui->gainComboBox->addItem(tr("Per Album", "ReplayGain type combobox"), gainTypeToString(GainType::Album));
    ui->gainComboBox->setToolTip(tr("ReplayGain is a standard to normalize the perceived loudness of computer audio formats. \n\n"
                                    "The analysis can be performed on individual tracks, so that all tracks will be of equal volume on playback. \n"
                                    "Using the album-gain analysis will preserve the volume differences within an album."));
}

ProfileTabWidget::~ProfileTabWidget()
{
    delete ui;
}

Profile ProfileTabWidget::profile() const
{
    return mProfile;
}

void ProfileTabWidget::setProfile(const Profile &profile)
{
    mProfile = profile;
    ui->formatLabel->setVisible(false);
    ui->formatLabel->setText(tr("%1 format", "Preferences dialog: format name label, %1 is a audio format name")
                                     .arg(profile.formatName()));

    ui->gainGroup->setVisible(profile.formatOptions().testFlag(FormatOption::SupportGain));
    ui->resampleGroup->setVisible(profile.formatOptions().testFlag(FormatOption::Lossless));

    ui->encoderGroup->setTitle(
            tr("%1 encoder settings:", "Preferences group title, %1 is a audio format name")
                    .arg(profile.formatName()));

    mEncoderWidget.reset(profile.configPage(ui->encoderGroup));

    if (mEncoderWidget && mEncoderWidget->layout()) {
        mEncoderWidget->layout()->setMargin(0);
    }

    ui->encoderGroup->layout()->addWidget(mEncoderWidget.get());
}

void ProfileTabWidget::setProfileName(const QString &value)
{
    mProfile.setName(value);
}
