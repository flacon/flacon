#ifndef TEST_VALIDATOR_CPP
#define TEST_VALIDATOR_CPP

#include <QTest>
#include "flacontest.h"
#include "types.h"
#include "validator.h"
#include "settings.h"
#include "json_struct.h"

namespace {
struct TcDisk
{
    std::string              cue;
    std::string              audio;
    std::vector<std::string> errors;

    JS_OBJ(
            cue,
            audio,
            errors);
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

    JS::ParseContext context(f.readAll().data());

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
            QFile::copy(QString("%1/%2").arg(dataDir).arg(d.audio.c_str()), QString("%1/%2").arg(dir()).arg(d.audio.c_str()));
        }

        Settings settings(cfgFile);

        Validator validator;
        validator.setProfile(settings.currentProfile());

        for (auto d : spec.disks) {
            Cue            cue(d.cue.c_str());
            InputAudioFile audio(d.audio.c_str());

            Disk *disk = new Disk(cue, &validator);
            disk->setAudioFile(audio, 0);
            validator.insertDisk(disk);
        }

        validator.revalidate();

        for (uint i = 0; i < spec.disks.size(); ++i) {
            QStringList expected;
            for (auto s : spec.disks[i].errors) {
                expected << QString::fromStdString(s);
            }

            QStringList actual = validator.diskErrors(validator.disks().at(i));
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
