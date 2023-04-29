#ifndef UCHARDETECT_H
#define UCHARDETECT_H

#include <QString>

class Track;
class TrackTags;

class UcharDet
{

public:
    UcharDet();
    UcharDet(const UcharDet &) = delete;
    UcharDet &operator=(const UcharDet &) = delete;
    ~UcharDet();

    void      add(const Track &track);
    UcharDet &operator<<(const Track &track);
    UcharDet &operator<<(const TrackTags &track);

    QString     textCodecName() const;
    QTextCodec *textCodec() const;

private:
    struct Data;
    Data *mData;
};
#endif // UCHARDETECT_H
