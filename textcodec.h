#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QByteArray>
#include <QString>
#include <QList>

class TextCodec
{

public:
    enum class BomCodec {
        Unknown  = 2,
        UTF_8    = 106,
        UTF_16BE = 1013,
        UTF_16LE = 1014,
    };

public:
    static QList<int> availableMibs();
    static TextCodec  codecForName(const QString &name);
    static TextCodec  codecForMib(int mib);

    TextCodec()                       = default;
    TextCodec(const TextCodec &other) = default;
    TextCodec &operator=(const TextCodec &other) = default;

    bool operator==(const TextCodec &other) const;
    bool operator!=(const TextCodec &other) const { return !this->operator==(other); }

    int     mib() const { return mMib; }
    QString name() const { return mName; }

    QString decode(const QByteArray &data) const noexcept(false);

    bool isValid() const { return mMib != 0; }

protected:
    TextCodec(int mib, const QString &name);

private:
    int     mMib = 0;
    QString mName;
};

class TextCodecUtf8 : public TextCodec
{
public:
    static constexpr int  MIB  = 106;
    static constexpr auto NAME = "UTF-8";

    TextCodecUtf8();
};

class TextCodecUtf16Be : public TextCodec
{
public:
    static constexpr int  MIB  = 1013;
    static constexpr auto NAME = "UTF-16BE";

    TextCodecUtf16Be();
};

class TextCodecUtf16Le : public TextCodec
{
public:
    static constexpr int  MIB  = 1014;
    static constexpr auto NAME = "UTF-16LE";

    TextCodecUtf16Le();
};

class TextCodecIso88591 : public TextCodec
{
public:
    static constexpr int  MIB  = 4;
    static constexpr auto NAME = "ISO-8859-1";

    TextCodecIso88591();
};

class TextCodecIso88592 : public TextCodec
{
public:
    static constexpr int  MIB  = 5;
    static constexpr auto NAME = "ISO-8859-2";

    TextCodecIso88592();
};

class TextCodecIso88593 : public TextCodec
{
public:
    static constexpr int  MIB  = 6;
    static constexpr auto NAME = "ISO-8859-3";

    TextCodecIso88593();
};

class TextCodecIso88594 : public TextCodec
{
public:
    static constexpr int  MIB  = 7;
    static constexpr auto NAME = "ISO-8859-4";

    TextCodecIso88594();
};

class TextCodecIso88595 : public TextCodec
{
public:
    static constexpr int  MIB  = 8;
    static constexpr auto NAME = "ISO-8859-5";

    TextCodecIso88595();
};

class TextCodecIso88596 : public TextCodec
{
public:
    static constexpr int  MIB  = 9;
    static constexpr auto NAME = "ISO-8859-6";

    TextCodecIso88596();
};

class TextCodecIso88597 : public TextCodec
{
public:
    static constexpr int  MIB  = 10;
    static constexpr auto NAME = "ISO-8859-7";

    TextCodecIso88597();
};

class TextCodecIso88598 : public TextCodec
{
public:
    static constexpr int  MIB  = 11;
    static constexpr auto NAME = "ISO-8859-8";

    TextCodecIso88598();
};

class TextCodecIso88599 : public TextCodec
{
public:
    static constexpr int  MIB  = 12;
    static constexpr auto NAME = "ISO-8859-9";

    TextCodecIso88599();
};

class TextCodecIso885910 : public TextCodec
{
public:
    static constexpr int  MIB  = 13;
    static constexpr auto NAME = "ISO-8859-10";

    TextCodecIso885910();
};

class TextCodecIso885913 : public TextCodec
{
public:
    static constexpr int  MIB  = 109;
    static constexpr auto NAME = "ISO-8859-13";

    TextCodecIso885913();
};

class TextCodecIso885914 : public TextCodec
{
public:
    static constexpr int  MIB  = 110;
    static constexpr auto NAME = "ISO-8859-14";

    TextCodecIso885914();
};

class TextCodecIso885915 : public TextCodec
{
public:
    static constexpr int  MIB  = 111;
    static constexpr auto NAME = "ISO-8859-15";

    TextCodecIso885915();
};

class TextCodecIso885916 : public TextCodec
{
public:
    static constexpr int  MIB  = 112;
    static constexpr auto NAME = "ISO-8859-16";

    TextCodecIso885916();
};

class TextCodecGb18030 : public TextCodec
{
public:
    static constexpr int  MIB  = 114;
    static constexpr auto NAME = "GB18030";

    TextCodecGb18030();
};

class TextCodecBig5 : public TextCodec
{
public:
    static constexpr int  MIB  = 2026;
    static constexpr auto NAME = "Big5";

    TextCodecBig5();
};

class TextCodecIbm866 : public TextCodec
{
public:
    static constexpr int  MIB  = 2086;
    static constexpr auto NAME = "IBM866";

    TextCodecIbm866();
};

class TextCodecWindows1250 : public TextCodec
{
public:
    static constexpr int  MIB  = 2250;
    static constexpr auto NAME = "windows-1250";

    TextCodecWindows1250();
};

class TextCodecWindows1251 : public TextCodec
{
public:
    static constexpr int  MIB  = 2251;
    static constexpr auto NAME = "windows-1251";

    TextCodecWindows1251();
};

class TextCodecWindows1252 : public TextCodec
{
public:
    static constexpr int  MIB  = 2252;
    static constexpr auto NAME = "windows-1252";

    TextCodecWindows1252();
};

class TextCodecWindows1253 : public TextCodec
{
public:
    static constexpr int  MIB  = 2253;
    static constexpr auto NAME = "windows-1253";

    TextCodecWindows1253();
};

class TextCodecWindows1254 : public TextCodec
{
public:
    static constexpr int  MIB  = 2254;
    static constexpr auto NAME = "windows-1254";

    TextCodecWindows1254();
};

class TextCodecWindows1255 : public TextCodec
{
public:
    static constexpr int  MIB  = 2255;
    static constexpr auto NAME = "windows-1255";

    TextCodecWindows1255();
};

class TextCodecWindows1256 : public TextCodec
{
public:
    static constexpr int  MIB  = 2256;
    static constexpr auto NAME = "windows-1256";

    TextCodecWindows1256();
};

class TextCodecWindows1257 : public TextCodec
{
public:
    static constexpr int  MIB  = 2257;
    static constexpr auto NAME = "windows-1257";

    TextCodecWindows1257();
};

class TextCodecWindows1258 : public TextCodec
{
public:
    static constexpr int  MIB  = 2258;
    static constexpr auto NAME = "windows-1258";

    TextCodecWindows1258();
};

class TextCodecShiftJis : public TextCodec
{
public:
    static constexpr int  MIB  = 17;
    static constexpr auto NAME = "Shift-JIS";

    TextCodecShiftJis();
};

#endif // TEXTCODEC_H
