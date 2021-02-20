#include "discspec.h"
#include <QSettings>

#include "../disc.h"
#include <QTest>

using namespace Tests;

DiscSpec::DiscSpec(const QString &fileName) :
    mData(fileName, QSettings::IniFormat),
    mDir(QFileInfo(fileName).dir().path())
{
    mData.setIniCodec("UTF-8");
    mData.allKeys();
}

QString DiscSpec::cueFilePath() const
{
    return mDir + "/" + mData.value(KEY_CUE_FILE_PATH).toString();
}

int DiscSpec::tracksCount() const
{
    mData.beginGroup("TRACKS");
    int res = mData.childGroups().count();
    mData.endGroup();
    return res;
}

QString DiscSpec::trackAudioFilePath(int index) const
{
    if (trackAudioFile(index).isEmpty()) {
        return "";
    }

    return mDir + "/" + trackAudioFile(index);
}

QString DiscSpec::trackKey(int track, const QString tag) const
{
    return QString("TRACKS/%1/%2").arg(track, 2, 10, QChar('0')).arg(tag);
}

QString DiscSpec::trackValue(int track, const QString &key) const
{
    return mData.value(trackKey(track, key)).toString();
}

void DiscSpec::verify(const Disc &disc) const
{
    QCOMPARE(disc.cueFilePath(), cueFilePath());
    QCOMPARE(disc.count(), tracksCount());
    for (int i = 0; i < tracksCount(); ++i) {
        const Track *track = disc.track(i);
        int          n     = i + 1;
        QCOMPARE(track->title(), trackTitle(n));
        QCOMPARE(track->date(), trackDate(n));
        QCOMPARE(track->discId(), trackDiscId(n));
        QCOMPARE(track->comment(), trackComment(n));
        QCOMPARE(track->tag(TagId::File), trackFile(n));
        QCOMPARE(track->artist(), trackPerformer(n));
        QCOMPARE(track->cueIndex(0).toString(false), trackIndex0(n));
        QCOMPARE(track->cueIndex(1).toString(false), trackIndex1(n));

        QCOMPARE(track->audioFileName(), trackAudioFile(n));
    }
}

namespace {
class Writer
{
public:
    explicit Writer(const QString &fileName) :
        mFile(fileName)
    {
        mFile.open(QFile::WriteOnly);
    }

    ~Writer()
    {
        mFile.flush();
        mFile.close();
    }

    void beginGroup(const QString &group)
    {
        mGroups << group;
        mFile.write("\n[");
        mFile.write(mGroups.join("/").toUtf8());
        mFile.write("]\n");
    }

    void endGroup()
    {
        mGroups.removeLast();
    }

    void write(const QString &key, const QString &value)
    {
        bool quoted = needQuote(value);

        if (!mGroups.isEmpty()) {
            mFile.write("    ");
        }
        mFile.write(key.toUtf8());
        mFile.write(" = ");

        if (quoted) {
            mFile.write("\"");
        }
        mFile.write(value.toUtf8());
        if (quoted) {
            mFile.write("\"");
        }

        mFile.write("\n");
    }

    void write(const QString &key, const CueIndex &value)
    {
        write(key, value.toString(false));
    }

private:
    QFile       mFile;
    QStringList mGroups;

    bool needQuote(const QString &value) const
    {
        // clang-format off
        return
            value.contains(",");
        // clang-format on
    }
};

}
void DiscSpec::write(const Disc &disc, const QString &fileName)
{
    QDir dir = QFileInfo(fileName).dir();

    Writer w(fileName);
    w.write(KEY_CUE_FILE_PATH, QFileInfo(disc.cueFilePath()).fileName());
    w.write(KEY_COVER_FILE, dir.relativeFilePath(disc.coverImageFile()));

    for (int i = 0; i < disc.count(); ++i) {
        const Track *track = disc.track(i);
        w.beginGroup(QString(KEY_TRACK_GROUP).arg(track->trackNum(), 2, 10, QChar('0')));

        w.write(KEY_TRACK_TITLE, track->title());
        w.write(KEY_TRACK_DATE, track->date());
        w.write(KEY_TRACK_DISCID, track->discId());
        w.write(KEY_TRACK_COMMENT, track->comment());
        w.write(KEY_TRACK_FILE, track->tag(TagId::File));
        w.write(KEY_TRACK_PERFORMER, track->artist());
        w.write(KEY_TRACK_TITLE, track->title());
        w.write(KEY_TRACK_INDEX_0, track->cueIndex(0));
        w.write(KEY_TRACK_INDEX_1, track->cueIndex(1));

        w.write(KEY_TRACK_AUDIO_FILE, track->audioFileName());

        w.endGroup();
    }
}
