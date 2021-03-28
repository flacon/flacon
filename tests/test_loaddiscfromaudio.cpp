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
    QFETCH(QString, dir);

    QSettings spec(dir + "/test.spec", QSettings::IniFormat);
    spec.setIniCodec("UTF-8");

    try {
        TestProject p;
        if (spec.value("LOAD").isNull()) {
            QFAIL("Can't set LOAD tag in the spec file");
        }

        Disc *disc = p.addAudioFile(dir + "/" + spec.value("LOAD").toString());
        disc->setCodecName(spec.value("CODEC", "UTF-8").toString());

        Tests::DiscSpec exp(dir + "/disc.expected");
        if (!QFile::exists(exp.fileName())) {
            Tests::DiscSpec::write(*disc, dir + "/_disc.expected");
        }
        exp.verify(*disc);
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
