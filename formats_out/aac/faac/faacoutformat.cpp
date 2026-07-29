#include "faacoutformat.h"
#include "../../extprogram.h"
#include "faacconfigpage.h"

QHash<QString, QVariant> FaacOutFormat::defaultParameters() const
{
    QHash<QString, QVariant> res;
    res.insert("UseQuality", true);
    res.insert("Quality", 100);
    res.insert("Bitrate", 256);
    return res;
}

EncoderConfigPage *FaacOutFormat::configPage(QWidget *parent) const
{
    return new FaacConfigPage(parent);
}

ExtProgram *FaacOutFormat::encoderProgram(const Profile &) const
{
    return ExtProgram::faac();
}

QStringList FaacOutFormat::encoderArgs(const Profile &profile, const QString &outFile) const
{
    QStringList args;

    args << "-w"; // Wrap  AAC  data  in  an MP4 container.

    // Quality settings .........................................
    if (profile.encoderValues()->value("UseQuality").toBool())
        args << "-q" << profile.encoderValues()->value("Quality").toString();
    else
        args << "-b" << profile.encoderValues()->value("Bitrate").toString();

    args << "-o" << outFile;
    args << "-";
    return args;
}
