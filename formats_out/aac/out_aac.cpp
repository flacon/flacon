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

#include "out_aac.h"
#include "../metadatawriter.h"
#include "configpage_acc.h"
#include "faac/faacoutformat.h"
#include "fdkaac/fdkaacoutformat.h"

/************************************************

 ************************************************/
OutFormat_Aac::OutFormat_Aac() :
    mFaaccOutFormat(new FaacOutFormat()),
    mFdkAacOutFormat(new FdkAacOutFormat())
{
    mId      = "AAC";
    mExt     = "m4a";
    mName    = "AAC";
    mOptions = FormatOption::SupportGain | FormatOption::SupportEmbeddedImage;
}

/************************************************

 ************************************************/
OutFormat_Aac::~OutFormat_Aac()
{
    delete mFaaccOutFormat;
    delete mFdkAacOutFormat;
}

/************************************************

 ************************************************/
QHash<QString, QVariant> OutFormat_Aac::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert(mFdkAacOutFormat->defaultParameters());
    res.insert(mFaaccOutFormat->defaultParameters());
    return res;
}

/************************************************

 ************************************************/
EncoderConfigPage *OutFormat_Aac::configPage(QWidget *parent) const
{
    return new ConfigPage_Acc(parent);
}

/************************************************

 ************************************************/
ExtProgram *OutFormat_Aac::encoderProgram(const Profile &profile) const
{
    return subFormat(profile)->encoderProgram(profile);
}

/************************************************

 ************************************************/
QStringList OutFormat_Aac::encoderArgs(const Profile &profile, const QString &outFile) const
{
    return subFormat(profile)->encoderArgs(profile, outFile);
}

/************************************************
 *
 ************************************************/
MetadataWriter *OutFormat_Aac::createMetadataWriter(const Profile &profile, const QString &filePath) const
{
    return new Mp4MetaDataWriter(profile, filePath);
}

/************************************************
 *
 ************************************************/
OutFormat *OutFormat_Aac::subFormat(const Profile &profile) const
{
    bool useFdkaac = true;
    useFdkaac      = useFdkaac && profile.encoderValues()->value("Program").toString() == "fdkaac";
    useFdkaac      = useFdkaac && mFdkAacOutFormat->encoderProgram(profile)->check();

    if (useFdkaac) {
        return mFdkAacOutFormat;
    }
    else {
        return mFaaccOutFormat;
    }
}
