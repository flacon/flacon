/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2026
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

#include "faacoutformat.h"
#include "../../extprogram.h"
#include "faacconfigpage.h"

QHash<QString, QVariant> FaacOutFormat::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality", true);
    res.insert("Quality", 100);
    res.insert("Bitrate", 256);
    return res;
}

EncoderConfigPage *FaacOutFormat::configPage(QWidget *parent) const
{
    return new FaacConfigPage(parent);
}

ExtProgram *FaacOutFormat::encoderProgram(const Profile &) const
{
    return ExtProgram::faac();
}

QStringList FaacOutFormat::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;

    args << "-w"; // Wrap  AAC  data  in  an MP4 container.

    // Quality settings .........................................
    if (profile.encoderValues()->value("UseQuality").toBool())
        args << "-q" << profile.encoderValues()->value("Quality").toString();
    else
        args << "-b" << profile.encoderValues()->value("Bitrate").toString();

    args << "-o" << outFile;
    args << "-";
    return args;
}
