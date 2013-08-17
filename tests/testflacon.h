/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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


#ifndef TEST_FLACON_H
#define TEST_FLACON_H

#include <QObject>
#include <QStringList>
class Disk;

class ConverterTester
{
public:
    ConverterTester(const QString &cueFile,
                    const QString &audioFile,
                    const QString &expectedCue,
                    const QString &resultFiles);


    void run();

    Disk *disk() { return mDisk; }

private:
    Disk *mDisk;
    QStringList mResultFiles;
    QString mExpectedCue;

    ConverterTester(const ConverterTester &other) {}
};

class TestFlacon : public QObject
{
    Q_OBJECT
public:
    explicit TestFlacon(QObject *parent = 0);

    static bool compareCue(const QString &result, const QString &expected, QString *error);

    
private:
    QStringList readFile(const QString &fileName);
    void writeFile(const QStringList &strings, const QString &fileName);
    QString stigListToString(const QStringList &strings, const QString divider = "");

private slots:
    void initTestCase();

    void testSafeString();

    void testTrackResultFileName_data();
    void testTrackResultFileName();

    void testTrackResultFilePath_data();
    void testTrackResultFilePath();

    void testTrackSetCodepages_data();
    void testTrackSetCodepages();

    void testConvert();


private:
    void createAudioFile(const QString &fileName, int duration, bool cdQuality = true);
    void writeTextFile( const QString &fileName, const QString &content);
    void writeTextFile( const QString &fileName, const QStringList &content);

    bool removeDir(const QString &dirName) const;
    bool clearDir(const QString &dirName) const;
    void checkFileExists(const QString &fileName);
    void checkFileNotExists(const QString &fileName);


    QString mCdAudioFile;
    QString mHdAudioFile;
    const QString mTmpDir;
    const QString mDataDir;
};

QStringList &operator<<(QStringList &list, int value);

#endif // TEST_FLACON_H
