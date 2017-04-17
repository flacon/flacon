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

#include "testflacon.h"
#include "tools.h"
#include "converter/wavheader.h"
#include "../settings.h"

#include <QString>
#include <QProcessEnvironment>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


#ifdef Q_OS_WIN
    #define PATH_ENV_SEPARATOR ';'
    #define BINARY_EXT ".exe"

#elif defined(Q_OS_OS2)
    #define PATH_ENV_SEPARATOR ';'
    #define BINARY_EXT ".exe"

#else
    #define PATH_ENV_SEPARATOR ':'
    #define BINARY_EXT ""

#endif


/************************************************
 *
 ************************************************/
QString findProgram(const QString &program)
{
    QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(PATH_ENV_SEPARATOR);
    foreach(QString path, paths)
    {
        QFileInfo fi(path + QDir::separator() + program + BINARY_EXT);
        if (fi.exists() && fi.isExecutable())
            return fi.absoluteFilePath();
    }
    return "";
}



/************************************************
 *
 ************************************************/
void TestFlacon::initTestCase()
{
    Settings::setFileName(TEST_OUT_DIR "/flacon.conf");

//    QString shntool = settings->findProgram("shntool");
//    if (shntool.isEmpty())
//        FAIL(QString("Program \"%1\" not found.").arg("shntool").toLocal8Bit());

//    settings->setValue(Settings::Prog_Shntool, shntool);
//    settings->sync();

//    mFfmpeg = settings->findProgram("avconv");
//    if (mFfmpeg.isEmpty())
//        mFfmpeg = settings->findProgram("ffmpeg");

//    if (mFfmpeg.isEmpty())
//        FAIL(QString("Program \"%1\" not found.").arg("avconv/ffmpeg").toLocal8Bit());

    if (findProgram("mac").isEmpty())      QFAIL("mac program not found");
    if (findProgram("flac").isEmpty())     QFAIL("flac program not found");
    if (findProgram("wavpack").isEmpty())  QFAIL("wavpack program not found");
    if (findProgram("ttaenc").isEmpty())   QFAIL("ttaenc program not found");


    mAudio_cd_wav  = mTmpDir + "CD.wav";
    mAudio_cd_ape  = mTmpDir + "CD.ape";
    mAudio_cd_flac = mTmpDir + "CD.flac";
    mAudio_cd_wv   = mTmpDir + "CD.wv";
    mAudio_cd_tta  = mTmpDir + "CD.tta";

    createWavFile(  mAudio_cd_wav,  900, StdWavHeader::Quality_Stereo_CD);
    encodeAudioFile(mAudio_cd_wav, mAudio_cd_ape);
    encodeAudioFile(mAudio_cd_wav, mAudio_cd_flac);
    encodeAudioFile(mAudio_cd_wav, mAudio_cd_wv);
    encodeAudioFile(mAudio_cd_wav, mAudio_cd_tta);


    mAudio_24x96_wav  = mTmpDir + "24x96.wav";
    mAudio_24x96_ape  = mTmpDir + "24x96.ape";
    mAudio_24x96_flac = mTmpDir + "24x96.flac";
    mAudio_24x96_wv   = mTmpDir + "24x96.wv";
    mAudio_24x96_tta  = mTmpDir + "24x96.tta";

    createWavFile(  mAudio_24x96_wav,  900, StdWavHeader::Quality_Stereo_24_96);
    encodeAudioFile(mAudio_24x96_wav, mAudio_24x96_ape);
    encodeAudioFile(mAudio_24x96_wav, mAudio_24x96_flac);
    encodeAudioFile(mAudio_24x96_wav, mAudio_24x96_wv);
    encodeAudioFile(mAudio_24x96_wav, mAudio_24x96_tta);

}
