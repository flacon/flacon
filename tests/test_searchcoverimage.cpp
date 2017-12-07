/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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
#include "tools.h"
#include <QTest>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QImage>
#include "../disk.h"


struct TestSearchCoverImage_Case
{
    QStringList imgFiles;
    QString     expected;

    void clear()
    {
        imgFiles.clear();
        expected.clear();
    }
};
Q_DECLARE_METATYPE(TestSearchCoverImage_Case)



/************************************************
 *
 ************************************************/
void TestFlacon::testSearchCoverImage()
{
    QFETCH(TestSearchCoverImage_Case, test);

    foreach (QString f, test.imgFiles)
    {
        QFileInfo( dir() + "/" + f).dir().mkpath(".");
        QImage img(QSize(1,1), QImage::Format_RGB32);
        img.save(dir() + "/" + f);
    }

    QString result   = Disk::searchCoverImage(dir());
    QString expected;
    if (!test.expected.isEmpty())
        expected = QFileInfo( dir() + "/" + test.expected).absoluteFilePath();

    QCOMPARE(result, expected);
}



/************************************************
 *
 ************************************************/
void TestFlacon::testSearchCoverImage_data()
{
    QTest::addColumn<TestSearchCoverImage_Case>("test");
    TestSearchCoverImage_Case req;

    // .....................................
    req.clear();
    req.expected = "";

    QTest::newRow("01") << req;



    // .....................................
    req.clear();
    req.imgFiles
                << "cover.jpg";
    req.expected = "cover.jpg";

    QTest::newRow("02") << req;


    // .....................................
    req.clear();
    req.imgFiles
                << "Cover.jpg";
    req.expected = "Cover.jpg";

    QTest::newRow("03") << req;


    // .....................................
    req.clear();
    req.imgFiles
                << "Covers/Cover.jpg";
    req.expected = "Covers/Cover.jpg";

    QTest::newRow("03") << req;



    // .....................................
    req.clear();
    req.imgFiles
                << "AAA.jpg"
                << "Cover.jpg";
    req.expected = "Cover.jpg";

    QTest::newRow("04") << req;



    // .....................................
    req.clear();
    req.imgFiles
                << "CoVeR.Jpg"
                << "Folder.png";
    req.expected = "CoVeR.Jpg";

    QTest::newRow("04") << req;



    // .....................................
    req.clear();
    req.imgFiles
        << "Covers/folder.jpg"
        << "Covers/Back.jpg"
        << "Covers/Front_inside.jpg"
        << "Covers/Matrix.jpg"
        << "Covers/CD.jpg"
        << "Covers/Front.jpg"
        << "Front.JPG";
    req.expected = "Front.JPG";

    QTest::newRow("05") << req;

}

