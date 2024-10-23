/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2023
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

#include "uchardetect.h"
#include <uchardet.h>

/************************************************
 *
 ************************************************/
UcharDet &UcharDet::operator<<(const QByteArray &string)
{
    if (!string.isEmpty()) {
        mStrings << string;
    }
    return *this;
}

/************************************************
 *
 ************************************************/
TextCodec UcharDet::detect() const
{
    uchardet_t uchcharDet = uchardet_new();

    for (const QByteArray &s : mStrings) {
        uchardet_handle_data(uchcharDet, s.data(), s.length());
    }

    uchardet_data_end(uchcharDet);
    QString name = uchardet_get_charset(uchcharDet);
    if (name == "US-ASCII") {
        name = TextCodecUtf8::NAME;
    }

    uchardet_delete(uchcharDet);

    return TextCodec::codecForName(name);
}
