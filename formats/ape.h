/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#ifndef APE_H
#define APE_H

#include "format.h"

class Format_Ape: public AudioFormat
{
public:
    virtual QString name() const { return "APE"; }
    virtual QString ext() const { return "ape"; }
    virtual bool isInputFormat() const { return true; }

    virtual QByteArray magic() const { return "MAC "; }
    virtual uint const magicOffset() const { return 0; }


    virtual QString decoderProgramName() const { return "mac"; }
    virtual QStringList decoderArgs(const QString &fileName) const;
    virtual QString filterDecoderStderr(const QString &stdErr) const;

    virtual bool isOutputFormat() const { return false; }


};

#endif // APE_H
