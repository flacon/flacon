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


#ifndef MP3_H
#define MP3_H

#include "outformat.h"
#include "configdialog.h"
#include "ui_mp3_config.h"

class OutFormat_Mp3: public OutFormat
{
public:
    OutFormat_Mp3();

    virtual QString encoderProgramName() const { return "lame"; }
    virtual QString gainProgramName() const { return "mp3gain"; }

    virtual QStringList encoderArgs(const Track *track, const QString &outFile) const;
    virtual QStringList gainArgs(const QStringList &files) const;

    QHash<QString, QVariant> defaultParameters() const;
    EncoderConfigPage *configPage(QWidget *parent = 0) const;
};


class ConfigPage_Mp3: public EncoderConfigPage, private Ui::ConfigPage_Mp3
{
    Q_OBJECT
public:
    explicit ConfigPage_Mp3(QWidget *parent = 0);

    virtual void load();
    virtual void write();

private slots:
    void mp3PresetCbxCanged(int index);
};

#endif // MP3_H
