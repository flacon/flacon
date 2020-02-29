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


#include "in_wv.h"

REGISTER_INPUT_FORMAT(Format_Wv)

/************************************************
 * As I understand WavPack can be embedded as a chunk of a RIFF stream.
 * I have not such a file, if anyone has one, please send me.
 ************************************************/
bool Format_Wv::checkMagic(const QByteArray &data) const
{
    return data.contains(magic());
}


/************************************************
 *
 ************************************************/
QStringList Format_Wv::decoderArgs(const QString &fileName) const
{
    QStringList args;
    args << "-q";
    args << "-y";
    args << fileName;
    args << "-o" << "-";

    return args;
}
