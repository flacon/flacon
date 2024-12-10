/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019-2023
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

#include "profiles.h"
#include "formats_out/outformat.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QThread>
#include "patternexpander.h"
#include "disc.h"

/************************************************
 *
 ************************************************/
QString Profile::defaultOutFileDir()
{
    QString outDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (outDir.isEmpty())
        outDir = "~/Music";

    outDir.replace(QDir::homePath(), "~");

    return outDir;
}

/************************************************
 *
 ************************************************/
Profile::Profile(const QString &formatId, const QString &id)
{
    mFormat = OutFormat::formatForId(formatId);
    if (!mFormat) {
        return;
    }

    mId   = !id.isEmpty() ? id : mFormat->id();
    mName = mFormat->name();

    mEncoderValues = mFormat->defaultParameters();
}

/************************************************
 *
 ************************************************/
bool Profile::isValid() const noexcept
{
    return !mId.isEmpty() && mFormat != nullptr;
}

/************************************************
 *
 ************************************************/
void Profile::setName(const QString &value)
{
    mName = value;
}

/************************************************
 *
 ************************************************/
void Profile::setOutFileDir(const QString &value)
{
    mOutFileDir = value;
}

/************************************************
 *
 ************************************************/
void Profile::setOutFilePattern(const QString &value)
{
    mOutFilePattern = value;
}

/************************************************
 *
 ************************************************/
void Profile::setGainType(GainType value)
{
    if (mFormat->options().testFlag(FormatOption::SupportGain)) {
        mGainType = value;
    }
}

/************************************************
 *
 ************************************************/
void Profile::setBitsPerSample(BitsPerSample value)
{
    mBitsPerSample = value;
}

/************************************************
 *
 ************************************************/
void Profile::setSampleRate(SampleRate value)
{
    mSampleRate = value;
}

/************************************************
 *
 ************************************************/
void Profile::setCreateCue(bool value)
{
    mCreateCue = value;
}

/************************************************
 *
 ************************************************/
void Profile::setEmbedCue(bool value)
{
    if (mFormat->options().testFlag(FormatOption::SupportEmbeddedCue)) {
        mEmbedCue = value;
    }
}

/************************************************
 *
 ************************************************/
void Profile::setWriteSingleDiskNum(bool value)
{
    mWriteSingleDiskNum = value;
}

/************************************************
 *
 ************************************************/
void Profile::setCueFileName(const QString &value)
{
    mCueFileName = value;
}

/************************************************
 *
 ************************************************/
void Profile::setPregapType(PreGapType value)
{
    mPregapType = value;
}

/************************************************
 *
 ************************************************/
void Profile::setCopyCoverOptions(const CoverOptions &value)
{
    mCopyCoverOptions = value;
}

/************************************************
 *
 ************************************************/
void Profile::setEmbedCoverOptions(const CoverOptions &value)
{
    if (mFormat->options().testFlag(FormatOption::SupportEmbeddedImage)) {
        mEmbedCoverOptions = value;
    }
}

/************************************************
 *
 ************************************************/
void Profile::setEncoderValues(const EncoderValues &values)
{
    mEncoderValues = values;
}

/************************************************
 *
 ************************************************/
QVariant Profile::encoderValue(const QString &key, const QVariant &defaultValue) const
{
    return mEncoderValues.value(key, defaultValue);
}

/************************************************
 *
 ************************************************/
void Profile::setEncoderValue(const QString &key, const QVariant &value)
{
    mEncoderValues[key] = value;
}

/************************************************
 *
 ************************************************/
void Profile::setTmpDir(const QString &value)
{
    globalParams().mTmpDir = value;
}

/************************************************
 *
 ************************************************/
uint Profile::defaultEncoderThreadCount()
{
    return std::max(6, QThread::idealThreadCount());
}

/************************************************
 *
 ************************************************/
void Profile::setEncoderThreadsCount(uint value)
{
    if (value > 0) {
        globalParams().mEncoderThreadsCount = value;
        return;
    }

    globalParams().mEncoderThreadsCount = defaultEncoderThreadCount();
}

/************************************************
 *
 ************************************************/
void Profile::setSplitTrackTitle(bool value)
{
    globalParams().splitTrackTitle = value;
}

/************************************************
 *
 ************************************************/
QString Profile::resultFileName(const Track *track) const
{
    return PatternExpander::resultFileName(outFilePattern(), track, ext());
}

/************************************************
 *
 ************************************************/
QString Profile::resultFileDir(const Track *track) const
{
    return QFileInfo(resultFilePath(track)).absoluteDir().path();
}

/************************************************
 *
 ************************************************/
QString Profile::resultFilePath(const Track *track) const
{
    QString fileName = resultFileName(track);
    if (fileName.isEmpty()) {
        return "";
    }

    QString dir = calcResultFilePath(track);
    if (dir.endsWith("/") || fileName.startsWith("/")) {
        return dir + fileName;
    }
    else {
        return dir + "/" + fileName;
    }
}

/************************************************
 *
 ************************************************/
QString Profile::calcResultFilePath(const Track *track) const
{
    QString dir = outFileDir();

    if (dir == "~" || dir == "~//") {
        return QDir::homePath();
    }

    if (dir == ".") {
        dir = "";
    }

    if (dir.startsWith("~/")) {
        return dir.replace(0, 1, QDir::homePath());
    }

    QFileInfo fi(dir);

    if (fi.isAbsolute()) {
        return fi.absoluteFilePath();
    }

    if (!track->disc()) {
        return "";
    }

    QString cueFile = track->disc()->cue().filePath();
    if (cueFile.startsWith(Cue::EMBEDED_PREFIX)) {
        cueFile = cueFile.mid(strlen(Cue::EMBEDED_PREFIX));
    }

    return QFileInfo(cueFile).dir().absolutePath() + QDir::separator() + dir;
}

/************************************************
 *
 ************************************************/
Profile::GlobalParams &Profile::globalParams()
{
    static Profile::GlobalParams res;
    return res;
}

/************************************************
 *
 ************************************************/
int Profiles::indexOf(const QString &id, int from) const
{
    for (int i = from; i < count(); ++i) {
        if (at(i).id() == id) {
            return i;
        }
    }
    return -1;
}

/************************************************
 *
 ************************************************/
Profile *Profiles::find(const QString &id)
{
    for (Profile &p : *this) {
        if (p.id() == id) {
            return &p;
        }
    }

    return nullptr;
}

/************************************************
 *
 ************************************************/
const Profile *Profiles::find(const QString &id) const
{
    for (const Profile &p : *this) {
        if (p.id() == id) {
            return &p;
        }
    }

    return nullptr;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug debug, const Profile &profile)
{
    QDebug &dbg = debug.noquote().noquote();
    dbg << "ID:     " << profile.id() << "\n";
    dbg << "Format: " << profile.formatId() << "\n";
    dbg << "Name:   " << profile.name() << "\n";
    dbg << "Valid:  " << profile.isValid() << "\n";

    //    QMap<QString, QVariant> values = profile.save();
    //    for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
    //        dbg << "  " << i.key() << " = " << i.value() << "\n";
    //    }

    return debug;
}

/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Profiles &profiles)
{
    dbg.nospace().noquote() << profiles.count() << " items .....................\n";
    for (const Profile &p : profiles) {
        dbg << p;
        dbg << "\n";
    }
    dbg.nospace().noquote() << "....................................";
    return dbg.space();
}

/************************************************
 *
 ************************************************/
Profiles createStandardProfiles()
{
    Profiles res;
    foreach (const OutFormat *format, OutFormat::allFormats()) {
        res << Profile(format->id());
    }
    return res;
}
