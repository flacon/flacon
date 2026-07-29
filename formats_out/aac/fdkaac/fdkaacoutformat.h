#ifndef FDKAACOUTFORMAT_H
#define FDKAACOUTFORMAT_H

#include "../../outformat.h"

class FdkAacOutFormat : public OutFormat
{
public:
    QHash<QString, QVariant> defaultParameters() const override;
    EncoderConfigPage       *configPage(QWidget *parent) const override;

    virtual BitsPerSample maxBitPerSample() const override { return BitsPerSample::Bit_32; }
    virtual SampleRate    maxSampleRate() const override { return SampleRate::Hz_192000; }

    ExtProgram *encoderProgram(const Profile &profile) const override;
    QStringList encoderArgs(const Profile &profile, const QString &outFile) const override;

    MetadataWriter *createMetadataWriter(const Profile &, const QString &) const override { return nullptr; }
};

#endif // FDKAACOUTFORMAT_H
