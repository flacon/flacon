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

#ifndef WAVHEADER_H
#define WAVHEADER_H

#include <QtGlobal>
#include <QIODevice>
#include <QTime>

namespace Conv {

/************************************************
 * Info for format tags can be found at:
 *   http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
 ************************************************/
class WavHeader
{
public:
    enum Format {
        Format_Unknown           = 0x0000,
        Format_PCM               = 0x0001,
        Format_ADPCM             = 0x0002,
        Format_IEEE_FLOAT        = 0x0003,
        Format_ALAW              = 0x0006,
        Format_MULAW             = 0x0007,
        Format_OKI_ADPCM         = 0x0010,
        Format_IMA_ADPCM         = 0x0011,
        Format_DIGISTD           = 0x0015,
        Format_DIGIFIX           = 0x0016,
        Format_DOLBY_AC2         = 0x0030,
        Format_GSM610            = 0x0031,
        Format_ROCKWELL_ADPCM    = 0x003b,
        Format_ROCKWELL_DIGITALK = 0x003c,
        Format_G721_ADPCM        = 0x0040,
        Format_G728_CELP         = 0x0041,
        Format_MPEG              = 0x0050,
        Format_MPEGLAYER3        = 0x0055,
        Format_G726_ADPCM        = 0x0064,
        Format_G722_ADPCM        = 0x0065,
        Format_Extensible        = 0xFFFE,
    };

    enum Quality {
        Quality_Stereo_CD     = 2 * 16 * 44100,
        Quality_Stereo_24_96  = 2 * 24 * 96000,
        Quality_Stereo_24_192 = 2 * 24 * 192000

    };

    typedef char SubFormat[16];

    WavHeader() = default;
    explicit WavHeader(QIODevice *stream) noexcept(false);

    WavHeader(const WavHeader &other) = default;
    WavHeader &operator=(const WavHeader &other) = default;

    /// Duration of audio in milliseconds.
    quint64 duration() const;

    QByteArray toByteArray() const;

    void resizeData(quint32 dataSize);

    static quint32 bytesPerSecond(Quality quality);
    quint32        bytesPerSecond();

    quint64 fileSize() const { return mFileSize; }
    Format  format() const { return mFormat; }
    quint16 numChannels() const { return mNumChannels; }
    quint32 sampleRate() const { return mSampleRate; }
    quint32 byteRate() const { return mByteRate; }
    quint16 blockAlign() const { return mBlockAlign; }
    quint16 bitsPerSample() const { return mBitsPerSample; }
    quint16 validBitsPerSample() const { return mValidBitsPerSample; }
    quint32 channelMask() const { return mChannelMask; }
    quint64 dataSize() const { return mDataSize; }
    quint64 dataStartPos() const { return mDataStartPos; }
    bool    isCdQuality() const;
    bool    is64Bit() const { return m64Bit; }

protected:
    enum FmtChunkSize {
        FmtChunkMin = 16,
        FmtChunkMid = 18,
        FmtChunkExt = 40,
    };

    bool         m64Bit              = false;
    quint64      mFileSize           = 0;
    FmtChunkSize mFmtSize            = FmtChunkMin;
    Format       mFormat             = WavHeader::Format_Unknown;
    quint16      mNumChannels        = 0;
    quint32      mSampleRate         = 0;
    quint32      mByteRate           = 0;
    quint16      mBlockAlign         = 0;
    quint16      mBitsPerSample      = 0;
    quint16      mExtSize            = 0; // Size of the extension:
    quint16      mValidBitsPerSample = 0; // at most 8*M
    quint32      mChannelMask        = 0; // Speaker position mask
    QByteArray   mSubFormat          = 0; // GUID (first two bytes are the data format code)
    quint64      mDataSize           = 0;
    quint64      mDataStartPos       = 0;

    QByteArray mOtherCunks;

private:
    void loadFmtChunk(QIODevice *stream, const quint32 chunkSize);

    void readWavHeader(QIODevice *stream);
    void readWave64Header(QIODevice *stream);

    QByteArray wavToByteArray() const;
    QByteArray wave64ToByteArray() const;
};

} // namespace

QDebug operator<<(QDebug dbg, const Conv::WavHeader &header);

#endif // WAVHEADER_H
