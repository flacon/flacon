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
#include <QFileInfo>
#include <QDir>

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
    mValid(false),
    mSampleRate(0),
    mCdQuality(false),
    mDuration(0)

{
    mValid = load();
}


/************************************************

 ************************************************/
InputAudioFile::InputAudioFile(const InputAudioFile &other)
{
    mFileName    = other.mFileName;
    mValid       = other.mValid;
    mErrorString = other.mErrorString;
    mSampleRate  = other.mSampleRate;
    mCdQuality   = other.mCdQuality;
    mDuration    = other.mDuration;
}


/************************************************

 ************************************************/
bool InputAudioFile::load()
{
    if (mFileName == "")
    {
        qWarning() << "Audio file name is'n set";
        mErrorString = QObject::tr("Audio file name is'n set");
        return false;
    }

    if (!QFileInfo(mFileName).exists())
    {
        qWarning() << QString("Audio file <b>\"%1\"</b> not exists").arg(mFileName);
        mErrorString = QObject::tr("Audio file <b>\"%1\"</b> not exists").arg(mFileName);
        return false;
    }

    QString shntool = QDir::toNativeSeparators(settings->value(Settings::Prog_Shntool).toString());
    if (shntool.isEmpty())
    {
        qWarning() << "Program shntool not found.";
        mErrorString = QObject::tr("I can't find program <b>%1</b>.").arg("shntool");
        return false;
    }

    QProcess proc;

    QStringList args;
    args << "info";
    args << QDir::toNativeSeparators(mFileName);

    proc.start(shntool, args);

    if (!proc.waitForFinished())
    {
        qWarning("------------------------------------");
        qWarning() << "Test audio command:" << (shntool + " " + args.join(" "));
        qWarning() << "shntool info waitForFinished faild";
        qWarning(proc.readAllStandardError());
        qWarning("------------------------------------");
        return false;
    }


    if (proc.exitCode() != 0)
    {
        qWarning("------------------------------------");
        qWarning() << "Test audio command:" << (shntool + " " + args.join(" "));
        qWarning() << "shntool info nonzero exit code:" << proc.exitCode();
        qWarning(proc.readAllStandardError());
        qWarning("------------------------------------");
        mErrorString = QObject::tr("File <b>%1</b> is not a supported audio file. <br>"
                                   "<br>Verify that all required programs are installed and in your preferences.").arg(mFileName);
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


        if (name == "LENGTH")
        {
            // 0h 0m 3s - Length:   0:03.00
            // 1h 2m 5s - Length:  62:05.00
            QRegExp re("(\\d+):(\\d+)\\.(\\d+)");
            if (re.exactMatch(value))
            {
                mDuration = re.cap(1).toInt() * 60 * 1000 +
                            re.cap(2).toInt() * 1000 +
                            re.cap(3).toInt();
            }
            continue;
        }
    }

    return true;
}

