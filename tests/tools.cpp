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
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QCryptographicHash>
#include <QIODevice>
#include <QBuffer>
#include <QDebug>
#include "../settings.h"
#include "../cue.h"
#include "../disc.h"
#include "../converter/decoder.h"
#include <QDirIterator>
#include "testflacon.h"
#include "QJsonObject"
#include "QJsonArray"

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
                mHash.addData(mBuf.data() + n + 8, mBuf.length() - n - 8);
            }
            return len;
        }

        mHash.addData(data, len);
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
        FAIL(QString("Can't open input file '%1': %2").arg(fileName, err.what()).toLocal8Bit());
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
        QFAIL(QString("Can't create cue file '%1': %2").arg(mFileName).arg(f.errorString()).toLocal8Bit());

    QTextStream cue(&f);

    cue << QString("FILE \"%1\" WAVE\n").arg(mWavFile);
    for (int i = 0; i < mTracks.count(); ++i) {
        TestCueTrack track = mTracks.at(i);

        cue << QString("\nTRACK %1 AUDIO\n").arg(i + 1);
        if (track.index0 != "")
            cue << QString("  INDEX 00 %1\n").arg(track.index0);

        if (track.index1 != "")
            cue << QString("  INDEX 01 %1\n").arg(track.index1);
    }

    f.close();
}

/************************************************
 *
 ************************************************/
bool compareAudioHash(const QString &file1, const QString &expected)
{
    if (calcAudioHash(file1) != expected) {
        FAIL(QString("Compared hases are not the same for:\n"
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
                throw QString("Incorrect HEX data at %1:\n%2").arg(i).arg(line);

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
        QFAIL(QString("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());

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
        QFAIL(QString("Can't create file '%1': %2").arg(fileName, file.errorString()).toLocal8Bit());

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

    else if (ext == "flac") {
        program = "flac";
        args << "--silent";
        args << "--force";
        args << "-o" << outFileName;
        args << wavFileName;
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
        QFAIL(QString("Can't create file '%1': unknown file format").arg(outFileName).toLocal8Bit());
    }

    bool     ok = true;
    QProcess proc;
    proc.start(program, args);
    ok = proc.waitForStarted(3 * 1000);
    ok = ok && proc.waitForFinished(5 * 60 * 1000);
    ok = ok && (proc.exitStatus() == 0);

    if (!ok) {
        QFAIL(QString("Can't encode %1 %2:")
                      .arg(program)
                      .arg(args.join(" "))
                      .toLocal8Bit()
              + proc.readAllStandardError());
    }

    if (!QFileInfo(outFileName).isFile()) {
        QFAIL(QString("Can't encode to file '%1' (file don't exists'):")
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
        Cue   cue(cueFile);
        Disc *res = new Disc(cue);
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

 ************************************************/
ConverterTest::ConverterTest(const QString &dataDir, const QString &dir, const QString &tmpDir) :
    mDataDir(dataDir),
    mDir(dir),
    mTmpDir(tmpDir),
    mCfgFile(mDir + "/flacon.conf"),
    mInDir(mDir + "/IN"),
    mOutDir(mDir + "/OUT")

{
    QDir::setCurrent(mDir);

    QFile::copy(dataDir + "/spec.ini", mDir + "/spec.ini");
    QSettings spec(mDir + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    QDir(mInDir).mkpath(".");
    QDir(mOutDir).mkpath(".");

    // Create config ............................
    {
        QString     src  = dataDir + "/flacon.conf";
        QStringList data = readFile(src);
        for (int i = 0; i < data.length(); ++i) {
            data[i] = data[i].replace("@TEST_DIR@", mDir);
        }
        writeFile(data, mCfgFile);
    }

    // Prepare source audio files ...............
    for (const QString &group : spec.childGroups()) {
        if (!group.toUpper().startsWith("SOURCE_AUDIO"))
            continue;

        spec.beginGroup(group);
        QString dest = mInDir + "/" + spec.value("destination").toString();
        QFileInfo(dest).dir().mkpath(".");

        QString method = spec.value("method", "copy").toString().toLower();

        // Copy audio file ......................
        if (method == "copy") {
            QString src = mTmpDir + "/" + spec.value("source").toString();
            ;

            if (!QFile::copy(src, dest))
                QFAIL(QString("Can't copy audio file \"%1\"").arg(src).toLocal8Bit());
        }

        // Generate file ........................
        if (method == "generate") {
            QString header = dataDir + "/" + spec.value("header").toString();
            createWavFile(dest, readFile(header).join("\n"));
        }
        spec.endGroup();
    }
    // ..........................................

    // Copy source CUE files ....................
    spec.beginGroup("Source_CUE");
    foreach (auto key, spec.allKeys()) {
        QString src  = dataDir + "/" + key;
        QString dest = mInDir + "/" + spec.value(key).toString();

        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................

    // Copy source files ....................
    spec.beginGroup("Source_Files");
    foreach (auto key, spec.allKeys()) {
        QString src  = dataDir + "/" + key;
        QString dest = mInDir + "/" + spec.value(key).toString();

        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest)) {
            QFAIL(QString("Can't copy file \"%1\"").arg(src).toLocal8Bit());
        }
    }
    spec.endGroup();
    // ..........................................

    // Copy expected CUE files ....................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys()) {
        QString dest = mDir + "/" + spec.value(key).toString();
        QString src  = dataDir + "/" + spec.value(key).toString();
        QFileInfo(dest).dir().mkpath(".");
        if (!QFile::copy(src, dest))
            QFAIL(QString("Can't copy CUE file \"%1\"").arg(src).toLocal8Bit());
    }
    spec.endGroup();
    // ..........................................
}

/************************************************

 ************************************************/
static bool removeDir(const QString &dirName)
{
    QDir dir(dirName);
    if (!dir.exists())
        return true;

    bool res = false;
    foreach (QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (fi.isDir())
            res = removeDir(fi.absoluteFilePath());
        else
            res = QFile::remove(fi.absoluteFilePath());

        if (!res)
            return false;
    }

    return dir.rmdir(dirName);
}

/************************************************

 ************************************************/
ConverterTest::~ConverterTest()
{
    if (getenv("FLACON_KEEP_TEST_DATA")) {
        return;
    }

    QDir dir(mDir);
    if (!dir.exists()) {
        return;
    }

    foreach (QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (fi.isDir())
            removeDir(fi.absoluteFilePath());
        else
            QFile::remove(fi.absoluteFilePath());
    }
}

/************************************************

 ************************************************/
bool ConverterTest::run()
{
    QString     flacon = QCoreApplication::applicationDirPath() + "/../flacon";
    QStringList args;
    args << "--config" << mCfgFile;
    args << "--start";
    args << "--debug";
    args << mInDir.toLocal8Bit().data();

    createStartSh(mDir + "/start.sh", flacon, args);

    QProcess proc;
    proc.setStandardErrorFile(mDir + "/out.log");
    proc.start(flacon, args);

    if (!proc.waitForFinished(30 * 1000)) {
        FAIL(QString("The program timed out waiting for the result: %1.")
                     .arg(QString::fromLocal8Bit(proc.readAllStandardError()))
                     .toLocal8Bit());
        return false;
    }

    if (proc.exitCode() != 0) {
        FAIL(QString("flacon returned non-zero exit status %1: %2")
                     .arg(proc.exitCode())
                     .arg(QString::fromLocal8Bit(proc.readAll()))
                     .toLocal8Bit());
        return false;
    }

    return true;
}

/************************************************

 ************************************************/
void ConverterTest::check()
{
    QSettings spec(mDir + "/spec.ini", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    QString     msg   = "";
    QStringList files = findFiles(mOutDir, "*");
    QStringList missing;

    // ..........................................
    spec.beginGroup("Result_Audio");
    foreach (auto key, spec.allKeys()) {
        QString file = key;
        QString hash = spec.value(key).toString();

        if (files.removeAll(file) == 0) {
            missing << file;
            continue;
        }

        if (!hash.isEmpty()) {
            if (!compareAudioHash(mOutDir + "/" + file, hash))
                QFAIL("");
        }
    }
    spec.endGroup();
    // ..........................................

    // ..........................................
    spec.beginGroup("Result_CUE");
    foreach (auto key, spec.allKeys()) {
        QString file     = key;
        QString expected = dir() + "/" + spec.value(key).toString();

        if (files.removeAll(file) == 0) {
            missing << file;
            continue;
        }

        QString err;
        if (!TestFlacon::compareCue(mOutDir + "/" + file, expected, &err))
            msg += "\n" + err;
    }
    spec.endGroup();
    // ..........................................

    // ..........................................
    spec.beginGroup("Result_Files");
    foreach (auto key, spec.allKeys()) {

        if (files.removeAll(key) == 0) {
            missing << key;
            continue;
        }
    }
    spec.endGroup();
    // ..........................................

    //    // ******************************************
    //    // Check commands
    //    spec.beginGroup("Check_Commands");
    //    foreach (auto key, spec.allKeys()) {
    //        QString cmd = spec.value(key).toString();
    //        int     res = QProcess::execute(cmd);
    //        if (res != 0) {
    //            QFAIL(QString("Chack is failed for %1").arg(cmd).toLocal8Bit());
    //        }
    //    }
    //    spec.endGroup();

    //    // ******************************************

    if (!missing.isEmpty())
        msg += QString("\nFiles not exists in %1:\n  * %2")
                       .arg(mOutDir)
                       .arg(missing.join("\n  * "));

    if (!files.isEmpty())
        msg += QString("\nFiles exists in %1:\n  * %2")
                       .arg(mOutDir)
                       .arg(files.join("\n  * "));

    if (!msg.isEmpty())
        QFAIL(QString("%1\n%2").arg(QTest::currentDataTag()).arg(msg).toLocal8Bit().data());

    // ******************************************
    // Check tags
    bool tagsError = false;
    spec.beginGroup("Result_Tags");
    foreach (auto file, spec.childGroups()) {

        Mediainfo mediainfo(mOutDir + "/" + file);
        mediainfo.save(mOutDir + "/" + file + ".json");

        spec.beginGroup(file);

        try {
            foreach (auto tag, spec.allKeys()) {

                QVariant actual   = mediainfo.value(tag);
                QVariant expected = spec.value(tag);

                // qDebug() << "-=-=-==-=-=-=-=-=-=-=-=-=-";
                // qDebug() << "actual:" << actual;
                // qDebug() << "expected:" << expected;
                // qDebug() << "-=-=-==-=-=-=-=-=-=-=-=-=-";
                if (actual != expected) {
                    QWARN(QString("Compared tags are not the same:\n"
                                  "    File: %1\n"
                                  "    Tag:  %2\n"
                                  "\n"
                                  "    Actual   : '%3' (%4)\n"
                                  "    Expected : '%5' (%6)\n")

                                  .arg(file)
                                  .arg(tag)

                                  .arg(QString::fromLocal8Bit(actual.toByteArray()))
                                  .arg(actual.toByteArray().toHex(' ').data())

                                  .arg(QString::fromLocal8Bit(expected.toByteArray()))
                                  .arg(expected.toByteArray().toHex(' ').data())

                                  .toLocal8Bit());
                    tagsError = true;
                }
            }
        }
        catch (const FlaconError &err) {
            QFAIL(err.what());
        }

        spec.endGroup();
    }
    spec.endGroup();

    if (tagsError) {
        QFAIL("Some tags not the same");
    }
}

/************************************************

 ************************************************/
QStringList ConverterTest::readFile(const QString &fileName)
{
    QStringList res;
    QFile       file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isOpen()) {
        FAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());
        return res;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString str = stream.readLine();
        res << str << "\n";
    }

    file.close();
    return res;
}

/************************************************

 ************************************************/
void ConverterTest::writeFile(const QStringList &strings, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    if (!file.isOpen())
        QFAIL(QString("Can't open file %1: %2").arg(file.fileName(), file.errorString()).toLocal8Bit().data());

    foreach (const QString &string, strings) {
        file.write(string.toLocal8Bit());
    }
}

/************************************************

 ************************************************/
void ConverterTest::createStartSh(const QString fileName, const QString flaconBin, const QStringList &args) const
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write("\"" + flaconBin.toLocal8Bit() + "\"");
    for (QString a : args) {
        a.replace("\"", "\\\"");
        file.write(" \\\n    \"" + a.toLocal8Bit() + "\"");
    }

    file.setPermissions(QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther | QFileDevice::ExeUser);

    file.close();
}

/************************************************

 ************************************************/
QStringList ConverterTest::findFiles(const QString &dir, const QString &pattern) const
{
    QStringList  res;
    QDirIterator it(dir, QStringList() << pattern, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        res << (it.next()).remove(dir + "/");
    }
    return res;
}

/************************************************

 ************************************************/
Mediainfo::Mediainfo(const QString &fileName) :
    mFileName(fileName)
{
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
        FAIL(QString("Can't read tags from \"%1\": %2").arg(mFileName).arg(err).toLocal8Bit());
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
static QVariant search(const QJsonObject &root, const QStringList &path)
{
    QJsonObject obj = root;
    for (const QString &key : path.mid(0, path.length() - 1)) {
        obj = obj.take(key).toObject();
    }

    return obj.take(path.last()).toVariant();
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
