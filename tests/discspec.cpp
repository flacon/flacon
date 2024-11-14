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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace Tests;

DiscSpec::DiscSpec(const QString &fileName) :
    mFileName(fileName)
{
}

void DiscSpec::write(const Disc &disk, const QString &fileName)
{
    QDir dir = QFileInfo(fileName).dir();

    QJsonObject json;
    json["cueFile"]   = QFileInfo(disk.cueFilePath()).fileName();
    json["coverFile"] = dir.relativeFilePath(disk.coverImageFile());

    json["discCount"] = disk.discCountTag();
    json["discNum"]   = disk.discNumTag();

    json["albumPerformer"] = disk.albumPerformerTag();
    json["album"]          = disk.albumTag();
    json["catalog"]        = disk.catalogTag();
    json["cdTextfile"]     = disk.cdTextfileTag();
    json["discId"]         = disk.discIdTag();

    QJsonArray tracks;
    for (const Track *track : disk.tracks()) {
        QJsonObject json;
        json["trackNum"]   = track->trackNumTag();
        json["comment"]    = track->commentTag();
        json["date"]       = track->dateTag();
        json["genre"]      = track->genreTag();
        json["flags"]      = track->flagsTag();
        json["isrc"]       = track->isrcTag();
        json["title"]      = track->titleTag();
        json["performer"]  = track->performerTag();
        json["songWriter"] = track->songWriterTag();
        json["file"]       = track->fileTag();

        tracks.append(json);
    }
    json["tracks"] = tracks;

    QJsonDocument doc;
    doc.setObject(json);

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QFAIL(file.errorString().toLocal8Bit());
    }

    file.write(doc.toJson());
    file.close();
}

void DiscSpec::verify(const Disc &disk) const
{
    QFile file(mFileName);
    if (!file.open(QFile::ReadOnly)) {
        QFAIL(QStringLiteral("Unable to read '%1': %2").arg(mFileName, file.errorString()).toLocal8Bit());
    }

    QJsonParseError err;
    QJsonDocument   doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        QFAIL(QStringLiteral("Unable to read '%1': %2").arg(mFileName, err.errorString()).toLocal8Bit());
    }

    verifyDiskTags(disk, doc.object());
}

void DiscSpec::verifyDiskTags(const Disc &disk, const QJsonObject &json) const
{
    QDir dir = QFileInfo(mFileName).dir();

    // clang-format off
    if (json.contains("cueFile"))    QCOMPARE(QFileInfo(disk.cueFilePath()).fileName(), json["cueFile"].toString());
    if (json.contains("coverFile"))  QCOMPARE(dir.relativeFilePath(disk.coverImageFile()), json["coverFile"].toString());

    if (json.contains("discCount"))  QCOMPARE(disk.discCountTag(), json["discCount"].toInt(1));
    if (json.contains("discNum"))    QCOMPARE(disk.discNumTag(), json["discNum"].toInt(1));

    if (json.contains("album"))      QCOMPARE(disk.albumTag(), json["album"].toString());
    if (json.contains("catalog"))    QCOMPARE(disk.catalogTag(), json["catalog"].toString());
    if (json.contains("cdTextfile")) QCOMPARE(disk.cdTextfileTag(), json["cdTextfile"].toString());
    if (json.contains("discId"))     QCOMPARE(disk.discIdTag(), json["discId"].toString());
    // clang-format on

    QJsonArray tracks = json["tracks"].toArray();
    QCOMPARE(disk.tracks().count(), tracks.count());

    int i = 0;
    for (const Track *track : disk.tracks()) {
        verifyTrackTags(track, tracks.at(i).toObject());
        i++;
    }
}

void DiscSpec::verifyTrackTags(const Track *track, const QJsonObject &json) const
{
    if (json.contains("audioFile")) {
        QCOMPARE(track->audioFileName(), json["audioFile"].toString());
    }

    if (json.contains("duration")) {
        int expected = strToDuration(json["duration"].toString());

        if (expected < 0) {
            QFAIL("Invalid track duration");
        }

        if (track->duration() != uint(expected)) {
            qWarning() << "Track" << track->trackNumTag();
            QCOMPARE(track->duration(), Duration(expected));
        }
        return;
    }

    if (json.contains("index00")) {
        QString expected = json["index00"].toString();
        if (track->cueIndex00().toString(true) != expected) {
            QCOMPARE(track->cueIndex00().toString(false), expected);
        }
    }

    if (json.contains("index01")) {
        QString expected = json["index01"].toString();
        if (track->cueIndex01().toString(true) != expected) {
            QCOMPARE(track->cueIndex01().toString(false), expected);
        }
    }

    // clang-format off
    if (json.contains("trackNum"))   QCOMPARE(track->trackNumTag(), json["trackNum"].toInt());
    if (json.contains("comment"))    QCOMPARE(track->commentTag(), json["comment"].toString());
    if (json.contains("date"))       QCOMPARE(track->dateTag(), json["date"].toString());
    if (json.contains("genre"))      QCOMPARE(track->dateTag(), json["genre"].toString());
    if (json.contains("flags"))      QCOMPARE(track->flagsTag(), json["flags"].toString());
    if (json.contains("isrc"))       QCOMPARE(track->isrcTag(), json["isrc"].toString());
    if (json.contains("title"))      QCOMPARE(track->titleTag(), json["title"].toString());
    if (json.contains("performer"))  QCOMPARE(track->performerTag(), json["performer"].toString());
    if (json.contains("songWriter")) QCOMPARE(track->songWriterTag(), json["songWriter"].toString());
    if (json.contains("file"))       QCOMPARE(track->fileTag(), json["file"].toString());
    // clang-format on
}

int DiscSpec::strToDuration(const QString &str) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList sl = str.split(QRegularExpression("\\D"), QString::KeepEmptyParts);
#else
    QStringList sl = str.split(QRegularExpression("\\D"), Qt::KeepEmptyParts);
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

    if (str.contains(".") || sl[2].length() > 2) {
        return (min * 60 + sec) * 1000 + msec;
    }
    else {
        return (min * 60 + sec) * 1000 + frm / 75.0 * 1000.0;
    }
}
