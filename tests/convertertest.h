#ifndef CONVERTERTEST_H
#define CONVERTERTEST_H
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

#include <QString>
#include <QSettings>
#include <QJsonDocument>

/************************************************

 ************************************************/
class ConverterTest
{
public:
    ConverterTest(const QString &dataDir, const QString &dir, const QString &tmpDir);
    virtual ~ConverterTest();

    bool run();
    void check();

    bool checkReplayGain();

    QString dataDir() const { return mDataDir; }
    QString dir() const { return mDir; }

private:
    const QString mDataDir;
    const QString mDir;
    const QString mTmpDir;

    const QString mCfgFile;
    const QString mInDir;
    const QString mOutDir;

    void srcAudioExec(QSettings &spec) const;

    QStringList readFile(const QString &fileName);
    void        writeFile(const QStringList &strings, const QString &fileName);

    void        createStartSh(const QString fileName, const QString flaconBin, const QStringList &args) const;
    QStringList findFiles(const QString &dir, const QString &pattern) const;
    void        printError(const QString &file, const QString &tag, const QVariant &actual, const QVariant &expected) const;
    void        printFile(const QString &fileName, bool printHeader = true);

    QByteArray trimmCueSheet(const QByteArray &cue) const;
};

/************************************************

 ************************************************/
class Mediainfo
{
public:
    Mediainfo(const QString &fileName);

    void save(const QString &fileName);

    QVariant value(const QString &key);

private:
    QString       mFileName;
    QJsonDocument mJsonDoc;
    QByteArray    mData;
};

#endif // CONVERTERTEST_H
