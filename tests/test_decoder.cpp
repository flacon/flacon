#include "testflacon.h"
#include "tools.h"
#include "../converter/decoder.h"


#include <QTest>
#include <QFile>
#include <QByteArray>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>
#include <QDir>
#include <QVector>


//void createCue(const QString &inputFile, const QString &start, const QString &end, const QString &cueFile)
//{
//    QFile f(cueFile);
//    f.open(QFile::WriteOnly | QFile::Truncate);
//    QTextStream cue(&f);

//    cue << QString("FILE \"%1\" WAVE\n").arg(inputFile);

//    cue << QString("TRACK 1 AUDIO\n");
//    cue << QString("  INDEX 01 %1\n").arg(start);

//    cue << QString("TRACK 2 AUDIO\n");
//    cue << QString("  INDEX 01 %1\n").arg(end);

//    f.close();
//}

//QString shnSplit(const QString &cueFile, const QString &audioFile)
//{
//    QString dir = QFileInfo(cueFile).absoluteDir().absolutePath();

//    QStringList args;
//    args << "split";
//    args << "-w";
//    args << "-q";
//    args << "-O" << "always";
//    args << "-n" << "%03d";
//    args << "-t" << "%n-shntool";
//    args << "-x" << "1";            // -x list only extract tracks in list (comma-separated, may contain ranges)
//    args << "-d" << dir;
//    args << "-f" << QDir::toNativeSeparators(cueFile);
//    args << QDir::toNativeSeparators(audioFile);
//    //qDebug() << args;

//    if (QProcess::execute("shntool", args) != 0)
//        FAIL("snhtool was crashed");

//    return QString("%1/001-shntool.wav").arg(dir);
//}



#if 0
void TestFlacon::testDecoder()
{
    QFETCH(QString, inputFile);
    QFETCH(QString, start);
    QFETCH(QString, end);

    QString dir = QDir::cleanPath(QString("%1/testDecoder/%2").arg(mTmpDir).arg(QTest::currentDataTag()));
    QDir().mkpath(dir);


    // Flacon decoder ***************************
    Decoder decoder;
    if (!decoder.open(inputFile))
        QFAIL(QString("Can't open input file '%1': %2").arg(inputFile, decoder.errorString()).toLocal8Bit());

    QString flaconFile = QString("%1/001-flacon.wav").arg(dir);

    if (!decoder.extract(CueTime(start), CueTime(end), flaconFile))
        QFAIL(QString("Can't extract file '%1' [%2-%3]: %4").arg(inputFile, start, end, decoder.errorString()).toLocal8Bit());


    // Flacon decoder ***************************
    QString cueFile = QString("%1/cue.cue").arg(dir);
    createCue(inputFile, start, end, cueFile);
    QString shnFile = shnSplit(cueFile, inputFile);


    // Checks ***********************************
    if (calcAudioHash(flaconFile) != calcAudioHash(shnFile))
    {
        int len = qMax(flaconFile.length(), shnFile.length());
        QFAIL(QString("Compared hases are not the same for:\n"
                     "    %1 [%2]\n"
                     "    %3 [%4]\n")

                    .arg(flaconFile, - len)
                    .arg(calcAudioHash(flaconFile))

                    .arg(shnFile, - len)
                    .arg(calcAudioHash(shnFile))

                    .toLocal8Bit());
    }
}

void TestFlacon::testDecoder_data()
{

    QTest::addColumn<QString>("inputFile");
    QTest::addColumn<QString>("start");
    QTest::addColumn<QString>("end");


    QTest::newRow("001-cd") << mCdAudioFile
                            << "00:00:00"
                            << "00:30:00";


    QTest::newRow("002-cd") << mCdAudioFile
                            << "00:30:00"
                            << "01:30:20";


    QTest::newRow("003-hd") << mHdAudioFile
                            << "00:00:000"
                            << "00:30:000";

    QTest::newRow("003-hd") << mHdAudioFile
                            << "00:30:000"
                            << "01:30:000";
}
#else


struct TestTrack {
    QString start;
    QString end;
};

void TestFlacon::testDecoder()
{

    QString dir = makeTestDir();

    QFETCH(QStringList, data);
    QString inputFile = data.first();

    QVector<TestTrack> tracks;
    for (int i=1; i < data.count(); i+=2)
    {
        TestTrack track = {data.at(i), data.at(i+1) };
        tracks << track;
    }


    // Flacon decoder ***************************
    Decoder decoder;
    if (!decoder.open(inputFile))
        QFAIL(QString("Can't open input file '%1': %2").arg(inputFile, decoder.errorString()).toLocal8Bit());

    for (int i=0; i < tracks.count(); ++i)
    {
        TestTrack track = tracks.at(i);

        QString flaconFile = QString("%1/%2-flacon.wav").arg(dir).arg(i + 1, 3, 10, QChar('0'));

        bool res = decoder.extract(
                    CueTime(track.start),
                    CueTime(track.end),
                    flaconFile);
        if (!res)
            QFAIL(QString("Can't extract file '%1' [%2-%3]: %4")
                  .arg(inputFile)
                  .arg(track.start, track.end)
                  .arg(decoder.errorString()).toLocal8Bit());
    }
    // Flacon decoder ***************************


    // ShnSplitter ******************************
    TestCueFile cueFile(QString("%1/cue.cue").arg(dir));
    cueFile.setWavFile(inputFile);

    for (int i=0; i < tracks.count(); ++i)
    {
        TestTrack track = tracks.at(i);
        cueFile.addTrack(track.start);
    }
    cueFile.addTrack(tracks.last().end);
    cueFile.write();

    QStringList shnFiles = shnSplit(cueFile.fileName(), inputFile);
    // ShnSplitter ******************************


    // Checks ***********************************
    if (shnFiles.length() < tracks.count())
        QFAIL("Not all files was extracted.");

    for (int i=0; i < tracks.count(); ++i)
    {
        compareAudioHash(
                    QString("%1/%2-flacon.wav").arg(dir).arg(i + 1, 3, 10, QChar('0')),
                    shnFiles.at(i));
    }
    // Checks ***********************************

}

void TestFlacon::testDecoder_data()
{
    QTest::addColumn<QStringList>("data");

    QTest::newRow("001-cd") << (QStringList()
                                << mCdAudioFile
                                << "00:00:00"
                                << "00:30:00"


                                << "00:30:00"
                                << "01:30:00"

                                << "01:30:00"
                                << "02:30:00"

                                );

    QTest::newRow("002-cd") << (QStringList()
                                << mCdAudioFile
                                << "00:00:10"
                                << "00:30:00"


                                << "00:30:00"
                                << "01:30:20"

                                << "01:30:20"
                                << "02:30:30"

                                );



    QTest::newRow("003-hd") << (QStringList()
                                << mHdAudioFile
                                << "00:00:000"
                                << "00:30:000"


                                << "00:30:000"
                                << "01:30:000"

                                << "01:30:000"
                                << "02:30:000"

                                );
}


#endif

