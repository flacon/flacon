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
#include <array>

using namespace Conv;

static const char *WAV_RIFF = "RIFF";
static const char *WAV_WAVE = "WAVE";
static const char *WAV_FMT  = "fmt ";
static const char *WAV_DATA = "data";

static const char                   *WAVE64_RIFF      = "riff";
static const char                   *WAVE64_WAVE      = "wave";
static const std::array<uint8_t, 16> WAVE64_GUID_RIFF = { 0x72, 0x69, 0x66, 0x66, 0x2E, 0x91, 0xCF, 0x11, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00 };
static const std::array<uint8_t, 16> WAVE64_GUID_WAVE = { 0x77, 0x61, 0x76, 0x65, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };
static const std::array<uint8_t, 16> WAVE64_GUID_FMT  = { 0x66, 0x6D, 0x74, 0x20, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };
static const std::array<uint8_t, 16> WAVE64_GUID_DATA = { 0x64, 0x61, 0x74, 0x61, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };

// 16 bytes of GUID + 8 bytes of Int64
static constexpr int WAVE64_CHUNK_HEADER_SIZE = 16 + 8;

static const int READ_DELAY = 1000;

/************************************************
 *
 ************************************************/
static inline void mustRead(QIODevice *device, char *data, qint64 size, int msecs = READ_DELAY)
{
    char  *d    = data;
    qint64 left = size;
    while (left > 0) {
        device->bytesAvailable() || device->waitForReadyRead(msecs);

        qint64 n = device->read(d, left);
        if (n < 0) {
            throw FlaconError(QStringLiteral("Error reading data: %1").arg(device->errorString()));
        }
        else if (n == 0) {
            if (!device->isSequential()) {
                throw FlaconError(QStringLiteral("Unexpected end of file on %1").arg(device->pos()));
            }
        }

        d += n;
        left -= n;
    }
}

/************************************************
 *
 ************************************************/
static inline QByteArray mustRead(QIODevice *device, qint64 size, int msecs = READ_DELAY)
{
    QByteArray res;
    res.resize(size);
    mustRead(device, res.data(), res.size(), msecs);
    return res;
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
QByteArray &operator<<(QByteArray &out, quint64 val)
{
    union {
        quint64 n;
        char    bytes[8];
    };

    n = qToLittleEndian(val);
    out += bytes[0];
    out += bytes[1];
    out += bytes[2];
    out += bytes[3];
    out += bytes[4];
    out += bytes[5];
    out += bytes[6];
    out += bytes[7];

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
static QByteArray &operator<<(QByteArray &out, const std::array<uint8_t, 16> &val)
{
    for (const uint8_t &b : val) {
        out += b;
    }

    return out;
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
static quint64 readUInt64(QIODevice *stream)
{
    quint64 n;
    mustRead(stream, (char *)&n, 8);
    return qFromLittleEndian(n);
}

/************************************************
 *
 ************************************************/
static quint32 readUInt32(QIODevice *stream)
{
    quint32 n;
    mustRead(stream, (char *)&n, 4);
    return qFromLittleEndian(n);
}

/************************************************
 *
 ************************************************/
static quint16 readUInt16(QIODevice *stream)
{
    quint16 n;
    mustRead(stream, (char *)&n, 2);
    return qFromLittleEndian(n);
}

/************************************************
 *
 ************************************************/
class FourCC : public std::array<char, 4>
{
public:
    FourCC() :
        std::array<char, 4>({ '\0' }) { }

    inline void load(QIODevice *device) { return mustRead(device, this->data(), this->size()); }
    inline bool operator==(const char *str) const { return strncmp(data(), str, size()) == 0; }
    inline bool operator!=(const char *str) const { return !this->operator==(str); }
};

QByteArray &operator<<(QByteArray &out, const FourCC &val)
{
    out.append(val.data(), val.size());
    return out;
}

/************************************************
 *
 ************************************************/
class Guid : public std::array<char, 16>
{
public:
    static constexpr int SIZE = 16;
    Guid() :
        std::array<char, SIZE>({ '\0' }) { }

    inline void load(QIODevice *device) { return mustRead(device, this->data(), this->size()); }
    inline bool operator==(const char *str) const { return strncmp(data(), str, size()) == 0; }
    inline bool operator!=(const char *str) const { return !this->operator==(str); }
    inline bool startsWidth(const char str[4]) const { return strncmp(data(), str, 4) == 0; }
};

/************************************************
 *
 ************************************************/
QByteArray &operator<<(QByteArray &out, const Guid &val)
{
    out.append(val.data(), val.size());
    return out;
}

/************************************************
 * See WAV specoification
 *   http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
 *   https://en.wikipedia.org/wiki/WAV
 ************************************************/
WavHeader::WavHeader(QIODevice *stream) noexcept(false)
{
    char tag[] = "\0\0\0\0";

    mustRead(stream, tag, 4);

    if (strcmp(tag, WAV_RIFF) == 0) {
        m64Bit = false;
        readWavHeader(stream);

        return;
    }

    if (strcmp(tag, WAVE64_RIFF) == 0) {
        mustRead(stream, 12); // Wave64 format uses 128-bit GUIDs, we readed 4 bytes, there are still 12 bytes
        m64Bit = true;
        readWave64Header(stream);
        return;
    }

    throw FlaconError("WAVE header is missing RIFF tag while processing file");
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
void WavHeader::readWavHeader(QIODevice *stream)
{
    this->mFileSize = readUInt32(stream) + 8;

    FourCC waveTag;
    waveTag.load(stream);

    if (waveTag != WAV_WAVE) {
        throw FlaconError("WAVE header is missing WAVE tag while processing file");
    }

    FourCC  chunkId;
    quint64 pos = 12;
    while (pos < this->mFileSize) {

        chunkId.load(stream);
        quint32 chunkSize = readUInt32(stream);
        pos += 8;

        if (chunkId == WAV_DATA) {
            this->mDataSize     = chunkSize;
            this->mDataStartPos = pos;
            return;
        }

        if (chunkSize < 1) {
            throw FlaconError(QStringLiteral("[WAV] incorrect chunk size %1 at %2").arg(chunkSize).arg(pos - 4));
        }

        if (chunkId == WAV_FMT) {
            loadFmtChunk(stream, chunkSize);
            pos += chunkSize;
        }
        else {
            mOtherCunks << chunkId;
            mOtherCunks << chunkSize;
            mOtherCunks.append(mustRead(stream, chunkSize));
            pos += chunkSize;
        }
    }

    throw FlaconError("data chunk not found");
}

/************************************************
 * All chunks are byte-aligned on 8-byte boundaries, but their
 * chunk size fields do not include any padding if it is necessary.
 ************************************************/
void WavHeader::readWave64Header(QIODevice *stream)
{
    this->mFileSize = readUInt64(stream);

    Guid waveTag;
    waveTag.load(stream);

    if (!waveTag.startsWidth(WAVE64_WAVE)) {
        throw FlaconError("WAVE64 header is missing WAVE tag while processing file");
    }

    Guid    chunkId;
    quint64 pos = 16 + 8 + 16;
    while (pos < this->mFileSize) {

        // All chunks are byte-aligned on 8-byte boundaries
        if (pos % 8) {
            char d[8];
            mustRead(stream, d, 8 - (pos % 8));
        }

        chunkId.load(stream);
        quint64 chunkSize = readUInt64(stream);
        pos += WAVE64_CHUNK_HEADER_SIZE;

        if (chunkId.startsWidth(WAV_DATA)) {
            this->mDataSize     = chunkSize - WAVE64_CHUNK_HEADER_SIZE;
            this->mDataStartPos = pos;
            return;
        }

        if (chunkSize < 1) {
            throw FlaconError(QStringLiteral("[WAV] incorrect chunk size %1 at %2").arg(chunkSize).arg(pos - 4));
        }

        if (chunkId.startsWidth(WAV_FMT)) {
            loadFmtChunk(stream, chunkSize - 16 - 8);
            pos += chunkSize - WAVE64_CHUNK_HEADER_SIZE;
        }
        else {
            mOtherCunks << chunkId;
            mOtherCunks << chunkSize;
            mOtherCunks.append(mustRead(stream, chunkSize));
            pos += chunkSize - WAVE64_CHUNK_HEADER_SIZE;
        }
    }

    throw FlaconError("data chunk not found");
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

    throw FlaconError(QStringLiteral("unknown format (%1) in WAVE header").arg(format, 0, 16));
}

/************************************************
 *
 ************************************************/
void WavHeader::loadFmtChunk(QIODevice *stream, const quint32 chunkSize)
{
    if (chunkSize != FmtChunkMin && chunkSize != FmtChunkMid && chunkSize != FmtChunkExt)
        throw FlaconError("fmt chunk in WAVE header hase incorrect length");

    mFmtSize = FmtChunkSize(chunkSize);

    quint16 format = readUInt16(stream);

    this->mFormat = static_cast<Format>(format);
    checkFormat(format);
    this->mNumChannels   = readUInt16(stream);
    this->mSampleRate    = readUInt32(stream);
    this->mByteRate      = readUInt32(stream);
    this->mBlockAlign    = readUInt16(stream);
    this->mBitsPerSample = readUInt16(stream);

    if (chunkSize == FmtChunkMin)
        return;

    mExtSize = readUInt16(stream); // Size of the extension:
    if (chunkSize == FmtChunkMid)
        return;

    if (mExtSize != FmtChunkExt - FmtChunkMid)
        throw FlaconError("Size of the extension in WAVE header hase incorrect length");

    mValidBitsPerSample = readUInt16(stream);   // at most 8*M
    mChannelMask        = readUInt32(stream);   // Speaker position mask
    mSubFormat          = mustRead(stream, 16); // GUID (first two bytes are the data format code)
}

/************************************************
 *
 ************************************************/
QByteArray WavHeader::toByteArray() const
{
    if (m64Bit) {
        return wave64ToByteArray();
    }
    else {
        return wavToByteArray(true);
    }
}

/************************************************
 *
 ************************************************/
QByteArray WavHeader::toLegacyWav() const
{
    return wavToByteArray(false);
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
QByteArray WavHeader::wavToByteArray(bool keepOtherChunks) const
{
    QByteArray res;
    res.reserve(mDataStartPos - 1);
    res << WAV_RIFF;
    res << quint32(0);
    res << WAV_WAVE;

    res << WAV_FMT;
    res << quint32(mFmtSize);
    res << quint16(mFormat);
    res << mNumChannels;
    res << mSampleRate;
    res << mByteRate;
    res << mBlockAlign;
    res << mBitsPerSample;

    if (mFmtSize > FmtChunkMin) {
        res << mExtSize;
    }

    if (mExtSize > 0) {
        res << mValidBitsPerSample;
        res << mChannelMask;
        res.append(mSubFormat);
    }

    if (keepOtherChunks) {
        res.append(mOtherCunks);
    }

    res << WAV_DATA;
    res << quint32(mDataSize);

    // Write file size .........
    quint64 fileSize = mDataSize + res.size() - 8;
    if (fileSize > 0xFFFFFFFF) {
        throw FlaconError("Stream is too big to fit in a legacy WAVE file");
    }

    qint32 le = qToLittleEndian(quint32(fileSize));
    res[4]    = (le >> 0) & 0xFF;
    res[5]    = (le >> 8) & 0xFF;
    res[6]    = (le >> 16) & 0xFF;
    res[7]    = (le >> 24) & 0xFF;

    return res;
}

/************************************************
 * The chunk size fields directly following the chunk-GUID and preceeding
 * the chunk body, include the size of the chunk-GUID and the chunk length
 * field itself.
 * Therefore, it corresponds to the chunk data size plus 24 (16 bytes for
 * the GUID, 8 bytes for the size field).
 ************************************************/
QByteArray WavHeader::wave64ToByteArray() const
{
    QByteArray res;
    res.reserve(mDataStartPos - 1);
    res << WAVE64_GUID_RIFF;
    res << quint64(mFileSize);
    res << WAVE64_GUID_WAVE;

    res << WAVE64_GUID_FMT;
    // The chunk size fields include the size of the chunk-GUID and the chunk length field itself. Therefore, it corresponds to the chunk data size plus 24 (16 bytes for the GUID, 8 bytes for the size field).
    res << quint64(mFmtSize + 24); //
    res << quint16(mFormat);
    res << mNumChannels;
    res << mSampleRate;
    res << mByteRate;
    res << mBlockAlign;
    res << mBitsPerSample;

    if (mFmtSize > FmtChunkMin) {
        res << mExtSize;
    }

    if (mExtSize > 0) {
        res << mValidBitsPerSample;
        res << mChannelMask;
        res.append(mSubFormat);
    }
    res.append(mOtherCunks);

    res << WAVE64_GUID_DATA;
    res << quint64(mDataSize + WAVE64_CHUNK_HEADER_SIZE);

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
            format = "GSM610";
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
