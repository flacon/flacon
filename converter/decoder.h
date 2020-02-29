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
#include "../cue.h"
#include "../formats/informat.h"

class QIODevice;
class QProcess;

class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = nullptr);
    virtual ~Decoder();

    void open(const QString &fileName);
    void close();

    void extract(const CueTime &start, const CueTime &end, QIODevice *outDevice);
    void extract(const CueTime &start, const CueTime &end, const QString &outFileName);

    // Duration of audio in milliseconds.
    uint duration() const { return mWavHeader.duration(); }

    WavHeader wavHeader() const { return mWavHeader; }

    const InputFormat *audioFormat() const { return mFormat; }

signals:
    void progress(int percent);

private:
    const InputFormat *mFormat;
    QProcess  *mProcess;
    QString    mInputFile;
    QFile     *mFile;
    WavHeader  mWavHeader;
    quint64    mPos;

    void openFile();
    void openProcess();
};





#endif // DECODER_H
