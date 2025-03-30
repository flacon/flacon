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

#include "tools.h"

#include <QTest>
#include <QFile>
#include <QProcess>
#include <QBuffer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include "../cue.h"
#include "../disc.h"
#include "../converter/decoder.h"

class HashDevice : public QIODevice
{
public:
    HashDevice(QCryptographicHash::Algorithm method, QObject *parent = nullptr) :
        QIODevice(parent),
        mHash(method),
        mInHeader(true)
    {
    }

    QByteArray result() const { return mHash.result(); }

protected:
    qint64 readData(char *, qint64) { return -1; }

    qint64 writeData(const char *data, qint64 len)
    {
        if (mInHeader) {
            mBuf.append(data, len);
            int n = mBuf.indexOf("data");
            if (n > -1 && n < mBuf.length() - 8) {
                mInHeader = false;
                mHash.addData(mBuf.mid(n + 8));
            }
            return len;
        }

        mHash.addData(QByteArray(data, len));
        return len;
    }

private:
    QByteArray         mBuf;
    QCryptographicHash mHash;
    bool               mInHeader;
};

/************************************************
 *
 ************************************************/
QString calcAudioHash(const QString &fileName)
{
    Conv::Decoder decoder;
    try {
        decoder.open(fileName);
    }
    catch (FlaconError &err) {
        FAIL(QStringLiteral("Can't open input file '%1': %2").arg(fileName, err.what()).toLocal8Bit());
        return "";
    }

    if (!decoder.audioFormat()) {
        FAIL("Unknown format");
        decoder.close();
        return "";
    }

    HashDevice hash(QCryptographicHash::Md5);
    hash.open(QIODevice::WriteOnly);
    decoder.extract(CueTime(), CueTime(), &hash);
    decoder.close();

    return hash.result().toHex();
}

/************************************************
 *
 ************************************************/
TestCueFile::TestCueFile(const QString &fileName) :
    mFileName(fileName)
{
}

/************************************************
 *
 ************************************************/
void TestCueFile::setWavFile(const QString &value)
{
    mWavFile = value;
}

/************************************************
 *
 ************************************************/
void TestCueFile::addTrack(const QString &index0, const QString &index1)
{
    mTracks << TestCueTrack(index0, index1);
}

/************************************************
 *
 ************************************************/
void TestCueFile::addTrack(const QString &index1)
{
    addTrack("", index1);
}

/************************************************
 *
 ************************************************/
void TestCueFile::write()
{
    QFile f(mFileName);
    if (!f.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QStringLiteral("Can't create cue file '%1': %2").arg(mFileName).arg(f.errorString()).toLocal8Bit());

    QTextStream cue(&f);

    cue << QStringLiteral("FILE \"%1\" WAVE\n").arg(mWavFile);
    for (int i = 0; i < mTracks.count(); ++i) {
        TestCueTrack track = mTracks.at(i);

        cue << QStringLiteral("\nTRACK %1 AUDIO\n").arg(i + 1);
        if (track.index0 != "")
            cue << QStringLiteral("  INDEX 00 %1\n").arg(track.index0);

        if (track.index1 != "")
            cue << QStringLiteral("  INDEX 01 %1\n").arg(track.index1);
    }

    f.close();
}

/************************************************
 *
 ************************************************/
bool compareAudioHash(const QString &file1, const QString &expected)
{
    if (calcAudioHash(file1) != expected) {
        FAIL(QStringLiteral(
                     "Compared hases are not the same for:\n"
                     "    [%1] %2\n"
                     "    [%3] %4\n")

                     .arg(calcAudioHash(file1))
                     .arg(file1)

                     .arg(expected)
                     .arg("expected")

                     .toLocal8Bit());
        return false;
    }
    return true;
}

/************************************************
 *
 ************************************************/
void writeHexString(const QString &str, QIODevice *out)
{
    for (QString line : str.split('\n')) {
        for (int i = 0; i < line.length() - 1;) {
            // Skip comments
            if (line.at(i) == '/')
                break;

            if (line.at(i).isSpace()) {
                ++i;
                continue;
            }

            union {
                quint16 n16;
                char    b;
            };

            bool ok;
            n16 = line.mid(i, 2).toShort(&ok, 16);
            if (!ok)
                throw QStringLiteral("Incorrect HEX data at %1:\n%2").arg(i).arg(line);

            out->write(&b, 1);
            i += 2;
        }
    }
}

/************************************************
 *
 ************************************************/
QByteArray writeHexString(const QString &str)
{
    QBuffer data;
    data.open(QBuffer::ReadWrite);
    writeHexString(str, &data);
    return data.buffer();
}

/************************************************
 *
 ************************************************/
static void writeTestWavData(QIODevice *device, quint64 dataSize)
{
    static const int BUF_SIZE = 1024 * 1024;

    quint32 x = 123456789, y = 362436069, z = 521288629;
    union {
        quint32 t;
        char    bytes[4];
    };

    QByteArray buf;

    buf.reserve(4 * 1024 * 1024);
    buf.reserve(BUF_SIZE);
    for (uint i = 0; i < (dataSize / sizeof(quint32)); ++i) {
        // xorshf96 ...................
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        // xorshf96 ...................

        buf.append(bytes, 4);
        if (buf.size() >= BUF_SIZE) {
            device->write(buf);
            buf.resize(0);
        }
    }

    device->write(buf);
}

/************************************************
 *
 ************************************************/
void createWavFile(const QString &fileName, const QString &header, const int duration)
{
    QBuffer wavHdr;
    wavHdr.open(QBuffer::ReadWrite);
    writeHexString(header, &wavHdr);

    if (duration) {
        quint32 bitsPerSample =
                (quint8(wavHdr.buffer()[28]) << 0) + (quint8(wavHdr.buffer()[29]) << 8) + (quint8(wavHdr.buffer()[30]) << 16) + (quint8(wavHdr.buffer()[31]) << 24);

        quint32 ckSize     = bitsPerSample * duration + wavHdr.buffer().size() - 8 + 8;
        wavHdr.buffer()[4] = quint8(ckSize >> 0);
        wavHdr.buffer()[5] = quint8(ckSize >> 8);
        wavHdr.buffer()[6] = quint8(ckSize >> 16);
        wavHdr.buffer()[7] = quint8(ckSize >> 24);
    }

    // See http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    quint32 ckSize =
            (quint8(wavHdr.buffer()[4]) << 0) + (quint8(wavHdr.buffer()[5]) << 8) + (quint8(wavHdr.buffer()[6]) << 16) + (quint8(wavHdr.buffer()[7]) << 24);

    quint32 dataSize = ckSize - wavHdr.size();

    QFile file(fileName);

    if (file.exists() && file.size() == ckSize + 8)
        return;

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QStringLiteral("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());

    file.write(wavHdr.buffer());
    file.write("data", 4);
    char buf[4];
    buf[0] = quint8(dataSize);
    buf[1] = quint8(dataSize >> 8);
    buf[2] = quint8(dataSize >> 16);
    buf[3] = quint8(dataSize >> 24);
    file.write(buf, 4);

    writeTestWavData(&file, dataSize);
    file.close();
}

/************************************************
 *
 ************************************************/
class TestWavHeader : public Conv::WavHeader
{
public:
    TestWavHeader(quint16 bitsPerSample, quint32 sampleRate, quint16 numChannels, uint durationSec)
    {
        mFormat        = Conv::WavHeader::Format_PCM;
        mNumChannels   = numChannels;
        mSampleRate    = sampleRate;
        mBitsPerSample = bitsPerSample;
        mFmtSize       = FmtChunkSize::FmtChunkMin;
        mByteRate      = mSampleRate * mBitsPerSample / 8 * mNumChannels;
        mBlockAlign    = mBitsPerSample * mNumChannels / 8;

        mDataStartPos = 12 + 8 + mFmtSize;
        mDataSize     = durationSec * mByteRate;

        mFileSize = mDataStartPos + mDataSize;

        mExtSize = 0;
    }
};

/************************************************
 *
 ************************************************/
void createWavFile(const QString &fileName, quint16 bitsPerSample, quint32 sampleRate, uint durationSec)
{
    TestWavHeader header(bitsPerSample, sampleRate, 2, durationSec);

    QFile file(fileName);

    if (file.exists() && quint64(file.size()) != header.fileSize() + 8)
        return;

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QStringLiteral("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());

    file.write(header.toByteArray());
    writeTestWavData(&file, header.dataSize());
    file.close();
}

/************************************************
 *
 ************************************************/
void encodeAudioFile(const QString &wavFileName, const QString &outFileName)
{
    if (QFileInfo(outFileName).exists() && QFileInfo(outFileName).size() > 1024)
        return;

    QString     program;
    QStringList args;

    QString ext = QFileInfo(outFileName).suffix();

    if (ext == "ape") {
        program = "mac";
        args << wavFileName;
        args << outFileName;
        args << "-c2000";
    }

    else if (ext == "aac") {
        program = "faac";
        args << "-w";
        args << "-o" << outFileName;
        args << wavFileName;
    }

    else if (ext == "alac") {
        program = "alacenc";
        args << "--quiet";
        args << "--fast";
        args << wavFileName;
        args << outFileName;
    }

    else if (ext == "flac") {
        program = "flac";
        args << "--silent";
        args << "--force";
        args << "-o" << outFileName;
        args << wavFileName;
    }

    else if (ext == "mp3") {
        program = "lame";
        args << "--silent";
        args << "--preset"
             << "medium";
        args << wavFileName;
        args << outFileName;
    }

    else if (ext == "ogg") {
        program = "oggenc";
        args << "--quiet";
        args << "-o" << outFileName;
        args << wavFileName;
    }

    else if (ext == "opus") {
        program = "opusenc";
        args << "--quiet";
        args << wavFileName;
        args << outFileName;
    }

    else if (ext == "wv") {
        program = "wavpack";
        args << wavFileName;
        args << "-y";
        args << "-q";
        args << "-o" << outFileName;
    }

    else if (ext == "tta") {
        program = "ttaenc";
        args << "-o" << outFileName;
        args << "-e";
        args << wavFileName;
        args << "/";
    }

    else {
        QFAIL(QStringLiteral("Can't create file '%1': unknown file format").arg(outFileName).toLocal8Bit());
    }

    bool     ok = true;
    QProcess proc;
    proc.start(program, args);
    ok = proc.waitForStarted(3 * 1000);
    ok = ok && proc.waitForFinished(5 * 60 * 1000);
    ok = ok && (proc.exitStatus() == 0);

    if (!ok) {
        QFAIL(QStringLiteral("Can't encode %1 %2:")
                      .arg(program)
                      .arg(args.join(" "))
                      .toLocal8Bit()
              + proc.readAllStandardError());
    }

    if (!QFileInfo(outFileName).isFile()) {
        QFAIL(QStringLiteral("Can't encode to file '%1' (file don't exists'):")
                      .arg(outFileName)
                      .toLocal8Bit()
              + proc.readAllStandardError());
    }
}

/************************************************
 *
 ************************************************/
void testFail(const QString &message, const char *file, int line)
{
    QTest::qFail(message.toLocal8Bit().data(), file, line);
}

/************************************************
 *
 ************************************************/
Disc *loadFromCue(const QString &cueFile)
{
    try {
        Disc *res = new Disc();
        res->setCue(Cue(cueFile));
        return res;
    }
    catch (FlaconError &err) {
        FAIL(err.what());
    }
    return nullptr;
}

/************************************************
 *
 ************************************************/
void TestSettings::apply(const QMap<QString, QVariant> &values)
{
    for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
        setValue(i.key(), i.value());
    }
    sync();
}

/************************************************
 *
 ************************************************/
static QByteArray readCue(const QString &fileName, bool skipEmptyLines)
{

    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QByteArray res;
    res.reserve(file.size());
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.startsWith("REM COMMENT"))
            continue;

        if (skipEmptyLines && line.isEmpty())
            continue;

        res += line;
        res += "\n";
    }
    file.close();
    return res;
}

/************************************************

 ************************************************/
bool compareCue(const QString &result, const QString &expected, QString *error, bool skipEmptyLines)
{
    QByteArray resData = readCue(result, skipEmptyLines);
    QByteArray expData = readCue(expected, skipEmptyLines);

    if (resData != expData) {
        QString s = "The result is different from the expected. Use the following command for details: \n diff -uw \"%1\" \"%2\"";
        *error    = s.arg(expected, result);
        return false;
    }

    return true;
}

/************************************************

 ************************************************/
Mediainfo::Mediainfo(const QString &fileName) :
    mFileName(fileName)
{
    if (!QFileInfo::exists(fileName)) {
        throw FlaconError(QStringLiteral("Can't read mediainfo to file '%1' (file don't exists').").arg(mFileName));
    }

    mFileExt = QFileInfo(fileName).suffix().toLower();

    QStringList args;
    args << "--Full";
    args << "--Output=JSON";
    args << mFileName;

    QProcess proc;
    proc.setEnvironment(QStringList("LANG=en_US.UTF-8"));
    proc.start("mediainfo", args);
    proc.waitForFinished();
    if (proc.exitCode() != 0) {
        QString err = QString::fromLocal8Bit(proc.readAll());
        FAIL(QStringLiteral("Can't read tags from \"%1\": %2").arg(mFileName).arg(err).toLocal8Bit());
    }

    QByteArray data = proc.readAllStandardOutput();
    mJsonDoc        = QJsonDocument::fromJson(data);
}

/************************************************

 ************************************************/
void Mediainfo::save(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(mJsonDoc.toJson(QJsonDocument::Indented));
    file.close();
}

/************************************************

 ************************************************/
QVariant Mediainfo::value(const QString &key)
{
    QJsonArray arr = mJsonDoc["media"]["track"].toArray();

    QStringList path = key.split("/");

    if (path.isEmpty()) {
        return {};
    }

    for (QJsonValue v : arr) {
        QJsonObject obj = v.toObject();
        QVariant    res = search(obj, path);

        if (!res.isNull()) {
            return res;
        }
    }
    return {};
}

/**************************************
 * Validate tags
 **************************************/
void Mediainfo::validateTags(const QMap<QString, QVariant> &expected)
{
    bool tagsError = false;
    try {
        foreach (auto tag, expected.keys()) {

            QString  path   = tagToJsonPath(tag);
            QVariant actual = value(path);
            QVariant expect = expected.value(tag);

            if (tag == "extra/CUESHEET") {
                actual = '\n' + trimmCueSheet(actual.toByteArray());
                expect = '\n' + trimmCueSheet(expect.toByteArray());
            }

            if (actual != expect) {
                printError(mFileName, tag, actual, expect);
                tagsError = true;
            }
        }
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }

    if (tagsError) {
        qWarning() << "metadata.json";
        for (const QByteArray line : mJsonDoc.toJson(QJsonDocument::Indented).split('\n')) {
            qWarning().noquote().nospace() << line;
        }
        QFAIL("Some tags not the same");
    }
}

/**************************************
 * Validate tags
 **************************************/
void Mediainfo::validateTags(const QJsonObject &expected)
{
    QMap<QString, QVariant> map;
    for (const QString &key : expected.keys()) {
        map[key] = expected[key].toVariant();
    }

    validateTags(map);
}

/**************************************
 *
 **************************************/
QByteArray Mediainfo::trimmCueSheet(const QByteArray &cue) const
{
    QByteArray res;
    for (QByteArray line : cue.split('/')) {
        line = line.trimmed();
        if (!line.isEmpty()) {
            res += line;
            res += '\n';
        }
    }

    return res;
}

/**************************************
 *
 **************************************/
void Mediainfo::printError(const QString &file, const QString &tag, const QVariant &actual, const QVariant &expected) const
{
    qWarning().noquote() << "Compared values are not the same:";
    qWarning().noquote() << "    File: " << file;
    qWarning().noquote() << "    Tag:  " << tag;
    qWarning().noquote() << "";
    qWarning().noquote() << "    Actual str   :" << QString::fromLocal8Bit(actual.toByteArray());
    qWarning().noquote() << "    Expected str :" << QString::fromLocal8Bit(expected.toByteArray());
    qWarning().noquote() << "    Actual hex   :" << actual.toByteArray().toHex(' ').data();
    qWarning().noquote() << "    Expected hex :" << expected.toByteArray().toHex(' ').data();
}

/**************************************
 *
 **************************************/
QString Mediainfo::tagToJsonPath(const QString &tag) const
{
    // clang-format off
    if (mFileExt == "wv") {
        if (tag == "AlbumPerformer") return "extra/ALBUM_ARTIST";
    }

    if (tag == "Date")           return "Recorded_Date";
    if (tag == "AlbumPerformer") return "Album_Performer";
    if (tag == "SongWriter")     return "Composer";
    if (tag == "Catalog")        return "extra/CATALOGNUMBER";

    return tag;
    //  clang-format on
}

/**************************************
 *
 **************************************/
QVariant Mediainfo::search(const QJsonObject &root, const QStringList &path) const
{
    QJsonObject obj = root;
    for (const QString &key : path.mid(0, path.length() - 1)) {
        obj = obj.take(key).toObject();
    }

    return obj.take(path.last()).toVariant();
}
