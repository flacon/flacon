#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <QStringList>
#include <QVector>



class TestCueFile
{
public:
    explicit TestCueFile(const QString &fileName);

    QString fileName() const { return mFileName; }

    void setWavFile(const QString &value);
    QString wavFile() const { return mWavFile; }

    void addTrack(const QString &index0, const QString &index1);
    void addTrack(const QString &index1);

    void write();

private:
    struct TestCueTrack {
        QString index0;
        QString index1;

        TestCueTrack():
            index0(""),
            index1("")
        {
        }


        TestCueTrack(const QString &index1):
            index0(""),
            index1(index1)
        {
        }


        TestCueTrack(const QString &index0, const QString &index1):
            index0(index0),
            index1(index1)
        {
        }


    };

    QString mFileName;
    QString mWavFile;
    QVector<TestCueTrack> mTracks;
};

QString makeTestDir();
QStringList shnSplit(const QString &cueFile, const QString &audioFile);
QString calcAudioHash(const QString &fileName);
void  compareAudioHash(const QString &file1, const QString &file2);

#define FAIL(message) \
do {\
    QTest::qFail(message, __FILE__, __LINE__);\
} while (0)


#endif // TOOLS_H
