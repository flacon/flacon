/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2023
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

#ifndef TEST_VALIDATOR_CPP
#define TEST_VALIDATOR_CPP

#include <QTest>
#include "flacontest.h"
#include "types.h"
#include "validator.h"
#include "settings.h"
#include "json_struct.h"
#include "tools.h"

namespace {
struct TcDisk
{
    std::string              cue;
    std::vector<std::string> audio;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    JS_OBJ(
            cue,
            audio,
            errors,
            warnings);
};

struct TcCase
{
    std::vector<TcDisk> disks;
    JS_OBJECT(JS_MEMBER(disks));
};

TcCase loadTestCase(const QString &dir)
{
    QFile f(dir + "/spec.json");
    if (!f.open(QFile::ReadOnly)) {
        throw FlaconError("Spec file not found");
    }

    QByteArray       data = f.readAll();
    JS::ParseContext context(data.constData(), data.size());

    TcCase res;
    if (context.parseTo(res) != JS::Error::NoError) {
        throw FlaconError(context.makeErrorString());
    }

    return res;
}

}

/************************************************
 *
 ************************************************/
void TestFlacon::testValidator()
{
    QFETCH(QString, dataDir);
    try {
        QString cfgFile = dir() + "/flacon.conf";
        QDir::setCurrent(dir());

        // Create config ............................
        {
            QString     src  = dataDir + "/flacon.conf";
            QStringList data = readFile(src);
            for (int i = 0; i < data.length(); ++i) {
                data[i] = data[i].replace("@TEST_DIR@", dir());
            }
            writeFile(data, cfgFile);
        }

        TcCase spec = loadTestCase(dataDir);

        for (auto d : spec.disks) {
            QFile::copy(QString("%1/%2").arg(dataDir).arg(d.cue.c_str()), QString("%1/%2").arg(dir()).arg(d.cue.c_str()));
            for (auto a : d.audio) {
                QFile::copy(QString("%1/%2").arg(dataDir).arg(a.c_str()), QString("%1/%2").arg(dir()).arg(a.c_str()));
            }
        }

        TestSettings settings(cfgFile);

        Profile profile = settings.readProfile(settings.readCurrentProfileId());
        settings.readExtPrograms();

        Validator validator;
        validator.setProfile(&profile);

        for (auto d : spec.disks) {
            Cue cue(d.cue.c_str());

            Disk *disk = new Disk(cue, &validator);

            for (size_t i = 0; i < d.audio.size(); ++i) {
                InputAudioFile audio(d.audio[i].c_str());
                disk->setAudioFile(audio, i);
            }

            validator.insertDisk(disk);
        }

        validator.revalidate();

        for (uint i = 0; i < spec.disks.size(); ++i) {
            QStringList expected;
            for (auto s : spec.disks[i].errors) {
                expected << QString::fromStdString(s).simplified();
            }

            QStringList actual;
            for (auto s : validator.diskErrors(validator.disks().at(i))) {
                actual << s.simplified();
            }

            if (actual != expected) {
                QFAIL(QString("Compared errors differ.\n"
                              "   Actual  : \"%1\"\n"
                              "   Expected: \"%2\"")
                              .arg(actual.join("\", \""), expected.join("\", \""))
                              .toLocal8Bit());
            }
        }

        for (uint i = 0; i < spec.disks.size(); ++i) {
            QStringList expected;
            for (auto s : spec.disks[i].warnings) {
                expected << QString::fromStdString(s).simplified();
            }

            QStringList actual;
            for (auto s : validator.diskWarnings(validator.disks().at(i))) {
                actual << s.simplified();
            }

            if (actual != expected) {
                QFAIL(QString("Compared warnings differ.\n"
                              "   Actual  : \"%1\"\n"
                              "   Expected: \"%2\"")
                              .arg(actual.join("\", \""), expected.join("\", \""))
                              .toLocal8Bit());
            }
            QCOMPARE(actual, expected);
        }
    }
    catch (const FlaconError &err) {
        QFAIL(err.what());
    }
}

/************************************************
 *
 ************************************************/
void TestFlacon::testValidator_data()
{
    QString curDir = QDir::currentPath();

    QTest::addColumn<QString>("dataDir", nullptr);
    QString dataDir = mDataDir + "testValidator";

    for (auto dir : QDir(dataDir).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QTest::newRow(dir.toUtf8()) << dataDir + "/" + dir;
    }
    QDir::setCurrent(curDir);
}

#endif // TEST_VALIDATOR_CPP
