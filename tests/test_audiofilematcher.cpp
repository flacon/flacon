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

#include "flacontest.h"
#include <QTest>
#include "tools.h"
#include <QDebug>
#include <QDir>

#include "../audiofilematcher.h"
#include <QSettings>
#include "../cue.h"
#include "../types.h"
#include "testspec.h"

void TestFlacon::testAudioFileMatcher()
{
    QLoggingCategory::setFilterRules("InputAudioFile.debug=false\nAudioFileMatcher.debug=false\n");
    QFETCH(QString, dir);

    try {
        TestSpec spec(dir);

        QFileInfo        inFile = dir + "/" + spec["run"]["load"].toString();
        AudioFileMatcher matcher;

        if (inFile.suffix().toLower() == "cue") {
            Cue cue(inFile.filePath());
            matcher.matchForCue(cue);
        }
        else {
            matcher.matchForAudio(inFile.filePath());
        }

        // Validate .............................
        TestSpec::Node expected = spec["expected"];

        if (expected["cue"].exists()) {
            QCOMPARE(matcher.cue().filePath(), expected["cue"].toFileInfo(dir).filePath());
        }

        if (expected["audio"].exists()) {
            QCOMPARE(matcher.audioFilePaths(), expected["audio"].toFileInfoList(dir));
        }

        if (expected["file_tags"].exists()) {
            QCOMPARE(matcher.fileTags(), expected["file_tags"].toStringList());
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
