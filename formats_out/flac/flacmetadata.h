#ifndef FLACMETADATA_H
#define FLACMETADATA_H

#include "../metadatawriter.h"

class FlacMetadata : public MetadataWriter
{
public:
    void writeTags() const override;
};

#endif // FLACMETADATA_H
