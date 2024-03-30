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
        TextCodecIso88591::MIB,
        TextCodecIso88592::MIB,
        TextCodecIso88593::MIB,
        TextCodecIso88594::MIB,
        TextCodecIso88595::MIB,
        TextCodecIso88596::MIB,
        TextCodecIso88597::MIB,
        TextCodecIso88598::MIB,
        TextCodecIso88599::MIB,
        TextCodecIso885910::MIB,
        TextCodecIso885913::MIB,
        TextCodecIso885914::MIB,
        TextCodecIso885915::MIB,
        TextCodecIso885916::MIB,
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
    if (s == QString(TextCodecIso88591::NAME).toUpper())     return TextCodecIso88591();
    if (s == QString(TextCodecIso88592::NAME).toUpper())     return TextCodecIso88592();
    if (s == QString(TextCodecIso88593::NAME).toUpper())     return TextCodecIso88593();
    if (s == QString(TextCodecIso88594::NAME).toUpper())     return TextCodecIso88594();
    if (s == QString(TextCodecIso88595::NAME).toUpper())     return TextCodecIso88595();
    if (s == QString(TextCodecIso88596::NAME).toUpper())     return TextCodecIso88596();
    if (s == QString(TextCodecIso88597::NAME).toUpper())     return TextCodecIso88597();
    if (s == QString(TextCodecIso88598::NAME).toUpper())     return TextCodecIso88598();
    if (s == QString(TextCodecIso88599::NAME).toUpper())     return TextCodecIso88599();
    if (s == QString(TextCodecIso885910::NAME).toUpper())    return TextCodecIso885910();
    if (s == QString(TextCodecIso885913::NAME).toUpper())    return TextCodecIso885913();
    if (s == QString(TextCodecIso885914::NAME).toUpper())    return TextCodecIso885914();
    if (s == QString(TextCodecIso885915::NAME).toUpper())    return TextCodecIso885915();
    if (s == QString(TextCodecIso885916::NAME).toUpper())    return TextCodecIso885916();
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
        case TextCodecIso88591::MIB:    return TextCodecIso88591();
        case TextCodecIso88592::MIB:    return TextCodecIso88592();
        case TextCodecIso88593::MIB:    return TextCodecIso88593();
        case TextCodecIso88594::MIB:    return TextCodecIso88594();
        case TextCodecIso88595::MIB:    return TextCodecIso88595();
        case TextCodecIso88596::MIB:    return TextCodecIso88596();
        case TextCodecIso88597::MIB:    return TextCodecIso88597();
        case TextCodecIso88598::MIB:    return TextCodecIso88598();
        case TextCodecIso88599::MIB:    return TextCodecIso88599();
        case TextCodecIso885910::MIB:   return TextCodecIso885910();
        case TextCodecIso885913::MIB:   return TextCodecIso885913();
        case TextCodecIso885914::MIB:   return TextCodecIso885914();
        case TextCodecIso885915::MIB:   return TextCodecIso885915();
        case TextCodecIso885916::MIB:   return TextCodecIso885916();
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

    int discardIlegalSequence = 1;
    if (iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &discardIlegalSequence) == -1) {
        // iconv_close(cd);
        // throw FlaconError(QString("Unable to set ICONV_SET_DISCARD_ILSEQ flag %1: %2").arg(mName, strerror(errno)));
    }

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

TextCodecIso88591::TextCodecIso88591() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88592::TextCodecIso88592() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88593::TextCodecIso88593() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88594::TextCodecIso88594() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88595::TextCodecIso88595() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88596::TextCodecIso88596() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88597::TextCodecIso88597() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88598::TextCodecIso88598() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso88599::TextCodecIso88599() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso885910::TextCodecIso885910() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso885913::TextCodecIso885913() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso885914::TextCodecIso885914() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso885915::TextCodecIso885915() :
    TextCodec(MIB, NAME)
{
}

TextCodecIso885916::TextCodecIso885916() :
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
