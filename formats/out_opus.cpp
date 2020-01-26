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


#include "out_opus.h"
#include "settings.h"
#include <QDebug>

static const constexpr char* BITRATE_TYPE_KEY = "BitrateType";
static const constexpr char* BITRATE_KEY      = "Bitrate";

/************************************************

 ************************************************/
OutFormat_Opus::OutFormat_Opus()
{
    mId   = "OPUS";
    mExt  = "opus";
    mName = "Opus";
}


/************************************************

 ************************************************/
QStringList OutFormat_Opus::encoderArgs(const Profile &profile, const Track *track, const QString &outFile) const
{
    QStringList args;

    args << Settings::i()->programName(encoderProgramName());

    args << "--quiet";

    QString type = profile.value(BITRATE_TYPE_KEY).toString();
    if (type == "VBR")
        args << "--vbr";

    if (type == "CBR")
        args << "--cvbr";

    args << "--bitrate" << profile.value(BITRATE_KEY).toString();

    // Tags .....................................................
    if (!track->artist().isEmpty())  args << "--artist"  << track->artist();
    if (!track->album().isEmpty())   args << "--album"   << track->album();
    if (!track->genre().isEmpty())   args << "--genre"   << track->genre();
    if (!track->date().isEmpty())    args << "--date"    << track->date();
    if (!track->title().isEmpty())   args << "--title"   << track->title();
    if (!track->comment().isEmpty()) args << "--comment" << QString("COMMENT=%1").arg(track->comment());
    if (!track->diskId().isEmpty())  args << "--comment" << QString("DISCID=%1").arg(track->diskId());
    if (!track->tag(TagId::AlbumArtist).isEmpty())
    {
        args << "--comment" << QString("album_artist=%1").arg(track->tag(TagId::AlbumArtist));
    }

    args << "--comment" << QString("tracknumber=%1").arg(track->trackNum());
    args << "--comment" << QString("tracktotal=%1").arg(track->trackCount());

    args << "--comment" << QString("disc=%1").arg(track->diskNum());
    args << "--comment" << QString("discnumber=%1").arg(track->diskNum());
    args << "--comment" << QString("disctotal=%1").arg(track->diskCount());

    // Files ....................................................
    args << "-";
    args << outFile;

    return args;
}


/************************************************

 ************************************************/
QStringList OutFormat_Opus::gainArgs(const QStringList &, const GainType) const
{
    return QStringList();
}


/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Opus::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert(BITRATE_TYPE_KEY, "VBR");
    res.insert(BITRATE_KEY,      96);
    return res;
}


/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Opus::configPage(Profile *profile, QWidget *parent) const
{
    return new ConfigPage_Opus(profile, parent);
}


/************************************************

 ************************************************/
ConfigPage_Opus::ConfigPage_Opus(Profile *profile, QWidget *parent):
    EncoderConfigPage(profile, parent)
{
    setupUi(this);

    opusBitrateTypeCbx->addItem(tr("VBR - variable bitrate"),    "VBR");
    opusBitrateTypeCbx->addItem(tr("CBR - constrained bitrate"), "CBR");

    opusBitrateTypeCbx->setToolTip(toolTipCss() + opusBitrateTypeCbx->toolTip());
    opusBitrateTypeLabel->setToolTip(opusBitrateTypeCbx->toolTip());

    opusBitrateSlider->setToolTip(toolTipCss() + opusBitrateSlider->toolTip());
    opusBitrateSpin->setToolTip(opusBitrateSlider->toolTip());
    opusBitrateLabel->setToolTip(opusBitrateSlider->toolTip());
}


/************************************************

 ************************************************/
void ConfigPage_Opus::load()
{
    loadWidget(BITRATE_TYPE_KEY, opusBitrateTypeCbx);
    loadWidget(BITRATE_KEY,      opusBitrateSlider);
}


/************************************************

 ************************************************/
void ConfigPage_Opus::save()
{
    saveWidget(BITRATE_TYPE_KEY, opusBitrateTypeCbx);
    saveWidget(BITRATE_KEY,      opusBitrateSlider);
}
