/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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

#include "flacoutformat.h"
#include "project.h"
#include "inputaudiofile.h"
#include "flacmetadatawriter.h"
#include <QDebug>

static constexpr int MATAFLAC_MAX_SAMPLE_RATE = 192 * 1000;

/************************************************
 *
 ************************************************/
OutFormat_Flac::OutFormat_Flac()
{
    mId      = "FLAC";
    mExt     = "flac";
    mName    = "FLAC";
    mOptions = FormatOption::Lossless | FormatOption::SupportGain | FormatOption::SupportEmbeddedCue | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
bool OutFormat_Flac::check(const Profile &profile, QStringList *errors) const
{
    bool res = OutFormat::check(profile, errors);

    if (profile.gainType() == GainType::Disable) {
        return res;
    }

    for (int i = 0; i < Project::instance()->count(); ++i) {
        const Disc *const disc = Project::instance()->disc(i);

        for (const InputAudioFile &audioFile : disc->audioFiles()) {
            if (calcSampleRate(audioFile.sampleRate(), profile.sampleRate()) > MATAFLAC_MAX_SAMPLE_RATE) {
                *errors << QObject::tr("you can't use 'ReplayGain' for files with sample rates above 48kHz. Metaflac doesn't support such files.",
                                       "This string should begin with a lowercase letter. This is a part of the complex sentence.");
                res = false;
                break;
            }
        }
    }

    return res;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Flac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Compression", 5);
    res.insert("ReplayGain", gainTypeToString(GainType::Disable));
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Flac::configPage(QWidget *parent) const
{
    return new ConfigPage_Flac(parent);
}

/************************************************

 ************************************************/
ExtProgram *OutFormat_Flac::encoderProgram(const Profile &) const
{
    return ExtProgram::flac();
}

/************************************************

 ************************************************/
QStringList OutFormat_Flac::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;

    args << "--force";  // Force overwriting of output files.
    args << "--silent"; // Suppress progress indicator

    // Settings .................................................
    // Compression parametr really looks like --compression-level-N
    args << QStringLiteral("--compression-level-%1").arg(profile.encoderValue("Compression").toString());

    args << "-";
    args << "-o" << outFile;
    return args;
}

/************************************************

************************************************/
MetadataWriter *OutFormat_Flac::createMetadataWriter(const QString &filePath) const
{
    return new FlacMetadataWriter(filePath);
}

/************************************************

 ************************************************/
ConfigPage_Flac::ConfigPage_Flac(QWidget *parent) :
    EncoderConfigPage(parent)
{
    setupUi(this);

    setLosslessToolTip(flacCompressionSlider);
    flacCompressionSpin->setToolTip(flacCompressionSlider->toolTip());
}

/************************************************

 ************************************************/
void ConfigPage_Flac::load(const Profile &profile)
{
    loadWidget(profile, "Compression", flacCompressionSlider);
}

/************************************************

 ************************************************/
void ConfigPage_Flac::save(Profile *profile)
{
    saveWidget(profile, "Compression", flacCompressionSlider);
}
