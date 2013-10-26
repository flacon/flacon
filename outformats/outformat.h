/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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


#ifndef OUTFORMAT_H
#define OUTFORMAT_H

#include <QStringList>
#include <QHash>
#include "disk.h"

class Encoder;
class Gain;
class EncoderConfigPage;

class OutFormat
{
public:
    static QList<OutFormat*> allFormats();
    static OutFormat *currentFormat();

    enum GainType {
        GainDisable,
        GainTrack,
        GainAlbum
    };

    enum PreGapType {
        PreGapExtractToFile,
        PreGapAddToFirstTrack
    };

    QString id() const { return mId; }
    QString name() const { return mName; }
    QString ext() const {return mExt; }
    GainType gainType() const;
    PreGapType preGapType() const;
    bool createCue() const;

    virtual QString encoderProgramName() const = 0;
    virtual QStringList encoderArgs(Track *track, const QString &outFile) const = 0;


    virtual QString gainProgramName() const = 0;
    virtual QStringList gainArgs(const QStringList &files) const = 0;


    virtual bool check(QStringList *errors) const;

    virtual QHash<QString, QVariant> defaultParameters() const = 0;
    virtual EncoderConfigPage *configPage(QWidget *parent = 0) const = 0;
    virtual bool hasConfigPage() const { return true; }

    static QString gainTypeToString(GainType type);
    static GainType strToGainType(const QString &str);

    static QString preGapTypeToString(PreGapType type);
    static PreGapType strToPreGapType(const QString &str);

    virtual Encoder *createEncoder(Track *track, QObject *parent = 0) const;
    virtual Gain *createGain(Disk *disk, Track *track, QObject *parent = 0) const;

protected:
    QString mId;
    QString mName;
    QString mExt;

    bool checkProgram(const QString &program, QStringList *errors) const;
};





#endif // OUTFORMAT_H
