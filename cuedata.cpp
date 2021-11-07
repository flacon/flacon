/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2020
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

#include "cuedata.h"

#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include "types.h"

/************************************************
 *
 ************************************************/
static QByteArray unQuote(const QByteArray &line)
{
    if (line.length() > 2 && (line.at(0) == '"' || line.at(0) == '\'') && line.at(0) == line.at(line.length() - 1)) {
        return line.mid(1, line.length() - 2);
    }
    return line;
}

/************************************************

 ************************************************/
static QByteArray extractFileFromFileTag(const QByteArray &value)
{
    int n = value.lastIndexOf(' ');
    if (n > -1)
        return unQuote(value.left(n));

    return unQuote(value);
}

/************************************************
 *
 ************************************************/
CueData::CueData(QIODevice *device) noexcept(false)
{
    read(device);
}

/************************************************
 *
 ************************************************/
CueData::CueData(const QString &fileName) :
    mFileName(fileName)
{
    QFileInfo fi(fileName);
    if (!fi.exists()) {
        throw FlaconError(QObject::tr("File <b>\"%1\"</b> does not exist").arg(fileName));
    }

    QFile file(fi.canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        throw FlaconError(file.errorString());
    }

    read(&file);
    file.close();
}

/************************************************
 Complete CUE sheet syntax documentation
 https://github.com/flacon/flacon/blob/master/cuesheet_syntax.md
 ************************************************/
void CueData::read(QIODevice *file)
{
    mCodecName = detectCodepage(file);

    uint       lineNum = 0;
    QByteArray tag;
    QByteArray value;
    QByteArray audioFile;

    // Read global tags ..............................
    while (!file->atEnd()) {
        lineNum++;
        QByteArray line = file->readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        parseLine(line, tag, value, lineNum);

        if (tag.isEmpty()) {
            continue;
        }

        if (tag == TRACK_TAG) {
            break;
        }

        if (tag == FILE_TAG) {
            audioFile = extractFileFromFileTag(value);
            continue;
        }

        mGlobalTags.insert(tag, value);
    }

    while (!file->atEnd()) {
        bool ok;

        leftPart(value, ' ').toInt(&ok);
        if (!ok)
            throw FlaconError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track number on line %2.", "Cue parser error.")
                                      .arg(mFileName)
                                      .arg(lineNum));

        Tags track;
        track.insert(TRACK_TAG, leftPart(value, ' '));
        track.insert(FILE_TAG, audioFile);

        while (!file->atEnd()) {
            lineNum++;
            QByteArray line = file->readLine().trimmed();
            if (line.isEmpty())
                continue;

            parseLine(line, tag, value, lineNum);

            if (tag.isEmpty())
                continue;

            if (tag == TRACK_TAG)
                break;

            if (tag == FILE_TAG) {
                audioFile = extractFileFromFileTag(value);
                continue;
            }

            track.insert(tag, value);
        }
        mTracks << track;
    }
}

/************************************************
 * Detect codepage and skip BOM
 ************************************************/
QString CueData::detectCodepage(QIODevice *file)
{
    QByteArray magic = file->read(3);

    if (magic.startsWith("\xEF\xBB\xBF")) {
        file->seek(3);
        return "UTF-8";
    }

    if (magic.startsWith("\xFE\xFF")) {
        file->seek(2);
        return "UTF-16BE";
    }

    if (magic.startsWith("\xFF\xFE")) {
        file->seek(2);
        return "UTF-16LE";
    }

    file->seek(0);
    return "";
}

/************************************************
 *
 ************************************************/
void CueData::parseLine(const QByteArray &line, QByteArray &tag, QByteArray &value, uint lineNum) const
{
    QByteArray l = line.trimmed();

    if (l.isEmpty()) {
        tag   = "";
        value = "";
        return;
    }

    tag   = leftPart(l, ' ').toUpper();
    value = rightPart(l, ' ').trimmed();

    if (tag == "REM") {
        tag   = leftPart(value, ' ').toUpper();
        value = rightPart(value, ' ').trimmed();
    }

    value = unQuote(value);

    //=============================
    if (tag == INDEX_TAG) {
        bool ok;
        int  num = leftPart(value, ' ').toInt(&ok);
        if (!ok)
            throw FlaconError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track index on line %2.", "Cue parser error.")
                                      .arg(mFileName)
                                      .arg(lineNum));

        if (num < 0 || num > 99)
            throw FlaconError(QObject::tr("<b>%1</b> is not a valid CUE file. Incorrect track index on line %2.", "Cue parser error.")
                                      .arg(mFileName)
                                      .arg(lineNum));

        tag   = QString("%1 %2").arg(INDEX_TAG).arg(num, 2, 10, QChar('0')).toLatin1();
        value = rightPart(value, ' ').trimmed();
    }
}
