/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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

#include "textcodec.h"
#include <iconv.h>
#include <QDebug>
#include "types.h"

QList<int> TextCodec::availableMibs()
{
    return {
        TextCodecUtf8::MIB,
        TextCodecUtf16Be::MIB,
        TextCodecUtf16Le::MIB,
        TextCodecIso8859_1::MIB,
        TextCodecIso8859_2::MIB,
        TextCodecIso8859_3::MIB,
        TextCodecIso8859_4::MIB,
        TextCodecIso8859_5::MIB,
        TextCodecIso8859_6::MIB,
        TextCodecIso8859_7::MIB,
        TextCodecIso8859_8::MIB,
        TextCodecIso8859_9::MIB,
        TextCodecIso8859_10::MIB,
        TextCodecIso8859_13::MIB,
        TextCodecIso8859_14::MIB,
        TextCodecIso8859_15::MIB,
        TextCodecIso8859_16::MIB,
        TextCodecGb18030::MIB,
        TextCodecBig5::MIB,
        TextCodecIbm866::MIB,
        TextCodecWindows1250::MIB,
        TextCodecWindows1251::MIB,
        TextCodecWindows1252::MIB,
        TextCodecWindows1253::MIB,
        TextCodecWindows1254::MIB,
        TextCodecWindows1255::MIB,
        TextCodecWindows1256::MIB,
        TextCodecWindows1257::MIB,
        TextCodecWindows1258::MIB,
        TextCodecShiftJis::MIB,
    };
}

TextCodec TextCodec::codecForName(const QString &name)
{
    QString s = name.toUpper();

    // clang-format off
    if (s == QString(TextCodecUtf8::NAME).toUpper())         return TextCodecUtf8();
    if (s == QString(TextCodecUtf16Be::NAME).toUpper())      return TextCodecUtf16Be();
    if (s == QString(TextCodecUtf16Le::NAME).toUpper())      return TextCodecUtf16Le();
    if (s == QString(TextCodecIso8859_1::NAME).toUpper())    return TextCodecIso8859_1();
    if (s == QString(TextCodecIso8859_2::NAME).toUpper())    return TextCodecIso8859_2();
    if (s == QString(TextCodecIso8859_3::NAME).toUpper())    return TextCodecIso8859_3();
    if (s == QString(TextCodecIso8859_4::NAME).toUpper())    return TextCodecIso8859_4();
    if (s == QString(TextCodecIso8859_5::NAME).toUpper())    return TextCodecIso8859_5();
    if (s == QString(TextCodecIso8859_6::NAME).toUpper())    return TextCodecIso8859_6();
    if (s == QString(TextCodecIso8859_7::NAME).toUpper())    return TextCodecIso8859_7();
    if (s == QString(TextCodecIso8859_8::NAME).toUpper())    return TextCodecIso8859_8();
    if (s == QString(TextCodecIso8859_9::NAME).toUpper())    return TextCodecIso8859_9();
    if (s == QString(TextCodecIso8859_10::NAME).toUpper())   return TextCodecIso8859_10();
    if (s == QString(TextCodecIso8859_13::NAME).toUpper())   return TextCodecIso8859_13();
    if (s == QString(TextCodecIso8859_14::NAME).toUpper())   return TextCodecIso8859_14();
    if (s == QString(TextCodecIso8859_15::NAME).toUpper())   return TextCodecIso8859_15();
    if (s == QString(TextCodecIso8859_16::NAME).toUpper())   return TextCodecIso8859_16();
    if (s == QString(TextCodecGb18030::NAME).toUpper())      return TextCodecGb18030();
    if (s == QString(TextCodecBig5::NAME).toUpper())         return TextCodecBig5();
    if (s == QString(TextCodecIbm866::NAME).toUpper())       return TextCodecIbm866();
    if (s == QString(TextCodecWindows1250::NAME).toUpper())  return TextCodecWindows1250();
    if (s == QString(TextCodecWindows1251::NAME).toUpper())  return TextCodecWindows1251();
    if (s == QString(TextCodecWindows1252::NAME).toUpper())  return TextCodecWindows1252();
    if (s == QString(TextCodecWindows1253::NAME).toUpper())  return TextCodecWindows1253();
    if (s == QString(TextCodecWindows1254::NAME).toUpper())  return TextCodecWindows1254();
    if (s == QString(TextCodecWindows1255::NAME).toUpper())  return TextCodecWindows1255();
    if (s == QString(TextCodecWindows1256::NAME).toUpper())  return TextCodecWindows1256();
    if (s == QString(TextCodecWindows1257::NAME).toUpper())  return TextCodecWindows1257();
    if (s == QString(TextCodecWindows1258::NAME).toUpper())  return TextCodecWindows1258();
    if (s == QString(TextCodecShiftJis::NAME).toUpper())     return TextCodecShiftJis();
    // clang-format on

    return TextCodec();
}

TextCodec TextCodec::codecForMib(int mib)
{
    // clang-format off
    switch (mib) {
        case TextCodecUtf8::MIB:        return TextCodecUtf8();
        case TextCodecUtf16Be::MIB:     return TextCodecUtf16Be();
        case TextCodecUtf16Le::MIB:     return TextCodecUtf16Le();
        case TextCodecIso8859_1::MIB:   return TextCodecIso8859_1();
        case TextCodecIso8859_2::MIB:   return TextCodecIso8859_2();
        case TextCodecIso8859_3::MIB:   return TextCodecIso8859_3();
        case TextCodecIso8859_4::MIB:   return TextCodecIso8859_4();
        case TextCodecIso8859_5::MIB:   return TextCodecIso8859_5();
        case TextCodecIso8859_6::MIB:   return TextCodecIso8859_6();
        case TextCodecIso8859_7::MIB:   return TextCodecIso8859_7();
        case TextCodecIso8859_8::MIB:   return TextCodecIso8859_8();
        case TextCodecIso8859_9::MIB:   return TextCodecIso8859_9();
        case TextCodecIso8859_10::MIB:  return TextCodecIso8859_10();
        case TextCodecIso8859_13::MIB:  return TextCodecIso8859_13();
        case TextCodecIso8859_14::MIB:  return TextCodecIso8859_14();
        case TextCodecIso8859_15::MIB:  return TextCodecIso8859_15();
        case TextCodecIso8859_16::MIB:  return TextCodecIso8859_16();
        case TextCodecGb18030::MIB:     return TextCodecGb18030();
        case TextCodecBig5::MIB:        return TextCodecBig5();
        case TextCodecIbm866::MIB:      return TextCodecIbm866();
        case TextCodecWindows1250::MIB: return TextCodecWindows1250();
        case TextCodecWindows1251::MIB: return TextCodecWindows1251();
        case TextCodecWindows1252::MIB: return TextCodecWindows1252();
        case TextCodecWindows1253::MIB: return TextCodecWindows1253();
        case TextCodecWindows1254::MIB: return TextCodecWindows1254();
        case TextCodecWindows1255::MIB: return TextCodecWindows1255();
        case TextCodecWindows1256::MIB: return TextCodecWindows1256();
        case TextCodecWindows1257::MIB: return TextCodecWindows1257();
        case TextCodecWindows1258::MIB: return TextCodecWindows1258();
        case TextCodecShiftJis::MIB:    return TextCodecShiftJis();
    }
    // clang-format on

    return TextCodec();
}

TextCodec::TextCodec(int mib, const QString &name) :
    mMib(mib),
    mName(name)
{
}

bool TextCodec::operator==(const TextCodec &other) const
{
    return mMib == other.mib();
}

QString TextCodec::decode(const QByteArray &data) const noexcept(false)
{
    if (mMib == TextCodecUtf8::MIB) {
        return QString::fromUtf8(data);
    }

    iconv_t cd = iconv_open("UTF-16", mName.toLatin1().constData());
    if (cd == (iconv_t)-1) {
        throw FlaconError(QString("Unable to open iconv_open for %1: %2").arg(mName, strerror(errno)));
    }

    // int discardIlegalSequence = 1;
    // if (iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &discardIlegalSequence) == -1) {
    //     iconv_close(cd);
    //     throw FlaconError(QString("Unable to set ICONV_SET_DISCARD_ILSEQ flag %1: %2").arg(mName, strerror(errno)));
    // }

    QByteArray out;
    out.fill('\0', data.length() * 4);

    char  *outBuffer    = out.data();
    size_t outBytesLeft = out.length();

    char  *in          = (char *)data.data();
    size_t inBytesLeft = data.length();

    // size_t ok = iconv(cd, &in, &inBytesLeft, &outBuffer, &outBytesLeft);
    //  if (ok == (size_t)-1) {
    //    iconv_close(cd);
    //    throw FlaconError(QString("Unable to decode for %1: %2 at %3").arg(mName, strerror(errno)).arg(int(in - data.data())));
    //  }

    iconv(cd, &in, &inBytesLeft, &outBuffer, &outBytesLeft);
    iconv_close(cd);

    return QString::fromUtf16((const char16_t *)(out.constData()));
}

TextCodecUtf8::TextCodecUtf8() :
    TextCodec(MIB, NAME)
{
}

TextCodecUtf16Be::TextCodecUtf16Be() :
    TextCodec(MIB, NAME)
{
}

TextCodecUtf16Le::TextCodecUtf16Le() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_1::TextCodecIso8859_1() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_2::TextCodecIso8859_2() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_3::TextCodecIso8859_3() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_4::TextCodecIso8859_4() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_5::TextCodecIso8859_5() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_6::TextCodecIso8859_6() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_7::TextCodecIso8859_7() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_8::TextCodecIso8859_8() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_9::TextCodecIso8859_9() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_10::TextCodecIso8859_10() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_13::TextCodecIso8859_13() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_14::TextCodecIso8859_14() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_15::TextCodecIso8859_15() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso8859_16::TextCodecIso8859_16() :
    TextCodec(MIB, NAME)
{
}

TextCodecGb18030::TextCodecGb18030() :
    TextCodec(MIB, NAME)
{
}

TextCodecBig5::TextCodecBig5() :
    TextCodec(MIB, NAME)
{
}

TextCodecIbm866::TextCodecIbm866() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1250::TextCodecWindows1250() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1251::TextCodecWindows1251() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1252::TextCodecWindows1252() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1253::TextCodecWindows1253() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1254::TextCodecWindows1254() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1255::TextCodecWindows1255() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1256::TextCodecWindows1256() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1257::TextCodecWindows1257() :
    TextCodec(MIB, NAME)
{
}

TextCodecWindows1258::TextCodecWindows1258() :
    TextCodec(MIB, NAME)
{
}

TextCodecShiftJis::TextCodecShiftJis() :
    TextCodec(MIB, NAME)
{
}
