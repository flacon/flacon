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
#include "settings.h"
#include <QJsonDocument>

class QIODevice;
class Disc;

class TestCueFile
{
public:
    explicit TestCueFile(const QString &fileName);

    QString fileName() const { return mFileName; }

    void    setWavFile(const QString &value);
    QString wavFile() const { return mWavFile; }

    void addTrack(const QString &index0, const QString &index1);
    void addTrack(const QString &index1);

    void write();

private:
    struct TestCueTrack
    {
        QString index0;
        QString index1;

        TestCueTrack() :
            index0(""),
            index1("")
        {
        }

        explicit TestCueTrack(const QString &index1) :
            index0(""),
            index1(index1)
        {
        }

        TestCueTrack(const QString &index0, const QString &index1) :
            index0(index0),
            index1(index1)
        {
        }
    };

    QString               mFileName;
    QString               mWavFile;
    QVector<TestCueTrack> mTracks;
};

class TestSettings : public Settings
{
    Q_OBJECT
public:
    TestSettings(const QString &fileName) :
        Settings(fileName) { }
    void apply(const QMap<QString, QVariant> &values);
};

QString    calcAudioHash(const QString &fileName);
bool       compareAudioHash(const QString &file1, const QString &expected);
void       writeHexString(const QString &str, QIODevice *out);
QByteArray writeHexString(const QString &str);
void       createWavFile(const QString &fileName, const QString &header, const int duration = 0);
void       createWavFile(const QString &fileName, quint16 bitsPerSample, quint32 sampleRate, uint durationSec);
void       encodeAudioFile(const QString &wavFileName, const QString &outFileName);
void       testFail(const QString &message, const char *file, int line);

#define FAIL(message)                          \
    do {                                       \
        testFail(message, __FILE__, __LINE__); \
    } while (0)

Disc *loadFromCue(const QString &cueFile);

class ConverterTest
{
public:
    ConverterTest(const QString &dataDir, const QString &dir, const QString &tmpDir);
    virtual ~ConverterTest();

    bool run();
    void check();

    QString dataDir() const { return mDataDir; }
    QString dir() const { return mDir; }

private:
    const QString mDataDir;
    const QString mDir;
    const QString mTmpDir;

    const QString mCfgFile;
    const QString mInDir;
    const QString mOutDir;

    QSettings mSpec;

    QStringList readFile(const QString &fileName);
    void        writeFile(const QStringList &strings, const QString &fileName);

    void        createStartSh(const QString fileName, const QString flaconBin, const QStringList &args) const;
    QStringList findFiles(const QString &dir, const QString &pattern) const;
};

class Mediainfo
{
public:
    Mediainfo(const QString &fileName);

    void save(const QString &fileName);

    QVariant value(const QString &key);

private:
    QString       mFileName;
    QJsonDocument mJsonDoc;
    QByteArray    mData;
};

#endif // TOOLS_H
