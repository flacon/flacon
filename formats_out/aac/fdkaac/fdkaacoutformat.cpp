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
