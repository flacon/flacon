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

#include "encoder.h"
#include "profiles.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QLoggingCategory>
#include "extprocess.h"
#include "formats_out/metadatawriter.h"

namespace {
Q_LOGGING_CATEGORY(LOG, "Encoder")
}

using namespace Conv;

const quint64 MIN_BUF_SIZE = 4 * 1024;
const quint64 MAX_BUF_SIZE = 1024 * 1024;

/************************************************
 *
 ************************************************/
Encoder::Encoder(QObject *parent) :
    Worker(parent)
{
}

/************************************************
 *
 ************************************************/
QProcess *Encoder::createEncoderProcess()
{
    ExtProgram *prog = mProfile.outFormat()->encoderProgram(mProfile);
    if (!prog) {
        return nullptr;
    }

    QStringList args = mProfile.outFormat()->encoderArgs(mProfile, mOutFile);

    qCDebug(LOG) << "Start encoder:" << debugProgramArgs(prog->path(), args);

    QProcess *res = new ExtProcess();
    res->setObjectName("encoder");
    res->setProgram(prog->path());
    res->setArguments(args);

#ifdef MAC_BUNDLE
    res->setEnvironment(QStringList("LANG=en_US.UTF-8"));
#endif

    return res;
}

/************************************************
 *
 ************************************************/
QProcess *Encoder::createRasmpler(const QString &outFile)
{
    const InputAudioFile &audio = mTrack.audioFile();

    int bps  = calcQuality(audio.bitsPerSample(), mProfile.bitsPerSample(), mProfile.outFormat()->maxBitPerSample());
    int rate = calcQuality(audio.sampleRate(), mProfile.sampleRate(), mProfile.outFormat()->maxSampleRate());

    qCDebug(LOG) << "Input audio: bitsPerSample =" << audio.bitsPerSample() << " sampleRate =" << audio.sampleRate();
    qCDebug(LOG) << "Required:    bitsPerSample =" << bps << " sampleRate =" << rate;

    if (bps == audio.bitsPerSample() && rate == audio.sampleRate()) {
        qCDebug(LOG) << "Resampling is not required";
        return nullptr;
    }

    ExtProgram *prog = ExtProgram::sox();
    QStringList args = resamplerArgs(bps, rate, outFile);

    qCDebug(LOG) << "Start resampler:" << debugProgramArgs(prog->path(), args);

    QProcess *res = new ExtProcess();
    res->setObjectName("resampler");
    res->setProgram(prog->path());
    res->setArguments(args);
    return res;
}

/************************************************

************************************************/
QProcess *Encoder::createDemph(const QString &outFile)
{
    if (!mTrack.preEmphased()) {
        qCDebug(LOG) << "DeEmphasis is not required";
        return nullptr;
    }

    // sample rate must be 44100 (audio-CD) or 48000 (DAT)
    int rate = mTrack.audioFile().sampleRate();
    if (rate != 44100 && rate != 48000) {
        qCDebug(LOG) << "DeEmphasis disabled, sample rate must be 44100 (audio-CD) or 48000 (DAT)";
        return nullptr;
    }

    ExtProgram *prog = ExtProgram::sox();
    QStringList args = deemphasisArgs(outFile);

    qCDebug(LOG) << "Start deEmphasis:" << debugProgramArgs(prog->path(), args);

    QProcess *res = new ExtProcess();
    res->setObjectName("deemphasis");
    res->setProgram(prog->path());
    res->setArguments(args);
    return res;
}

/************************************************

 ************************************************/
void Encoder::run()
{
    mReplayGainEnabled = mProfile.gainType() != GainType::Disable;

    emit trackProgress(track(), TrackState::Encoding, 0);

    QList<QProcess *> procs;

    QProcess *encoder = createEncoderProcess();
    if (encoder) {
        procs.insert(0, encoder);
    }

    QProcess *resampler = createRasmpler(procs.isEmpty() ? mOutFile : "-");
    if (resampler) {

        procs.insert(0, resampler);
    }

    QProcess *demph = createDemph(procs.isEmpty() ? mOutFile : "-");
    if (demph) {
        procs.insert(0, demph);
    }

    if (procs.isEmpty()) {
        //------------------------------------------------
        // The output file format is WAV and no preprocessing is required,
        // so just rename/copy the file.
        qCDebug(LOG) << "Copy file: in = " << inputFile() << "out = " << outFile();
        copyFile();
        emit trackProgress(track(), TrackState::Encoding, 100);
        emit trackReady(track(), outFile(), ReplayGain::Result());
        return;
    }

    QObject keeper(this);
    //------------------------------------------------
    try {
        for (int i = 0; i < procs.count(); ++i) {
            QProcess *proc = procs[i];
            proc->setParent(&keeper);
        }

        // We start all processes connected by a pipe
        for (int i = 0; i < procs.count() - 1; ++i) {
            QProcess *proc = procs[i];
            proc->setStandardOutputProcess(procs[i + 1]);
        }

        connect(procs.first(), &QProcess::bytesWritten, this, &Encoder::processBytesWritten);

        for (QProcess *proc : procs) {
            proc->start();
            proc->waitForStarted();
        }

        readInputFile(procs.first());

        for (QProcess *p : procs) {
            p->closeWriteChannel();
            p->waitForFinished(-1);
        }

        for (QProcess *p : procs) {
            if (p->exitCode() != 0) {
                throw(QString::fromLocal8Bit(p->readAllStandardError()));
            }
        }

        deleteFile(mInputFile);
        writeMetadata();

        emit trackReady(track(), outFile(), mTrackGain.result());
    }
    catch (const FlaconError &err) {
        deleteFile(mInputFile);
        QString msg = tr("Track %1. Encoder error:", "Track error message, %1 is a track number").arg(track().trackNumTag()) + "<pre>" + err.what() + "</pre>";
        emit    error(track(), msg);
    }
}

/************************************************
 *
 ************************************************/
void Encoder::writeMetadata() const
{
    MetadataWriter *writer = mProfile.outFormat()->createMetadataWriter(outFile());
    if (!writer) {
        return;
    }

    writer->setTags(mTrack);
    if (profile().isEmbedCue()) {
        writer->setEmbeddedCue(embeddedCue());
    }

    if (!coverImage().isEmpty()) {
        writer->setCoverImage(coverImage());
    }

    writer->save();
    delete writer;
}

/************************************************

 ************************************************/
void Encoder::processBytesWritten(qint64 bytes)
{
    mReady += bytes;
    int p = ((mReady * 100.0) / mTotal);
    if (p != mProgress) {
        mProgress = p;
        emit trackProgress(track(), TrackState::Encoding, mProgress);
    }
}

/************************************************

 ************************************************/
void Encoder::setProfile(const Profile &profile)
{
    mProfile = profile;
}

/************************************************

 ************************************************/
void Encoder::setCoverImage(const CoverImage &value)
{
    mCoverImage = value;
}

/************************************************

 ************************************************/
void Encoder::readInputFile(QProcess *process)
{
    qCDebug(LOG) << "Read " << inputFile() << "file";
    QFile file(inputFile());
    if (!file.open(QFile::ReadOnly)) {
        emit error(track(), tr("I can't read %1 file", "Encoder error. %1 is a file name.").arg(inputFile()));
    }

    mProgress = -1;
    mTotal    = file.size();

    quint64    bufSize = qBound(MIN_BUF_SIZE, mTotal / 200, MAX_BUF_SIZE);
    QByteArray buf;

    while (!file.atEnd()) {
        buf = file.read(bufSize);
        process->write(buf);
        if (mReplayGainEnabled) {
            mTrackGain.add(buf.constData(), buf.size());
        }
    }
}

/************************************************

 ************************************************/
void Encoder::copyFile()
{
    QFile srcFile(inputFile());
    bool  res = srcFile.rename(outFile());

    if (!res) {
        emit error(track(),
                   tr("I can't rename file:\n%1 to %2\n%3").arg(inputFile(), outFile(), srcFile.errorString()));
    }
}

/************************************************

 ************************************************/
QStringList Encoder::resamplerArgs(int bitsPerSample, int sampleRate, const QString &outFile)
{
    QStringList args;

    args << "--type"
         << "wav";

    args << "-"; // Read from STDIN
    if (bitsPerSample) {
        args << "-b" << QStringLiteral("%1").arg(bitsPerSample);
    }

    args << "--type"
         << "wav";
    args << outFile;

    if (sampleRate) {
        args << "rate";
        args << "-v"; // very high quality
        args << QStringLiteral("%1").arg(sampleRate);
    }

    return args;
}

/************************************************

 ************************************************/
QStringList Encoder::deemphasisArgs(const QString &outFile)
{
    QStringList args;

    // clang-format off
    args << "--type" << "wav" << "-"; // Read from STDIN
    args << "--type" << "wav" << outFile;
    args << "deemph";
    // clang-format on

    return args;
}
