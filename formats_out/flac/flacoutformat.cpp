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
#include "flacencoder.h"
#include <QDebug>

static constexpr int MATAFLAC_MAX_SAMPLE_RATE = 192 * 1000;

/************************************************
 *
 ************************************************/
OutFormat_Flac::OutFormat_Flac()
{
    mId      = "FLAC";
    mExt     = "flac";
    mName    = "Flac";
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

    for (int i = 0; i < project->count(); ++i) {
        const Disc *const disc = project->disc(i);

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
Conv::Encoder *OutFormat_Flac::createEncoder() const
{
    return new FlacEncoder();
}

/************************************************

 ************************************************/
Conv::Gain *OutFormat_Flac::createGain(const Profile &profile) const
{
    return new Gain_Flac(profile);
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

/************************************************
 *
 ************************************************/
QStringList Gain_Flac::programArgs(const QStringList &files, const GainType) const
{
    QStringList args;
    args << programPath();
    args << "--add-replay-gain";
    args << files;

    return args;
}
