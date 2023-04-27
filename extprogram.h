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

#ifndef EXTPROGRAM_H
#define EXTPROGRAM_H

#include <QString>
#include <QObject>

class QProcess;

class ExtProgram
{
public:
    QString name() const { return mName; }

    QString path() const { return mPath; }
    void    setPath(const QString &path);

    QString find() const;

    bool check(QStringList *errors) const;

    static QList<ExtProgram *> allPrograms();

    QProcess *open(QObject *parent = nullptr) const;
    QProcess *open(const QStringList &args, QObject *parent = nullptr) const;

private:
    QString mName;
    QString mPath;

protected:
    explicit ExtProgram(const QString &name);

public:
    static ExtProgram *alacenc();
    static ExtProgram *faac();
    static ExtProgram *flac();
    static ExtProgram *lame();
    static ExtProgram *mac();
    static ExtProgram *oggenc();
    static ExtProgram *opusenc();
    static ExtProgram *sox();
    static ExtProgram *ttaenc();
    static ExtProgram *wavpack();
    static ExtProgram *wvunpack();
};

using ExtPrograms = QList<ExtProgram *>;

#endif // EXTPROGRAM_H
