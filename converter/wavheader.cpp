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

using namespace Conv;

static const char *WAV_RIFF = "RIFF";
static const char *WAV_WAVE = "WAVE";
static const char *WAV_FMT  = "fmt ";
static const char *WAV_DATA = "data";

static const int READ_DELAY = 1000;

/************************************************
 *
 ************************************************/
inline bool mustRead(QIODevice *device, char *data, qint64 size, int msecs = READ_DELAY)
{
    char * d    = data;
    qint64 left = size;
    while (left > 0) {
        device->bytesAvailable() || device->waitForReadyRead(msecs);
        qint64 n = device->read(d, left);
        if (n < 0)
            return false;

        d += n;
        left -= n;
    }

    return true;
}

/************************************************
 *
 ************************************************/
QByteArray &operator<<(QByteArray &out, const char val[4])
{
    out += val;
    return out;
}

/************************************************
 *
 ************************************************/
QByteArray &operator<<(QByteArray &out, quint32 val)
{
    union {
        quint32 n;
        char    bytes[4];
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
QByteArray &operator<<(QByteArray &out, quint16 val)
{
    union {
        quint32 n;
        char    bytes[2];
    };

    n = qToLittleEndian(val);
    out += bytes[0];
    out += bytes[1];

    return out;
}

/************************************************
 *
 ************************************************/
bool readTag(QIODevice *device, char tag[5])
{
    tag[4] = '\0';
    return mustRead(device, tag, 4);
}

/************************************************
 *
 ************************************************/
struct SplitterError
{
    int     trackNum;
    QString msg;

    SplitterError(int num, QString msg) :
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
    if (stream->read((char *)&n, 4) != 4)
        throw FlaconError("Unexpected end of file");
    return qFromLittleEndian(n);
}

/************************************************
 *
 ************************************************/
quint16 readUInt16(QIODevice *stream)
{
    quint16 n;
    if (stream->read((char *)&n, 2) != 2)
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
WavHeader::WavHeader() :
    mFileSize(0),
    mFmtSize(0),
    mFormat(WavHeader::Format_Unknown),
    mNumChannels(0),
    mSampleRate(0),
    mByteRate(0),
    mBlockAlign(0),
    mBitsPerSample(0),
    mExtSize(0),
    mValidBitsPerSample(0),
    mChannelMask(0),
    mDataSize(0),
    mDataStartPos(0)
{
}

/************************************************
 * See WAV specoification
 *   http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
 *   https://en.wikipedia.org/wiki/WAV
 ************************************************/
WavHeader::WavHeader(QIODevice *stream) :
    mFormat(WavHeader::Format_Unknown),
    mNumChannels(0),
    mSampleRate(0),
    mByteRate(0),
    mBlockAlign(0),
    mBitsPerSample(0),
    mExtSize(0),
    mValidBitsPerSample(0),
    mChannelMask(0),
    mDataSize(0),
    mDataStartPos(0)
{
    char tag[5] = { '\0' };
    // look for "RIFF" in header
    if (!readTag(stream, tag) || strcmp(tag, WAV_RIFF) != 0)
        throw FlaconError("WAVE header is missing RIFF tag while processing file");

    this->mFileSize = readUInt32(stream) + 8;

    if (!readTag(stream, tag) || strcmp(tag, WAV_WAVE) != 0)
        throw FlaconError("WAVE header is missing WAVE tag while processing file");

    char    chunkID[5];
    quint64 pos = 12;
    while (pos < this->mFileSize) {
        if (!readTag(stream, chunkID))
            throw FlaconError("[WAV] can't read chunk ID");

        quint32 chunkSize = readUInt32(stream);
        pos += 8;

        if (strcmp(chunkID, WAV_DATA) == 0) {
            this->mDataSize     = chunkSize;
            this->mDataStartPos = pos;
            return;
        }

        if (chunkSize < 1) {
            throw FlaconError(QString("[WAV] incorrect chunk size %1 at %2").arg(chunkSize).arg(pos - 4));
        }

        if (strcmp(chunkID, WAV_FMT) == 0) {
            loadFmtChunk(stream, chunkSize);

            pos += chunkSize;
        }
        else {
            mOtherCunks << chunkID;
            mOtherCunks << chunkSize;
            mOtherCunks.append(stream->read(chunkSize));
            pos += chunkSize;
        }
    }

    throw FlaconError("data chunk not found");
}

/************************************************
 *
 ************************************************/
WavHeader::WavHeader(const WavHeader &other)
{
    this->operator=(other);
}

/************************************************
 *
 ************************************************/
WavHeader &WavHeader::operator=(const WavHeader &other)
{
    mFileSize           = other.mFileSize;
    mFmtSize            = other.mFmtSize;
    mFormat             = other.mFormat;
    mNumChannels        = other.mNumChannels;
    mSampleRate         = other.mSampleRate;
    mByteRate           = other.mByteRate;
    mBlockAlign         = other.mBlockAlign;
    mBitsPerSample      = other.mBitsPerSample;
    mExtSize            = other.mExtSize;
    mValidBitsPerSample = other.mValidBitsPerSample;
    mChannelMask        = other.mChannelMask;
    mDataSize           = other.mDataSize;
    mDataStartPos       = other.mDataStartPos;
    mSubFormat          = other.mSubFormat;
    mOtherCunks         = other.mOtherCunks;

    return *this;
}

/************************************************
 *
 ************************************************/
bool WavHeader::isCdQuality() const
{
    static const int CD_NUM_CHANNELS    = 2;
    static const int CD_BITS_PER_SAMPLE = 16;
    static const int CD_SAMPLE_RATE     = 44100;
    static const int CD_BYTE_RATE       = 176400;

    return mNumChannels == CD_NUM_CHANNELS && mBitsPerSample == CD_BITS_PER_SAMPLE && mSampleRate == CD_SAMPLE_RATE && mByteRate == CD_BYTE_RATE;
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
quint32 WavHeader::bytesPerSecond(WavHeader::Quality quality)
{
    switch (quality) {
        case Quality_Stereo_CD:
            return 2 * 16 * 44100 / 8;
        case Quality_Stereo_24_96:
            return 2 * 24 * 96000 / 8;
        case Quality_Stereo_24_192:
            return 2 * 24 * 192000 / 8;
    }
    return 0;
}

/************************************************
 *
 ************************************************/
quint32 WavHeader::bytesPerSecond()
{
    return mNumChannels * mBitsPerSample * mSampleRate / 8;
}

/************************************************
 *
 ************************************************/
void checkFormat(quint16 format)
{

    switch (format) {
        case WavHeader::Format_Unknown:
        case WavHeader::Format_PCM:
        case WavHeader::Format_ADPCM:
        case WavHeader::Format_IEEE_FLOAT:
        case WavHeader::Format_ALAW:
        case WavHeader::Format_MULAW:
        case WavHeader::Format_OKI_ADPCM:
        case WavHeader::Format_IMA_ADPCM:
        case WavHeader::Format_DIGISTD:
        case WavHeader::Format_DIGIFIX:
        case WavHeader::Format_DOLBY_AC2:
        case WavHeader::Format_GSM610:
        case WavHeader::Format_ROCKWELL_ADPCM:
        case WavHeader::Format_ROCKWELL_DIGITALK:
        case WavHeader::Format_G721_ADPCM:
        case WavHeader::Format_G728_CELP:
        case WavHeader::Format_MPEG:
        case WavHeader::Format_MPEGLAYER3:
        case WavHeader::Format_G726_ADPCM:
        case WavHeader::Format_G722_ADPCM:
        case WavHeader::Format_Extensible:
            return;
    }

    throw FlaconError(QString("unknown format (%1) in WAVE header").arg(format, 0, 16));
}

/************************************************
 *
 ************************************************/
void WavHeader::loadFmtChunk(QIODevice *stream, const quint32 chunkSize)
{
    if (chunkSize != 16 && chunkSize != 18 && chunkSize != 40)
        throw FlaconError("fmt chunk in WAVE header hase incorrect length");

    mFmtSize = chunkSize;

    quint16 format = readUInt16(stream);

    this->mFormat = static_cast<Format>(format);
    checkFormat(format);
    this->mNumChannels   = readUInt16(stream);
    this->mSampleRate    = readUInt32(stream);
    this->mByteRate      = readUInt32(stream);
    this->mBlockAlign    = readUInt16(stream);
    this->mBitsPerSample = readUInt16(stream);

    if (chunkSize == 16)
        return;

    mExtSize = readUInt16(stream); // Size of the extension:
    if (chunkSize == 18)
        return;

    if (mExtSize != 22)
        throw FlaconError("Size of the extension in WAVE header hase incorrect length");

    mValidBitsPerSample = readUInt16(stream); // at most 8*M
    mChannelMask        = readUInt32(stream); // Speaker position mask
    mSubFormat          = stream->read(16);   // GUID (first two bytes are the data format code)
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
    res.reserve(mDataStartPos - 1);
    res << WAV_RIFF;
    res << (mFileSize - 8);
    res << WAV_WAVE;

    res << WAV_FMT;
    res << mFmtSize;
    res << (quint16)(mFormat);
    res << mNumChannels;
    res << mSampleRate;
    res << mByteRate;
    res << mBlockAlign;
    res << mBitsPerSample;

    if (mExtSize) {
        res << mExtSize;
        res << mValidBitsPerSample;
        res << mChannelMask;
        res.append(mSubFormat);
    }

    res.append(mOtherCunks);

    res << WAV_DATA;
    res << mDataSize;

    return res;
}

/************************************************
 *
 ************************************************/
void WavHeader::resizeData(quint32 dataSize)
{
    mDataSize = dataSize;
    mFileSize = mDataStartPos + mDataSize;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const WavHeader &header)
{
    QString format;
    switch (header.format()) {
        case WavHeader::Format_Unknown:
            format = "Unknown";
            break;
        case WavHeader::Format_PCM:
            format = "PCM";
            break;
        case WavHeader::Format_ADPCM:
            format = "ADPCM";
            break;
        case WavHeader::Format_IEEE_FLOAT:
            format = "IEEE_FLOAT";
            break;
        case WavHeader::Format_ALAW:
            format = "ALAW";
            break;
        case WavHeader::Format_MULAW:
            format = "MULAW";
            break;
        case WavHeader::Format_OKI_ADPCM:
            format = "OKI_ADPCM";
            break;
        case WavHeader::Format_IMA_ADPCM:
            format = "IMA_ADPCM";
            break;
        case WavHeader::Format_DIGISTD:
            format = "DIGISTD";
            break;
        case WavHeader::Format_DIGIFIX:
            format = "DIGIFIX";
            break;
        case WavHeader::Format_DOLBY_AC2:
            format = "DOLBY_AC2";
            break;
        case WavHeader::Format_GSM610:
            format = "Unknown";
            break;
        case WavHeader::Format_ROCKWELL_ADPCM:
            format = "ROCKWELL_ADPCM";
            break;
        case WavHeader::Format_ROCKWELL_DIGITALK:
            format = "ROCKWELL_DIGITALK";
            break;
        case WavHeader::Format_G721_ADPCM:
            format = "G721_ADPCM";
            break;
        case WavHeader::Format_G728_CELP:
            format = "G728_CELP";
            break;
        case WavHeader::Format_MPEG:
            format = "MPEG";
            break;
        case WavHeader::Format_MPEGLAYER3:
            format = "MPEGLAYER3";
            break;
        case WavHeader::Format_G726_ADPCM:
            format = "G726_ADPCM";
            break;
        case WavHeader::Format_G722_ADPCM:
            format = "G722_ADPCM";
            break;
        case WavHeader::Format_Extensible:
            format = "Extensible";
            break;
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
