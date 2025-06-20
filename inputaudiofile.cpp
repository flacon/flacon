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
#include "decoder.h"
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>

namespace {
Q_LOGGING_CATEGORY(LOG, "InputAudioFile")
}

void InputAudioFile::Data::load(const QString &filePath)
{
    mFilePath = filePath;
    qCDebug(LOG) << "load" << filePath;

    if (mFilePath.isEmpty()) {
        mErrorString = QObject::tr("The audio file name is not set.");
        qCDebug(LOG) << mErrorString;
        mValid = false;
        mFileName.clear();
        return;
    }

    QFileInfo fi(filePath);
    mFileName = fi.fileName();

    if (!fi.exists()) {
        mErrorString = QObject::tr("The audio file does not exist.");
        qCDebug(LOG) << mErrorString;
        mValid = false;
        return;
    }

    try {
        Conv::Decoder dec;
        dec.open(mFilePath);

        mSampleRate    = dec.wavHeader().sampleRate();
        mBitsPerSample = dec.wavHeader().bitsPerSample();
        mCdQuality     = dec.wavHeader().isCdQuality();
        mDuration      = dec.duration();
        mChannelsCount = dec.wavHeader().numChannels();
        mFormatName    = dec.formatName();
        mFormatId      = dec.formatId();

        mValid = true;

        // clang-format off
        qCDebug(LOG) << "Audio is loaded: "
                        "format ="         << mFormatName <<
                        "mDuration ="      << mDuration   <<
                        "mCdQuality ="     << mCdQuality  <<
                        "mSampleRate ="    << mSampleRate <<
                        "mBitsPerSample =" << mBitsPerSample;
        // clang-format on
    }
    catch (FlaconError &err) {
        mErrorString = err.what();
        qCDebug(LOG) << mErrorString;
        mValid = false;
    }
}

bool InputAudioFile::operator==(const InputAudioFile &other) const
{
    return mData->mFilePath == other.mData->mFilePath;
}

InputAudioFile::InputAudioFile() :
    mData(new Data())
{
}

InputAudioFile::InputAudioFile(const QString &fileName) :
    mData(new Data())
{
    mData->load(fileName);
}

InputAudioFile::InputAudioFile(const InputAudioFile &other) :
    mData(other.mData)
{
}

InputAudioFile &InputAudioFile::operator=(const InputAudioFile &other)
{
    mData = other.mData;
    return *this;
}

QByteArray InputAudioFile::readEmbeddedCue() const
{
    if (!isValid()) {
        return {};
    }

    if (mData->mFormatId == AV_CODEC_ID_FLAC) {
        TagLib::FLAC::File file(mData->mFilePath.toLocal8Bit().data());
        if (!file.isOpen()) {
            return QByteArray();
        }

        TagLib::Ogg::XiphComment *comment = file.xiphComment(false);
        if (!comment) {
            return QByteArray();
        }

        const TagLib::Ogg::FieldListMap &tags = comment->fieldListMap();

        static constexpr auto CUE_SHEET_TAGS = { "CUESHEET", "cuesheet" };

        for (auto key : CUE_SHEET_TAGS) {
            if (tags.contains(key)) {
                return QByteArray(tags[key].front().toCString(true));
            }
        }

        return QByteArray();
    }

    return {};
}
