#include "newsplitter.h"
#include <QIODevice>
#include <QDebug>
#include <QtEndian>
#include <QString>

#define WAV_RIFF  "RIFF"
#define WAV_WAVE  "WAVE"
#define WAV_FMT   "fmt "
#define WAV_DATA  "data"

#define CD_NUM_CHANNELS     2
#define CD_BITS_PER_SAMPLE  16
#define CD_SAMPLE_RATE      44100
#define CD_BYTE_RATE        176400
#define CD_BLOCK_SIZE       2352

#define CANONICAL_HEADER_SIZE 44

#define BUF_SIZE            4096

QByteArray readTag(QIODevice *stream)
{
    QByteArray res = stream->read(4);
    if (res.length() != 4)
        throw "Unexpected end of file";

    return res;
}


quint32 readUInt32(QIODevice *stream)
{
    quint32 n;
    if (stream->read((char*)&n, 4) != 4)
        throw "Unexpected end of file";
    return qFromLittleEndian(n);
}

quint16 readUInt16(QIODevice *stream)
{
    quint16 n;
    if (stream->read((char*)&n, 2) != 2)
        throw "Unexpected end of file";
    return qFromLittleEndian(n);
}


WavHeader::WavHeader():
    mFileSize(0),
    mFormat(WavHeader::Format_Unknown),
    mNumChannels(0),
    mSampleRate(0),
    mByteRate(0),
    mBlockAlign(0),
    mBitsPerSample(0),
    mDataSize(0),
    mDataStartPos(0)
{
}

bool WavHeader::cdQuality() const
{
    return mNumChannels   == CD_NUM_CHANNELS &&
           mBitsPerSample == CD_BITS_PER_SAMPLE &&
           mSampleRate    == CD_SAMPLE_RATE &&
           mByteRate      == CD_BYTE_RATE;
}


StdWavHeader::StdWavHeader(quint32 dataSize, const WavHeader &base):
    WavHeader()
{
    mDataSize      = dataSize;
    mDataStartPos  = CANONICAL_HEADER_SIZE;
    mFileSize      = mDataStartPos + mDataSize;
    mFormat        = base.format();
    mNumChannels   = base.numChannels();
    mSampleRate    = base.sampleRate();
    mByteRate      = base.byteRate();
    mBlockAlign    = base.blockAlign();
    mBitsPerSample = base.bitsPerSample();
}



/************************************************
 * See WAV specoification
 *   http://soundfile.sapp.org/doc/WaveFormat/
 *   https://en.wikipedia.org/wiki/WAV
 ************************************************/
void WavHeader::load(QIODevice *stream)
{
    // look for "RIFF" in header
    if (readTag(stream) != WAV_RIFF)
        throw "WAVE header is missing RIFF tag while processing file";

    this->mFileSize = readUInt32(stream) + 8;

    if (readTag(stream) != WAV_WAVE)
        throw "WAVE header is missing WAVE tag while processing file";

    QByteArray chunkID;
    quint32    chunkSize;

    while (!stream->atEnd())
    {
        chunkID   = readTag(stream);
        chunkSize = readUInt32(stream);
        //qDebug()<< QString("found chunk: [%1] with length %2").arg(chunkID.data()).arg(chunkSize);

        if (chunkID == WAV_FMT)
        {
            if (chunkSize < 16)
                throw "fmt chunk in WAVE header was too short";

            this->mFormat        = static_cast<Format>(readUInt16(stream));
            this->mNumChannels   = readUInt16(stream);
            this->mSampleRate    = readUInt32(stream);
            this->mByteRate      = readUInt32(stream);
            this->mBlockAlign    = readUInt16(stream);
            this->mBitsPerSample = readUInt16(stream);

            if (chunkSize > 16)
                stream->seek(stream->pos() + chunkSize - 16);

        }

        else if (chunkID == WAV_DATA)
        {
            this->mDataSize = chunkSize;
            this->mDataStartPos = stream->pos();
            return;
        }

        else
        {
            //qDebug() << "  skip chunk";
            stream->seek(stream->pos() + chunkSize);
        }
    }

    throw "data chunk not found";
}

QByteArray& operator<<(QByteArray& out, const char val[4])
{
    out += val;
    return out;
}

QByteArray& operator<<(QByteArray& out, quint32 val)
{
    union {
        quint32 n;
        char bytes[4];
    };

    n = qToLittleEndian(val);
    out += bytes[0];
    out += bytes[1];
    out += bytes[2];
    out += bytes[3];

    return out;
}

QByteArray& operator<<(QByteArray& out, quint16 val)
{
    union {
        quint32 n;
        char bytes[2];
    };

    n = qToLittleEndian(val);
    out += bytes[0];
    out += bytes[1];

    return out;
}

/************************************************
 * 52 49 46 46      RIFF
 * 24 B9 4D 02      file size - 8
 * 57 41 56 45      WAVE
 *
 * // Chanks
 *   66 6D 74 20    SubchunkID      "fmt "
 *   10 00 00 00    SubchunkSize        16
 *         01 00      AudioFormat      PCM
 *         02 00	  NumChannels        2
 *   44 AC 00 00      SampleRate     44100
 *   10 B1 02 00      ByteRate      176400
 *         04 00      BlockAlign         4
 *         10 00      BitsPerSample     16
 * // Data
 *   64 61 74 61 	SubchunkID 		"data"
 *   00 B9 4D 02 	SubchunkSize
 ************************************************/
QByteArray WavHeader::toByteArray() const
{
    QByteArray res;
    res.reserve(mDataStartPos -1);
    res << WAV_RIFF;
    res << (mFileSize - 8);
    res << WAV_WAVE;

    res << WAV_FMT;
    res << quint32(16);
    res << (quint16)(mFormat);
    res << mNumChannels;
    res << mSampleRate;
    res << mByteRate;
    res << mBlockAlign;
    res << mBitsPerSample;
    res << WAV_DATA;
    res << mDataSize;

    return res;
}


void skip(QIODevice *stream, quint64 bytes)
{
    quint64 pos = stream->pos();
    stream->read(bytes);
    if (stream->pos() - pos != bytes)
        throw "Can't skip in the stream";
}

SplitterNew::SplitterNew(QObject *parent) :
    QObject(parent)
{
}

void SplitterNew::setInputStream(QIODevice *inputStream)
{
    mInputStream = inputStream;
}

void SplitterNew::addTrack(SplitterNew::OutTrack track)
{
    mOutTracks << track;
}

void SplitterNew::addTrack(const CueTime &start, const CueTime &end, QIODevice *stream)
{
    OutTrack track;
    track.Start  = start;
    track.End    = end;
    track.Stream = stream;

    addTrack(track);
}

int timeToBytes(CueTime time, WavHeader wav)
{
    if (time.type() == CueTime::TypeFrames)
    {
        return (int)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else
    {
        return (int)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}

void SplitterNew::run()
{
    mHeader.load(mInputStream);


    QByteArray buf;
    for (int i=0; i<mOutTracks.count(); ++i)
    {
        OutTrack track = mOutTracks.at(i);
        quint32 start = timeToBytes(track.Start, mHeader);
        quint32 end   = timeToBytes(track.End, mHeader);

        track.Stream->write(StdWavHeader(end - start, mHeader).toByteArray());

        skip(mInputStream, mHeader.dataStartPos() + start - mInputStream->pos());

        int remain = end - start;
        if (remain < 0)
            throw "Incorrect track start or end";

        while (remain > 0)
        {
            int n = qMin(BUF_SIZE, remain);
            buf = mInputStream->read(n);
            if (buf.length() != n)
                throw "Unexpected end of input stream";

            remain -= n;
            int w = track.Stream->write(buf);
            if (w != n)
                throw "Can't write to output stream";
        }
    }

    emit finished();
}








