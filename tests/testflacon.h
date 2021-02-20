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

#ifndef TEST_FLACON_H
#define TEST_FLACON_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
class Disc;

#define SettingsValues QMap<QString, QVariant>

class TestFlacon : public QObject
{
    Q_OBJECT
public:
    explicit TestFlacon(QObject *parent = nullptr);

    static bool compareCue(const QString &result, const QString &expected, QString *error, bool skipEmptyLines = false);

private:
    QStringList readFile(const QString &fileName);
    void        writeFile(const QStringList &strings, const QString &fileName);
    QString     stigListToString(const QStringList &strings, const QString divider = "");

private slots:
    void initTestCase();
    void init();

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

    void testFormatWavLast();

    void testFormat();
    void testFormat_data();

    void testFormatFromFile();
    void testFormatFromFile_data();

    void testInputAudioFile();
    void testInputAudioFile_data();

    void testDecoder();
    void testDecoder_data();

    void testByteArraySplit_data();
    void testByteArraySplit();

    void testSafeString_data();
    void testSafeString();

    void testTrackResultFileName_data();
    void testTrackResultFileName();

    void testTrackResultFilePath_data();
    void testTrackResultFilePath();

    void testOutFormatEncoderArgs_data();
    void testOutFormatEncoderArgs();

    void testOutFormatGainArgs_data();
    void testOutFormatGainArgs();

    void testTrackSetCodepages_data();
    void testTrackSetCodepages();

    void testCueIndex_data();
    void testCueIndex();

    void testDiscPipelineCalcQuality_data();
    void testDiscPipelineCalcQuality();

    void testFindCueFile_data();
    void testFindCueFile();

    void testPatternExpander_data();
    void testPatternExpander();

    void testLoadProfiles();
    void testLoadProfiles_data();

    void testAudioFileMatcher();
    void testAudioFileMatcher_data();

    void testLoadDiscFromAudio();
    void testLoadDiscFromAudio_data();

    void testConvert();
    void testConvert_data();

private:
    void writeTextFile(const QString &fileName, const QString &content);
    void writeTextFile(const QString &fileName, const QStringList &content);

    bool removeDir(const QString &dirName) const;
    bool clearDir(const QString &dirName) const;
    void checkFileExists(const QString &fileName);
    void checkFileNotExists(const QString &fileName);

    void    applySettings(const SettingsValues &config);
    QString dir(const QString &subTest = "");
    QString sourceDir(const QString &subTest = "");

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
    Disc *        mStandardDisc;

    static int mTestNum;
};

QStringList &operator<<(QStringList &list, int value);

#endif // TEST_FLACON_H
