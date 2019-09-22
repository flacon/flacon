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


#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <QStringList>
#include <QVector>
#include "wavheader.h"

class QIODevice;
class Disk;


class TestCueFile
{
public:
    explicit TestCueFile(const QString &fileName);

    QString fileName() const { return mFileName; }

    void setWavFile(const QString &value);
    QString wavFile() const { return mWavFile; }

    void addTrack(const QString &index0, const QString &index1);
    void addTrack(const QString &index1);

    void write();

private:
    struct TestCueTrack {
        QString index0;
        QString index1;

        TestCueTrack():
            index0(""),
            index1("")
        {
        }


        explicit TestCueTrack(const QString &index1):
            index0(""),
            index1(index1)
        {
        }


        TestCueTrack(const QString &index0, const QString &index1):
            index0(index0),
            index1(index1)
        {
        }


    };

    QString mFileName;
    QString mWavFile;
    QVector<TestCueTrack> mTracks;
};


QString calcAudioHash(const QString &fileName);
bool  compareAudioHash(const QString &file1, const QString &expected);
void writeHexString(const QString &str, QIODevice *out);
void createWavFile(const QString &fileName, const QString &header);
void createWavFile(const QString &fileName, int duration, StdWavHeader::Quality quality);
void encodeAudioFile(const QString &wavFileName, const QString &outFileName);
void testFail(const QString &message, const char *file, int line);

#define FAIL(message) \
do {\
    testFail(message, __FILE__, __LINE__); \
} while (0)

Disk *loadFromCue(const QString &cueFile);

#endif // TOOLS_H
