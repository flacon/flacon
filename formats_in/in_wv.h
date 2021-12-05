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

#ifndef IN_WV_H
#define IN_WV_H

#include "informat.h"

class Format_Wv : public InputFormat
{
public:
    virtual QString    name() const override { return "WavPack"; }
    virtual QString    ext() const override { return "wv"; }
    virtual QByteArray magic() const override { return "wvpk"; }
    virtual uint       magicOffset() const override { return 0; }

    virtual QString     decoderProgramName() const override { return "wvunpack"; }
    virtual QStringList decoderArgs(const QString &fileName) const override;

protected:
    virtual bool checkMagic(const QByteArray &data) const override;
};

#endif // IN_WV_H
