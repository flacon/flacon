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

#include "fdkaacoutformat.h"
#include "../extprogram.h"
#include "fdkaacconfigpage.h"

QHash<QString, QVariant> FdkAacOutFormat::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("Fdkaac/UseVBR", true);
    res.insert("Fdkaac/Quality", 5);
    res.insert("Fdkaac/Bitrate", 320);
    return res;
}

EncoderConfigPage *FdkAacOutFormat::configPage(QWidget *parent) const
{
    return new FdkaacConfigPage(parent);
}

ExtProgram *FdkAacOutFormat::encoderProgram(const Profile &) const
{
    return ExtProgram::fdkaac();
}

QStringList FdkAacOutFormat::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;
    if (profile.encoderValues()->value("Fdkaac/UseVBR").toBool()) {
        // Bitrate configuration mode.  Available VBR quality value depends on other parameters such as profile, sample rate, or number of channels.
        args << "--bitrate-mode" << profile.encoderValues()->value("Fdkaac/Quality").toString();
    }
    else {
        // Bitrate configuration mode. 0 CBR (default)
        args << "--bitrate-mode"
             << "0";

        // Target bitrate (for CBR)
        args << "--bitrate" << profile.encoderValues()->value("Fdkaac/Bitrate").toString();
    }

    args << "-o" << outFile;
    args << "-";
    return args;
}
