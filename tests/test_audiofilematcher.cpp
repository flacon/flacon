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

#include "testflacon.h"
#include <QTest>
#include "tools.h"
#include <QDebug>
#include <QDir>

#include "../audiofilematcher.h"
#include <QSettings>
#include "../cue.h"
#include "../types.h"

static QString findFile(const QString dir, const QString &pattern)
{
    QFileInfoList files = QDir(dir).entryInfoList(QStringList(pattern), QDir::Files);
    if (files.count() < 1) {
        throw FlaconError(QString("%1 file not found").arg(pattern));
    }

    if (files.count() > 1) {
        throw FlaconError(QString("Multipy %1 files found").arg(pattern));
    }

    return files.first().filePath();
}

void TestFlacon::testAudioFileMatcher()
{
    QLoggingCategory::setFilterRules("InputAudioFile.debug=false\nAudioFileMatcher.debug=false\n");
    QFETCH(QString, dir);

    try {
        QString cueFile      = findFile(dir, "*.cue");
        QString expectedFile = findFile(dir, "*.expected");

        Cue disc(cueFile);

        AudioFileMatcher matcher(cueFile, disc);

        QSettings exp(expectedFile, QSettings::IniFormat);
        exp.setIniCodec("UTF-8");

        QCOMPARE(matcher.fileTags().count(), exp.childGroups().count());

        for (const QString &tag : exp.childGroups()) {

            QStringList actualAudio = matcher.audioFiles(tag);

            exp.beginGroup(tag);

            QStringList keys = exp.childKeys();
            keys.sort();

            QStringList expectedAudio;
            for (const QString &key : keys) {
                if (!exp.value(key).toString().isEmpty()) {
                    expectedAudio << QDir(dir).filePath(exp.value(key).toString());
                }
            }

            exp.endGroup();

            if (!QTest::qCompare(
                        actualAudio.join(", "), expectedAudio.join(", "),
                        QString("actual   %1").arg(tag).toLocal8Bit(),
                        QString("expected %1").arg(tag).toLocal8Bit(),
                        __FILE__, __LINE__)) {
                return;
            }
        }
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }
}

void TestFlacon::testAudioFileMatcher_data()
{
    QTest::addColumn<QString>("dir", nullptr);
    QString srcDir = mDataDir + "testAudioFileMatcher";

    QDir dir(srcDir);
    foreach (QFileInfo f, dir.entryInfoList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(f.fileName().toLocal8Bit()) << f.filePath();
    }
}
