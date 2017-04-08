#include "tools.h"

#include <QTest>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QCryptographicHash>

#include <QDebug>


QString makeTestDir()
{
    QString dir = QDir::cleanPath(QString("%1/testDecoder/%2").arg(TEST_OUT_DIR).arg(QTest::currentDataTag()));
    if (!QDir().mkpath(dir))
    {
        QTest::qFail(QString("Can't create directory '%1'").arg(dir).toLocal8Bit(), __FILE__, __LINE__);
        return "";
    }

    return dir;
}

QString calcAudioHash(const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray ba = f.read(1024);
    int n = ba.indexOf("data");
    f.seek(n + 8);

    QCryptographicHash hash(QCryptographicHash::Md5);
    while (!f.atEnd())
    {
        ba = f.read(1024 * 1024);
        hash.addData(ba);
    }

    return hash.result().toHex();
}


TestCueFile::TestCueFile(const QString &fileName):
    mFileName(fileName)
{
}

void TestCueFile::setWavFile(const QString &value)
{
    mWavFile = value;
}

void TestCueFile::addTrack(const QString &index0, const QString &index1)
{
    mTracks << TestCueTrack(index0, index1);
}

void TestCueFile::addTrack(const QString &index1)
{
    addTrack("", index1);
}

void TestCueFile::write()
{
    QFile f(mFileName);
    if (!f.open(QFile::WriteOnly | QFile::Truncate))
        QFAIL(QString("Can't create cue file '%1': %2").arg(mFileName).arg(f.errorString()).toLocal8Bit());

    QTextStream cue(&f);

    cue << QString("FILE \"%1\" WAVE\n").arg(mWavFile);
    for (int i=0; i < mTracks.count(); ++i)
    {
        TestCueTrack track = mTracks.at(i);

        cue << QString("\nTRACK %1 AUDIO\n").arg(i + 1);
        if (track.index0 != "")
            cue << QString("  INDEX 00 %1\n").arg(track.index0);

        if (track.index1 != "")
            cue << QString("  INDEX 01 %1\n").arg(track.index1);
    }

    f.close();
}


QStringList shnSplit(const QString &cueFile, const QString &audioFile)
{
    QString dir = QFileInfo(cueFile).absoluteDir().absolutePath();
    foreach (QFileInfo file, QDir(dir).entryInfoList(QStringList() << "*-shntool.wav"))
    {
        QFile::remove(file.absoluteFilePath());
    }

    QStringList args;
    args << "split";
    args << "-w";
    args << "-q";
    args << "-O" << "always";
    args << "-n" << "%03d";
    args << "-t" << "%n-shntool";
    args << "-d" << dir;
    args << "-f" << QDir::toNativeSeparators(cueFile);
    args << QDir::toNativeSeparators(audioFile);
    //qDebug() << args;

    if (QProcess::execute("shntool", args) != 0)
        FAIL("snhtool was crashed");

    QStringList res;
    foreach (QFileInfo file, QDir(dir).entryInfoList(QStringList() << "*-shntool.wav"))
    {
        if (!file.absoluteFilePath().endsWith("000-shntool.wav"))
            res << file.absoluteFilePath();
    }

    return res;
}


void compareAudioHash(const QString &file1, const QString &file2)
{
    if (calcAudioHash(file1) != calcAudioHash(file2))
    {
        int len = qMax(file1.length(), file2.length());
        QFAIL(QString("Compared hases are not the same for:\n"
                     "    %1 [%2]\n"
                     "    %3 [%4]\n")

                    .arg(file1, -len)
                    .arg(calcAudioHash(file1))

                    .arg(file2, -len)
                    .arg(calcAudioHash(file2))

                    .toLocal8Bit());
    }
    QCOMPARE(calcAudioHash(file1), calcAudioHash(file2));
}
