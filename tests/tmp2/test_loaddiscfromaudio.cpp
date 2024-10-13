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
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QLoggingCategory>
#include <QProcessEnvironment>

#include <QSettings>
#include "settings.h"
#include "../types.h"
#include "../project.h"
#include "discspec.h"

namespace {
class TestProject : public Project
{
public:
    using Project::Project;
};

}

void TestFlacon::testLoadDiscFromAudio()
{
    QLoggingCategory::setFilterRules("InputAudioFile.debug=false\n"
                                     "AudioFileMatcher.debug=false\n"
                                     "SearchAudioFiles.debug=false\n");
    QFETCH(QString, dir);

    QSettings spec(dir + "/test.spec", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    spec.setIniCodec("UTF-8");
#endif

    try {
        TestProject p;
        if (spec.value("LOAD").isNull()) {
            QFAIL("Can't set LOAD tag in the spec file");
        }

        QString fileName = dir + "/" + spec.value("LOAD").toString();
        Disc   *disc     = fileName.endsWith("cue") ? p.addCueFile(fileName) : p.addAudioFile(fileName);

        disc->setCodecName(spec.value("CODEC", "UTF-8").toString());

        Tests::DiscSpec exp(dir + "/disc.expected");
        if (!QFile::exists(exp.fileName())) {
            Tests::DiscSpec::write(*disc, dir + "/_disc.expected");
        }
        exp.verify(*disc);
        delete disc;
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }
}

void TestFlacon::testLoadDiscFromAudio_data()
{
    QTest::addColumn<QString>("dir", nullptr);
    QString srcDir = sourceDir();

    QDir dir(srcDir);
    foreach (QFileInfo f, dir.entryInfoList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(f.fileName().toLocal8Bit()) << f.filePath();
    }
}

void TestFlacon::testLoadDiscFromAudioErrors()
{
    QFETCH(QString, dir);

    QSettings spec(dir + "/test.spec", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    spec.setIniCodec("UTF-8");
#endif

    QString            expected = spec.value("EXPECTED").toString();
    QRegularExpression re(expected);

    QLoggingCategory::setFilterRules("*.debug=false\n*.warning=false\n");

    try {
        if (spec.value("LOAD").isNull()) {
            QFAIL("Can't set LOAD tag in the spec file");
        }

        QFile::copy(dir + "/conf.ini", this->dir() + "/conf.ini");
        Settings::setFileName(this->dir() + "/conf.ini");

        if (spec.contains("PROGRAM")) {

            ExtProgram::flac()->setPath(spec.value("PROGRAM").toString());
        }

        InputAudioFile audio(dir + "/" + spec.value("LOAD").toString());

        QVERIFY(audio.isValid() == false);
        if (!re.match(audio.errorString()).hasMatch()) {
            QFAIL(QString("Error doesen't match to expected\n\tError:    %1\n\tExpected: %2")
                          .arg(audio.errorString(), expected)
                          .toLocal8Bit());
        }
    }
    catch (const FlaconError &err) {
        QLoggingCategory::setFilterRules("");
        QFAIL(err.what());
    }

    QLoggingCategory::setFilterRules("");
    findPrograms();
}

void TestFlacon::testLoadDiscFromAudioErrors_data()
{
    QTest::addColumn<QString>("dir", nullptr);
    QString srcDir = sourceDir();

    QDir dir(srcDir);
    foreach (QFileInfo f, dir.entryInfoList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(f.fileName().toLocal8Bit()) << f.filePath();
    }
}
