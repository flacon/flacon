/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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


#ifndef FLAC_H
#define FLAC_H

#include "outformat.h"
#include "format.h"
#include "configdialog.h"
#include "ui_flac_config.h"

class OutFormat_Flac: public OutFormat
{
public:
    OutFormat_Flac();
    bool check(QStringList *errors) const;

    virtual QString encoderProgramName() const { return "flac"; }
    virtual QString gainProgramName() const { return "metaflac"; }

    virtual QStringList encoderArgs(Track *track, const QString &outFile) const;
    virtual QStringList gainArgs(const QStringList &files) const;

    QHash<QString, QVariant> defaultParameters() const;
    EncoderConfigPage *configPage(QWidget *parent = 0) const;
};

class Format_Flac: public AudioFormat
{
public:
    virtual QString name() const { return "FLAC"; }
    virtual QString ext() const { return "flac"; }
    virtual bool isInputFormat() const { return true; }

    virtual QString decoderProgramName() const { return "flac"; }
    virtual QStringList decoderArgs(const QString &fileName) const;

    virtual QByteArray magic() const { return "fLaC"; }
    virtual uint const magicOffset() const { return 0; }
};


class ConfigPage_Flac: public EncoderConfigPage, private Ui::ConfigPage_Flac
{
    Q_OBJECT
public:
    explicit ConfigPage_Flac(QWidget *parent = 0);

    virtual void load();
    virtual void write();

};


#endif // FLAC_H
