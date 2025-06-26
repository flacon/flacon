/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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

#ifndef DECODER_H
#define DECODER_H

#include "wavheader.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include "../types.h"
extern "C" {
#include "libavcodec/codec_id.h"
}

class QIODevice;
class QProcess;
class QFile;
class AVFormatContext;
class AVCodecContext;
struct AVFrame;
struct AVPacket;

namespace Conv {

class Decoder : public QObject
{
    Q_OBJECT
public:
    struct Format
    {
        QString name;
        QString ext;
    };

    static QList<Format> allFormats();
    static QStringList   allFormatsExts();

public:
    explicit Decoder(QObject *parent = nullptr);
    virtual ~Decoder();

    void open(const QString &fileName);
    void close();

    uint64_t extract(const CueTime &startTime, const CueTime &endTime, QIODevice *outDevice);

    WavHeader wavHeader() const { return mWavHeader; }

    QString inputFile() const { return mInputFile; }

    int64_t bytesCount(const CueTime &startTime, const CueTime &endTime) const;

    // Duration of audio in milliseconds.
    mSec duration() const;

    QString   formatName() const;
    AVCodecID formatId() const;

signals:
    void progress(int percent);

private:
    QString   mInputFile;
    WavHeader mWavHeader;

    AVFormatContext *mFormatContext  = nullptr;
    AVCodecContext  *mDecoderContext = nullptr;
    AVPacket        *mPacket         = nullptr;
    AVFrame         *mFrame          = nullptr;
    int              mStreamIndex    = -1;
    int64_t          mDecoderPos     = 0;
    QByteArray       mFrameBuff;

    bool readFrame();

    static uint64_t writeInterleavedFrame(AVFrame *frame, QByteArray *buf);
    static uint64_t writePlanarFrame(AVFrame *frame, QByteArray *buf);

    uint64_t aproximateBytes(const CueTime &endTime) const;
};

} // namespace
#endif // DECODER_H
