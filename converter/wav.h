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


#ifndef WAV_H
#define WAV_H

#include <QtGlobal>
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


#endif // WAV_H
