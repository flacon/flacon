#ifndef SPLITTER_H
#define SPLITTER_H

#include <QObject>
#include <QVector>

#include "../cue.h"

class QIODevice;


class WavHeader {
public:
    enum Format{
        Format_Unknown            = 0x0000,
        Format_PCM                = 0x0001,
        Format_ADPCM              = 0x0002,
        Format_IEEE_FLOAT         = 0x0003,
        Format_ALAW               = 0x0006,
        Format_MULAW              = 0x0007,
        Format_OKI_ADPCM          = 0x0010,
        Format_IMA_ADPCM          = 0x0011,
        Format_DIGISTD            = 0x0015,
        Format_DIGIFIX            = 0x0016,
        Format_DOLBY_AC2          = 0x0030,
        Format_GSM610             = 0x0031,
        Format_ROCKWELL_ADPCM     = 0x003b,
        Format_ROCKWELL_DIGITALK  = 0x003c,
        Format_G721_ADPCM         = 0x0040,
        Format_G728_CELP          = 0x0041,
        Format_MPEG               = 0x0050,
        Format_MPEGLAYER3         = 0x0055,
        Format_G726_ADPCM         = 0x0064,
        Format_G722_ADPCM         = 0x0065

    };


    WavHeader();

    quint32     fileSize() const      { return mFileSize; }
    Format      format() const        { return mFormat; }
    quint16     numChannels() const   { return mNumChannels; }
    quint32     sampleRate() const    { return mSampleRate; }
    quint32     byteRate() const      { return mByteRate; }
    quint16     blockAlign() const    { return mBlockAlign; }
    quint16     bitsPerSample() const { return mBitsPerSample; }
    quint32     dataSize() const      { return mDataSize; }
    quint32     dataStartPos() const  { return mDataStartPos; }
    bool        isCdQuality() const;

    void load(QIODevice* stream);

    QByteArray toByteArray() const;

protected:
    quint32 mFileSize;
    Format  mFormat;
    quint16 mNumChannels;
    quint32 mSampleRate;
    quint32 mByteRate;
    quint16 mBlockAlign;
    quint16 mBitsPerSample;
    quint32 mDataSize;
    quint32 mDataStartPos;

private:

};

class StdWavHeader: public WavHeader
{
public:
    StdWavHeader(quint32 dataSize, const WavHeader &base);
};

class SplitterNew : public QObject
{
    Q_OBJECT

public:
    struct OutTrack {
        CueTime Start;
        CueTime End;
        QIODevice* Stream;
    };

    explicit SplitterNew(QObject *parent = 0);

    QIODevice* inputStream() const { return mInputStream; }
    void setInputStream(QIODevice *inputStream);

    int outTracksCount() const { return mOutTracks.count(); }
    OutTrack outTracks(uint index) const { return mOutTracks[index]; }
    void addTrack(OutTrack track);
    void addTrack(const CueTime &start, const CueTime &end, QIODevice *stream);

    void run();

signals:
    void finished();

private:
    struct SplitInfo {
        uint Start;
        uint End;
    };

    QIODevice *mInputStream;
    QVector<OutTrack> mOutTracks;
    WavHeader mHeader;


};

#endif // SPLITTER_H
