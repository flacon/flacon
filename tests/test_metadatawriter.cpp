/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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

#include "flacontest.h"
#include <QTest>
#include "tools.h"
#include <QDebug>
#include "disc.h"
#include "formats_out/outformat.h"
#include "formats_out/metadatawriter.h"
#include <QJsonObject>
#include "math.h"

using TagsMap = QMap<QString, QVariant>;

static void createCue(const QString &fileName)
{
    QFile f(fileName);
    f.open(QFile::WriteOnly);
    f.write("FILE 'in.wav' WAVE\n");
    f.write("TRACK 01 AUDIO\n");
    f.write("INDEX 00 00:00:00\n");
    f.close();
}

static QJsonObject readSpec(const QString &specFileName)
{
    QFile file(specFileName);
    if (!file.open(QFile::ReadOnly)) {
        throw FlaconError(QStringLiteral("Unable to read '%1': %2").arg(specFileName, file.errorString()));
    }

    QJsonParseError err;
    QJsonDocument   doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        throw FlaconError(QStringLiteral("Unable to read '%1': %2").arg(specFileName, err.errorString()));
    }
    return doc.object();
}

static void setTags(Disc *disk, const QMap<QString, QVariant> &tags)
{
    QMap<QString, AlbumTags::TagId> albumTags;
    albumTags["Album"]          = AlbumTags::TagId::Album;
    albumTags["Catalog"]        = AlbumTags::TagId::Catalog;
    albumTags["CdTextfile"]     = AlbumTags::TagId::CdTextfile;
    albumTags["DiscId"]         = AlbumTags::TagId::DiscId;
    albumTags["AlbumPerformer"] = AlbumTags::TagId::AlbumPerformer;
    albumTags["AlbumArtist"]    = AlbumTags::TagId::AlbumPerformer;

    QMap<QString, TrackTags::TagId> trackTags;
    trackTags["Comment"]    = TrackTags::TagId::Comment;
    trackTags["Date"]       = TrackTags::TagId::Date;
    trackTags["Genre"]      = TrackTags::TagId::Genre;
    trackTags["Isrc"]       = TrackTags::TagId::Isrc;
    trackTags["Title"]      = TrackTags::TagId::Title;
    trackTags["Performer"]  = TrackTags::TagId::Performer;
    trackTags["Artist"]     = TrackTags::TagId::Performer;
    trackTags["SongWriter"] = TrackTags::TagId::SongWriter;

    for (const QString &key : tags.keys()) {
        if (albumTags.contains(key)) {
            disk->setTag(albumTags.value(key), tags.value(key).toString());
            continue;
        }

        if (trackTags.contains(key)) {
            disk->tracks().at(0)->setTag(trackTags.value(key), tags.value(key).toString());
            continue;
        }

        QFAIL(QString("Incorrect spec file: Unknown tag %1").arg(key).toLocal8Bit());
    }
}

void TestFlacon::testMetaDataWriter()
{
    QFETCH(QString, srcDir);
    QFETCH(QString, fileType);
    QFETCH(TagsMap, tags);
    QDir::setCurrent(dir());
    Q_UNUSED(srcDir);

    createCue("in.cue");
    Cue cue("in.cue");

    // Older versions of mediainfo have a bug, and incorrectly display the lone "AlbumPerformer" tag.
    if (fileType == "WV" && tags.keys().join("") == "AlbumPerformer") {
        // QTest::qSkip("Older versions of mediainfo have a bug, and incorrectly display the lone 'AlbumPerformer' tag.", __FILE__, __LINE__);
        return;
    }

    QString audioFile = QString("out.%1").arg(fileType.toLower());
    encodeAudioFile("../../1sec.wav", audioFile);

    Disc disk;
    disk.setCue(cue);
    setTags(&disk, tags);

    Profile         profile;
    OutFormat      *format = OutFormat::formatForId(fileType);
    MetadataWriter *writer = format->createMetadataWriter(profile, audioFile);

    writer->setTags(*disk.tracks().at(0));
    writer->save();

    delete writer;

    Mediainfo mediaInfo(audioFile);
    mediaInfo.save(audioFile + ".json");
    mediaInfo.validateTags(tags);
}

void TestFlacon::testMetaDataWriter_data()
{
    QTest::addColumn<QString>("srcDir", nullptr);
    QTest::addColumn<QString>("fileType", nullptr);
    QTest::addColumn<TagsMap>("tags", nullptr);

    QString srcDir = sourceDir();

    QDir dir(srcDir);
    foreach (QFileInfo d, dir.entryInfoList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QJsonObject spec;
        try {
            spec = readSpec(d.filePath() + "/spec.json");
        }
        catch (const FlaconError &err) {
            QFAIL(err.what());
        }

        QStringList allTags = spec["tags"].toObject().keys();

        for (uint i = 0; i < pow(2, allTags.count()); ++i) {
            TagsMap tags;
            for (int t = 0; t < allTags.count(); ++t) {
                if ((i) & (1 << (t))) {
                    QString key = allTags.at(t);
                    tags[key]   = spec["tags"].toObject()[key].toVariant();
                }
            }

            QString testName = QString("%1_%2 %3").arg(d.fileName()).arg(i, 4, 10, QChar('0')).arg(tags.keys().join(" "));
            QTest::newRow(testName.toLocal8Bit())
                    << d.filePath()
                    << spec["fileType"].toString()
                    << tags;
        }
    }
}
