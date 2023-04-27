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

#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QList>
#include <QIcon>
#include "disc.h"
#include "validator.h"

class Settings;
class Disc;
class Track;
class DataProvider;

class Project : public QObject
{
    Q_OBJECT
public:
    static Project *instance();

    QList<Disc *> disks() const { return mDiscs; }

    Disc *disc(int index) const;
    int   count() const;
    int   indexOf(const Disc *disc) const;

    void addDisc(Disc *disc) { insertDisc(disc); }
    int  insertDisc(Disc *disc, int index = -1);
    void removeDisc(const QList<Disc *> &discs);

    void emitDiscChanged(Disc *disc);
    void emitLayoutChanged() const;

    bool discExists(const QString &cueUri);

    void  clear();
    Disc *addAudioFile(const QString &fileName) noexcept(false);
    Disc *addCueFile(const QString &fileName);

    Profile *profile() { return mProfile; }
    bool     selectProfile(const QString &profileId);

    Profiles profiles() const { return mProfiles; }
    void     setProfiles(Profiles profiles);

    Validator &validator() { return mValidator; }

    void load(Settings *settings);
    void save(Settings *settings);

signals:
    void discChanged(Disc *disc) const;
    void layoutChanged() const;
    void beforeRemoveDisc(Disc *disc);
    void afterRemoveDisc();

protected:
    explicit Project(QObject *parent = nullptr);

private:
    QList<Disc *> mDiscs;
    Validator     mValidator;
    Profile      *mProfile = nullptr;
    Profiles      mProfiles;
};

#define project Project::instance()

#endif // PROJECT_H
