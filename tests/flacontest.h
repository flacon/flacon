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

#ifndef FLACON_TEST_H
#define FLACON_TEST_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QLoggingCategory>
#include <QDir>

class Disc;

#define SettingsValues QMap<QString, QVariant>

class TestFlacon : public QObject
{
    Q_OBJECT
public:
    explicit TestFlacon(QObject *parent = nullptr);

private:
    QStringList readFile(const QString &fileName);
    void        writeFile(const QStringList &strings, const QString &fileName);
    QString     stigListToString(const QStringList &strings, const QString divider = "");
    void        findPrograms();

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void testByteArraySplit_data();
    void testByteArraySplit();

    void testSafeString_data();
    void testSafeString();

    void testTrackResultFileName_data();
    void testTrackResultFileName();

    void testTrackResultFilePath_data();
    void testTrackResultFilePath();

    void testTrackSetCodepages_data();
    void testTrackSetCodepages();

    void testCueTime_data();
    void testCueTime();

    // .................
    void testCalcDiskState();

    // .................
    void testCue();
    void testCue_data();

    void testCueData();
    void testCueData_data();

    void testSearchCoverImage();
    void testSearchCoverImage_data();

    void testReadWavHeader();
    void testReadWavHeader_data();

    void testResizeWavHeader();
    void testResizeWavHeader_data();

    void testToLegacyWav();
    void testToLegacyWav_data();

    void testInputAudioFile();
    void testInputAudioFile_data();

    void testDecoder();
    void testDecoder_data();

    void testPatternExpander();
    void testPatternExpander_data();

    void testPregapTrackResultFileName();
    void testPregapTrackResultFileName_data();

    void testPatternExpanderLastDir();
    void testPatternExpanderLastDir_data();

    void testLoadProfiles();
    void testLoadProfiles_data();

    void testAudioFileMatcher();
    void testAudioFileMatcher_data();

    void testLoadDiscFromAudio();
    void testLoadDiscFromAudio_data();

    void testLoadDiscFromAudioErrors();
    void testLoadDiscFromAudioErrors_data();

    void testReplayGain();
    void testReplayGain_data();

    void testValidator();
    void testValidator_data();

    void testTextCodecNames();

    void testTextCodecs();
    void testTextCodecs_data();

    void testConvert();
    void testConvert_data();

    void testMetaDataWriter();
    void testMetaDataWriter_data();

    void testConvertBroken();
    void testConvertBroken_data();

private:
    void writeTextFile(const QString &fileName, const QString &content);
    void writeTextFile(const QString &fileName, const QStringList &content);

    bool removeDir(const QString &dirName) const;
    bool clearDir(const QString &dirName) const;
    void removeLargeFiles(const QString &dirPath, qint64 sizeLimit = 1024 * 1024);
    void checkFileExists(const QString &fileName);
    void checkFileNotExists(const QString &fileName);

    void    applySettings(const SettingsValues &config);
    QString dir(const QString &subTest = "");
    QString sourceDir(const QString &subTest = "");
    void    copyTestDir(const QString &srcDir, const QString &destDir);
    void    copyFile(const QString &srcFile, const QString &destFile);

    Disc *standardDisc();

    QString mFfmpeg;
    QString mAudio_cd_wav;
    QString mAudio_24x96_wav;

    QString mAudio_cd_ape;
    QString mAudio_24x96_ape;

    QString mAudio_cd_flac;
    QString mAudio_24x96_flac;

    QString mAudio_cd_wv;
    QString mAudio_24x96_wv;

    QString mAudio_cd_tta;
    QString mAudio_24x96_tta;

    const QString mTmpDir;
    const QString mDataDir;
    Disc         *mStandardDisc;

    static int mTestNum;
};

QStringList &operator<<(QStringList &list, int value);

#endif // FLACON_TEST_H
