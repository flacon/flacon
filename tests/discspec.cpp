/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#include "discspec.h"
#include <QSettings>
#include <QRegularExpression>

#include "../disc.h"
#include <QTest>

using namespace Tests;

DiscSpec::DiscSpec(const QString &fileName) :
    mData(fileName, QSettings::IniFormat),
    mDir(QFileInfo(fileName).dir().path())
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mData.setIniCodec("UTF-8");
#endif
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

int DiscSpec::durationValue(const QString &key) const
{
    QString s = mData.value(key).toString();

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList sl = s.split(QRegularExpression("\\D"), QString::KeepEmptyParts);
#else
    QStringList sl = s.split(QRegularExpression("\\D"), Qt::KeepEmptyParts);
#endif

    if (sl.length() < 3)
        return -1;

    bool ok;
    int  min = sl[0].toInt(&ok);
    if (!ok)
        return -1;

    int sec = sl[1].toInt(&ok);
    if (!ok)
        return -1;

    int frm = sl[2].leftJustified(2, '0').toInt(&ok);
    if (!ok)
        return -1;

    int msec = sl[2].leftJustified(3, '0').toInt(&ok);
    if (!ok)
        return -1;

    if (s.contains(".") || sl[2].length() > 2) {
        return (min * 60 + sec) * 1000 + msec;
    }
    else {
        return (min * 60 + sec) * 1000 + frm / 75.0 * 1000.0;
    }
}

void DiscSpec::verifyTrack(const Track *track, const QString &key) const
{
    QString expected = mData.value(key).toString();

    QMap<QString, TagId> TAGS;
    TAGS["Album"]       = TagId::Album;
    TAGS["Catalog"]     = TagId::Catalog;
    TAGS["CDTextfile"]  = TagId::CDTextfile;
    TAGS["Comment"]     = TagId::Comment;
    TAGS["Date"]        = TagId::Date;
    TAGS["Flags"]       = TagId::Flags;
    TAGS["Genre"]       = TagId::Genre;
    TAGS["ISRC"]        = TagId::ISRC;
    TAGS["Artist"]      = TagId::Artist;
    TAGS["SongWriter"]  = TagId::SongWriter;
    TAGS["Title"]       = TagId::Title;
    TAGS["DiscId"]      = TagId::DiscId;
    TAGS["File"]        = TagId::File;
    TAGS["DiscNum"]     = TagId::DiscNum;
    TAGS["DiscCount"]   = TagId::DiscCount;
    TAGS["CueFile"]     = TagId::CueFile;
    TAGS["AlbumArtist"] = TagId::AlbumArtist;
    TAGS["TrackNum"]    = TagId::TrackNum;
    TAGS["TrackCount"]  = TagId::TrackCount;
    TAGS["Performer"]   = TagId::Artist;

    QStringList keys = TAGS.keys();
    for (auto key : keys) {
        TAGS[key.toUpper()] = TAGS[key];
    }

    if (TAGS.contains(key)) {
        QCOMPARE(track->tag(TAGS[key]), expected);
        return;
    }

    if (key == KEY_TRACK_INDEX_0) {
        if (track->cueIndex(0).toString(true) != expected) {
            QCOMPARE(track->cueIndex(0).toString(false), expected);
        }
        return;
    }

    if (key == KEY_TRACK_INDEX_1) {
        if (track->cueIndex(1).toString(true) != expected) {
            QCOMPARE(track->cueIndex(1).toString(false), expected);
        }
        return;
    }

    if (key == KEY_TRACK_AUDIO_FILE) {
        QCOMPARE(track->audioFileName(), expected);
        return;
    }

    if (key == KEY_TRACK_DURATION) {
        int expected = durationValue(key);
        if (expected < 0) {
            QFAIL("Invalid track duration");
        }

        if (track->duration() != uint(expected)) {
            qWarning() << "Track" << track->trackNum();
            QCOMPARE(track->duration(), Duration(expected));
        }
        return;
    }

    QFAIL(QString("Unknown expected key: %1").arg(key).toLocal8Bit());
}

void DiscSpec::verify(const Disc &disc) const
{
    if (mData.contains(KEY_CUE_FILE_PATH)) {
        QCOMPARE(disc.cueFilePath(), cueFilePath());
    }

    QCOMPARE(disc.count(), tracksCount());

    mData.beginGroup("TRACKS");
    for (const QString &group : mData.childGroups()) {
        Track *track;
        {
            bool ok;
            int  n = group.toInt(&ok);
            if (!ok) {
                QFAIL("Invalid Track number");
                return;
            }
            track = disc.track(n - 1);
        }
        mData.beginGroup(group);
        for (const QString &key : mData.childKeys()) {
            verifyTrack(track, key);
        }
        mData.endGroup();
    }
    mData.endGroup();
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
        return value.contains(",");
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
