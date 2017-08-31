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


#ifndef TTA_H
#define TTA_H

#include "format.h"

class Format_Tta: public AudioFormat
{
public:
    virtual QString name() const override { return "TTA"; }
    virtual QString ext() const override { return "tta"; }
    virtual QByteArray magic() const override { return "TTA1"; }
    virtual uint const magicOffset() const override { return 0; }


    virtual bool isInputFormat() const override { return true; }
    virtual QString decoderProgramName() const override { return "ttaenc"; }
    virtual QStringList decoderArgs(const QString &fileName) const override;
    virtual QString filterDecoderStderr(const QString &stdErr) const override;

    virtual bool isOutputFormat() const override { return false; }


};

#endif // TTA_H
