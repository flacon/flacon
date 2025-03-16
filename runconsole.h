/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2025
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

#ifndef RUNCONSOLE_H
#define RUNCONSOLE_H

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include "commandlineparser.h"
#include "track.h"
#include "profiles.h"

class Project;
class Disc;

class RunConsole : public QObject
{
    Q_OBJECT
public:
    int run(int argc, char *argv[]);

private:
    CommandLineParser mCommandLineParser;
    QDateTime         mStartTime;
    QDateTime         mFinishTime;
    Profile           mProfile;

    void doRun();
    void finished(bool success);
    void printStatistic();
    bool validate() const;

    void addFile(const QString &file);
    void addFile(const QString &file, bool showError);

    void printProgress(const Track &track, TrackState state, Percent);

    QString diskDescription(const Disc *disk) const;
};

#endif // RUNCONSOLE_H
