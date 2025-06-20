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

#ifndef INPUTAUDIOFILE_H
#define INPUTAUDIOFILE_H

#include "types.h"
#include <QString>
#include <QList>
#include <QExplicitlySharedDataPointer>
#include "converter/decoder.h"

class InputAudioFile
{
private:
    class Data : public QSharedData
    {
    public:
        Data()                  = default;
        Data(const Data &other) = default;

    public:
        QString   mFilePath;
        QString   mFileName;
        QString   mErrorString;
        QString   mFormatName;
        AVCodecID mFormatId      = AV_CODEC_ID_NONE;
        quint32   mSampleRate    = 0;
        int       mBitsPerSample = 0;
        mSec      mDuration      = 0;
        bool      mValid         = false;
        bool      mCdQuality     = false;
        uint      mChannelsCount = 0;

        void load(const QString &fileName);
    };

    QExplicitlySharedDataPointer<Data> mData;

public:
    InputAudioFile();
    explicit InputAudioFile(const QString &fileName);
    InputAudioFile(const InputAudioFile &other);
    InputAudioFile &operator=(const InputAudioFile &other);

    bool operator==(const InputAudioFile &other) const;

    QString filePath() const { return mData->mFilePath; }
    QString fileName() const { return mData->mFileName; }
    QString formatName() const { return mData->mFormatName; }
    QString errorString() const { return mData->mErrorString; }

    bool isNull() const { return mData->mFileName.isEmpty(); }
    bool isValid() const { return mData->mValid; }
    bool isCdQuality() const { return mData->mCdQuality; }
    int  sampleRate() const { return mData->mSampleRate; }
    int  bitsPerSample() const { return mData->mBitsPerSample; }
    mSec duration() const { return mData->mDuration; }
    uint channelsCount() const { return mData->mChannelsCount; }

    QByteArray readEmbeddedCue() const;
};

using InputAudioFileList = QList<InputAudioFile>;

#endif // INPUTAUDIOFILE_H
