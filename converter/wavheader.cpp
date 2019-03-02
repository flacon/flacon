/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "wavheader.h"
#include "types.h"

#include <QByteArray>
#include <QtEndian>
#include <QDebug>

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
#define BUF_SIZE              4096


/************************************************
 *
 ************************************************/
inline bool mustRead(QIODevice *device, char *data, qint64 size, int msecs)
{
    char *d = data;
    qint64 left = size;
    while (left > 0)
    {
        device->bytesAvailable() || device->waitForReadyRead(msecs);
        qint64 n = device->read(d, left);
        if (n<0)
            return false;

        d += n;
        left -= n;
    }

    return true;
}


/************************************************
 *
 ************************************************/
bool mustSkip(QIODevice *device, qint64 size, int msecs)
{
    if (size == 0)
        return true;

    char buf[BUF_SIZE];
    qint64 left = size;
    while (left > 0)
    {
        device->bytesAvailable() || device->waitForReadyRead(msecs);
        qint64 n = device->read(buf, qMin(qint64(BUF_SIZE), left));
        if (n<0)
            return false;

        left -= n;
    }

    return true;
}


/************************************************
 *
 ************************************************/
bool readTag(QIODevice *device, char tag[5])
{
    tag[4] = '\0';
    return mustRead(device, tag, 4);
}


struct SplitterError {
    int        trackNum;
    QString    msg;

    SplitterError(int num, QString msg):
        trackNum(num),
        msg(msg)
    {
    }
};


/************************************************
 *
 ************************************************/
quint32 readUInt32(QIODevice *stream)
{
    quint32 n;
    if (stream->read((char*)&n, 4) != 4)
        throw FlaconError("Unexpected end of file");
    return qFromLittleEndian(n);
}


/************************************************
 *
 ************************************************/
quint16 readUInt16(QIODevice *stream)
{
    quint16 n;
    if (stream->read((char*)&n, 2) != 2)
        throw FlaconError("Unexpected end of file");
    return qFromLittleEndian(n);
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


/************************************************
 *
 ************************************************/
bool WavHeader::isCdQuality() const
{
    return mNumChannels   == CD_NUM_CHANNELS &&
           mBitsPerSample == CD_BITS_PER_SAMPLE &&
           mSampleRate    == CD_SAMPLE_RATE &&
           mByteRate      == CD_BYTE_RATE;
}


/************************************************
 *
 ************************************************/
quint64 WavHeader::duration() const
{
    return (mDataSize * 1000ull) / mByteRate;
}


/************************************************
 *
 ************************************************/
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
 *
 ************************************************/
StdWavHeader::StdWavHeader(quint32 dataSize, quint32 sampleRate, quint16 bitsPerSample, quint8 numChannels):
    WavHeader()
{
    mDataSize      = dataSize;
    mNumChannels   = numChannels;
    mSampleRate    = sampleRate;
    mBitsPerSample = bitsPerSample;

    mDataStartPos  = CANONICAL_HEADER_SIZE;
    mFileSize      = mDataStartPos + mDataSize;
    mFormat        = Format_PCM;
    mByteRate      = mSampleRate * mNumChannels * mBitsPerSample / 8;
    mBlockAlign    =               mNumChannels * mBitsPerSample / 8;
}


/************************************************
 *
 ************************************************/
StdWavHeader::StdWavHeader(quint32 dataSize, StdWavHeader::Quality quality)
{
    mDataSize      = dataSize;

    switch (quality)
    {
    case Quality_Stereo_CD:
        mNumChannels   = 2;
        mBitsPerSample = 16;
        mSampleRate    = 44100;
        break;


    case Quality_Stereo_24_96:
        mNumChannels   = 2;
        mBitsPerSample = 24;
        mSampleRate    = 96000;
        break;

    case Quality_Stereo_24_192:
        mNumChannels   = 2;
        mBitsPerSample = 24;
        mSampleRate    = 192000;
        break;

    }

    mDataStartPos  = CANONICAL_HEADER_SIZE;
    mFileSize      = mDataStartPos + mDataSize;
    mFormat        = Format_PCM;
    mByteRate      = mSampleRate * mNumChannels * mBitsPerSample / 8;
    mBlockAlign    =               mNumChannels * mBitsPerSample / 8;

}


/************************************************
 *
 ************************************************/
quint32 StdWavHeader::bytesPerSecond(StdWavHeader::Quality quality)
{
    switch (quality)
    {
    case Quality_Stereo_CD:     return 2 * 16 *  44100 / 8;
    case Quality_Stereo_24_96:  return 2 * 24 *  96000 / 8;
    case Quality_Stereo_24_192: return 2 * 24 * 192000 / 8;
    }
    return 0;
}


/************************************************
 * See WAV specoification
 *   http://soundfile.sapp.org/doc/WaveFormat/
 *   https://en.wikipedia.org/wiki/WAV
 ************************************************/
void WavHeader::load(QIODevice *stream)
{
    char tag[5] = { '\0' };
    // look for "RIFF" in header
    if (!readTag(stream, tag) || strcmp(tag, WAV_RIFF) != 0)
        throw FlaconError("WAVE header is missing RIFF tag while processing file");

    this->mFileSize = readUInt32(stream) + 8;

    if (!readTag(stream, tag) || strcmp(tag, WAV_WAVE) != 0)
        throw FlaconError("WAVE header is missing WAVE tag while processing file");


    char    chunkID[5];
    quint64 pos=12;
    while (!stream->atEnd())
    {
        if (!readTag(stream, chunkID))
            throw FlaconError("[WAV] can't read chunk ID");

        quint32 chunkSize = readUInt32(stream);
        pos+=8;
        //qDebug()<< QString("found chunk: [%1] with length %2").arg(chunkID.data()).arg(chunkSize);

        if (strcmp(chunkID, WAV_FMT) == 0)
        {
            pos+=chunkSize;
            if (chunkSize < 16)
                throw FlaconError("fmt chunk in WAVE header was too short");

            this->mFormat        = static_cast<Format>(readUInt16(stream));
            this->mNumChannels   = readUInt16(stream);
            this->mSampleRate    = readUInt32(stream);
            this->mByteRate      = readUInt32(stream);
            this->mBlockAlign    = readUInt16(stream);
            this->mBitsPerSample = readUInt16(stream);

            if (chunkSize > 16)
                mustSkip(stream, chunkSize - 16);

        }

        else if (strcmp(chunkID, WAV_DATA) == 0)
        {
            this->mDataSize = chunkSize;
            this->mDataStartPos = pos;
            return;
        }

        else
        {
            pos+=chunkSize;
            mustSkip(stream, chunkSize);
        }
    }

    throw FlaconError("data chunk not found");
}


/************************************************
 *
 ************************************************/
QByteArray& operator<<(QByteArray& out, const char val[4])
{
    out += val;
    return out;
}


/************************************************
 *
 ************************************************/
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


/************************************************
 *
 ************************************************/
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

QDebug operator<<(QDebug dbg, const WavHeader &header)
{
    QString format;
    switch (header.format())
    {
    case WavHeader::Format_Unknown:             format = "Unknown";            break;
    case WavHeader::Format_PCM:                 format = "PCM";                break;
    case WavHeader::Format_ADPCM:               format = "ADPCM";              break;
    case WavHeader::Format_IEEE_FLOAT:          format = "IEEE_FLOAT";         break;
    case WavHeader::Format_ALAW:                format = "ALAW";               break;
    case WavHeader::Format_MULAW:               format = "MULAW";              break;
    case WavHeader::Format_OKI_ADPCM:           format = "OKI_ADPCM";          break;
    case WavHeader::Format_IMA_ADPCM:           format = "IMA_ADPCM";          break;
    case WavHeader::Format_DIGISTD:             format = "DIGISTD";            break;
    case WavHeader::Format_DIGIFIX:             format = "DIGIFIX";            break;
    case WavHeader::Format_DOLBY_AC2:           format = "DOLBY_AC2";          break;
    case WavHeader::Format_GSM610:              format = "Unknown";            break;
    case WavHeader::Format_ROCKWELL_ADPCM:      format = "ROCKWELL_ADPCM";     break;
    case WavHeader::Format_ROCKWELL_DIGITALK:   format = "ROCKWELL_DIGITALK";  break;
    case WavHeader::Format_G721_ADPCM:          format = "G721_ADPCM";         break;
    case WavHeader::Format_G728_CELP:           format = "G728_CELP";          break;
    case WavHeader::Format_MPEG:                format = "MPEG";               break;
    case WavHeader::Format_MPEGLAYER3:          format = "MPEGLAYER3";         break;
    case WavHeader::Format_G726_ADPCM:          format = "G726_ADPCM";         break;
    case WavHeader::Format_G722_ADPCM:          format = "Unknown";            break;
    }


    dbg.nospace() << "file size:       " << header.fileSize() << "\n";
    dbg.nospace() << "format:          " << format << "\n";
    dbg.nospace() << "num channels:    " << header.numChannels() << "\n";
    dbg.nospace() << "sample rate:     " << header.sampleRate() << "\n";
    dbg.nospace() << "byte rate:       " << header.byteRate() << "\n";
    dbg.nospace() << "block align:     " << header.blockAlign() << "\n";
    dbg.nospace() << "bits per sample: " << header.bitsPerSample() << "\n";
    dbg.nospace() << "data size:       " << header.dataSize() << "\n";
    dbg.nospace() << "data start pos:  " << header.dataStartPos() << "\n";

    return dbg.space();
}
