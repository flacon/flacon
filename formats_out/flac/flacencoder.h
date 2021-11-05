#ifndef FLACENCODER_H
#define FLACENCODER_H

#include "../converter/encoder.h"

class FlacEncoder : public Conv::Encoder
{
public:
    QString     programName() const override { return "flac"; }
    QStringList programArgs() const override;
};

#endif // FLACENCODER_H
