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


#include "inputaudiofile.h"
#include <settings.h>
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>


void initInputAudioFormat(QList<InputAudioFormat> *formats)
{
    *formats << InputAudioFormat("APE",     "ape",  "mac");
    *formats << InputAudioFormat("FLAC",    "flac", "flac");
    *formats << InputAudioFormat("WavPack", "wv",   "wvunpack");
    *formats << InputAudioFormat("TTA",     "tta",  "ttaenc");
    *formats << InputAudioFormat("WAV",     "wav",  "");
}

/************************************************

 ************************************************/
InputAudioFormat::InputAudioFormat(const QString &name, const QString &ext, const QString &program):
    mName(name),
    mExt(ext),
    mProgram(program)
{
}


/************************************************

 ************************************************/
QList<InputAudioFormat> InputAudioFormat::allFormats()
{
    QList<InputAudioFormat> formats;
    if (formats.count() == 0)
        initInputAudioFormat(&formats);

    return formats;
}


/************************************************

 ************************************************/
InputAudioFile::InputAudioFile(const QString &fileName):
    mFileName(fileName),
    mValid(false)
{
    mSampleRate = 0;
    mCdQuality = false;
    mValid = load();
}


/************************************************

 ************************************************/
bool InputAudioFile::load()
{
    QString shntool = settings->value(Settings::Prog_Shntool).toString();
    if (shntool.isEmpty())
    {
        qWarning() << "Program shntool not found.";
        mErrorString = QObject::tr("I can't find program <b>%1</b>.").arg("shntool");
        return false;
    }

    QProcess proc;

    QStringList args;
    args << "info";
    args << mFileName;

    proc.start(shntool, args);

    if (!proc.waitForStarted())
        return false;

    if (!proc.waitForFinished())
        return false;

    if (proc.exitCode() != 0)
    {
        mErrorString = QObject::tr("File <b>%1</b> is not a supported audio file. <br>"
                                   "<br>Verify that all required programs are installed and in your preferences.");
        return false;
    }

    QTextStream stream(&proc);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QString name = QString(line).section(':', 0, 0).toUpper().trimmed();
        QString value =QString(line).section(':', 1).trimmed();

        if (name == "SAMPLES/SEC")
        {
            mSampleRate = value.toInt();
            continue;
        }

        if (name == "CD QUALITY" && value.toUpper() == "YES")
        {
            mCdQuality = true;
            continue;
        }

    }

    return true;
}

