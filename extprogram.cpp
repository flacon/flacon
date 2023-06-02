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

#include "extprogram.h"
#include <QDir>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>
#include "types.h"
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "ExtProgram")
}

#ifdef Q_OS_WIN
static constexpr auto PATH_ENV_SEPARATOR = ';';
static constexpr auto BINARY_EXT         = ".exe";

#elif defined(Q_OS_OS2)
static constexpr auto PATH_ENV_SEPARATOR = ';';
static constexpr auto BINARY_EXT         = ".exe";

#else
static constexpr auto PATH_ENV_SEPARATOR = ':';
static constexpr auto BINARY_EXT         = "";

#endif

ExtProgram::ExtProgram(const QString &name) :
    mName(name)
{
#ifdef BUNDLED_PROGRAMS
    mPath = QDir(qApp->applicationDirPath()).absoluteFilePath(name);
#endif
}

void ExtProgram::setPath(const QString &path)
{
    Q_UNUSED(path);
#ifndef BUNDLED_PROGRAMS
    mPath = path;
#endif
}

QString ExtProgram::find() const
{
#ifdef BUNDLED_PROGRAMS
    return mPath;
#else
    QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(PATH_ENV_SEPARATOR);

    foreach (QString path, paths) {
        QFileInfo fi(path + QDir::separator() + mName + BINARY_EXT);
        if (fi.exists() && fi.isExecutable()) {
            return fi.absoluteFilePath();
        }
    }
    return "";
#endif
}

bool ExtProgram::check(QStringList *errors) const
{
    if (mPath.isEmpty()) {
        if (errors) {
            *errors << QCoreApplication::translate("ExtProgram",
                                                   "The %1 program is not installed.<br>Verify that all required programs are installed and in your preferences.",
                                                   "Error message. %1 - is an program name")
                               .arg(mName);
        }
        return false;
    }

    QFileInfo fi(mPath);
    if (!fi.exists()) {
        if (errors) {
            *errors << QCoreApplication::translate("ExtProgram",
                                                   "The %1 program is installed according to your settings, but the binary file can’t be found.<br>"
                                                   "Verify that all required programs are installed and in your preferences.",
                                                   "Error message. %1 - is an program name")
                               .arg(mName);
        }
        return false;
    }

    if (!fi.isExecutable()) {
        if (errors) {
            *errors << QCoreApplication::translate("ExtProgram",
                                                   "The %1 program is installed according to your settings, but the file is not executable.<br>"
                                                   "Verify that all required programs are installed and in your preferences.",
                                                   "Error message. %1 - is an program name")
                               .arg(mName);
        }
        return false;
    }

    return true;
}

QList<ExtProgram *> ExtProgram::allPrograms()
{
    QList<ExtProgram *> res;
    res << alacenc();
    res << faac();
    res << flac();
    res << lame();
    res << mac();
    res << oggenc();
    res << opusenc();
    res << sox();
    res << ttaenc();
    res << wavpack();
    res << wvunpack();
    return res;
}

QProcess *ExtProgram::open(QObject *parent) const
{
    QProcess *proc = new QProcess(parent);
    proc->setProgram(path());
    // QDir::toNativeSeparators(prog);

    proc->connect(proc, &QProcess::errorOccurred, proc, [proc](QProcess::ProcessError error) {
        qCWarning(LOG) << "ERROR";
        qCWarning(LOG) << QString("%1: The '%2' program crashes").arg(proc->objectName()).arg(proc->program());
        qCWarning(LOG) << "Program with args:" << debugProgramArgs(proc->program(), proc->arguments());
        qCWarning(LOG) << "Error:" << error;
        qCWarning(LOG) << "Error string:" << proc->errorString();
        if (proc->isOpen()) {
            qCWarning(LOG) << "Stderr: ............";
            qCWarning(LOG) << proc->readAllStandardError();
            qCWarning(LOG) << "....................";
        }

        QString msg = QString("The '%1' program crashes with an error: %2").arg(proc->program()).arg(proc->errorString());
        throw FlaconError(msg);
    });

    return proc;
}

QProcess *ExtProgram::open(const QStringList &args, QObject *parent) const
{
    QProcess *res = open(parent);
    res->setArguments(args);
    return res;
}

// clang-format off
ExtProgram *ExtProgram::alacenc() { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::faac()    { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::flac()    { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::lame()    { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::mac()     { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::oggenc()  { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::opusenc() { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::sox()     { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::ttaenc()  { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::wavpack() { static ExtProgram res(__FUNCTION__); return &res; }
ExtProgram *ExtProgram::wvunpack(){ static ExtProgram res(__FUNCTION__); return &res; }
// clang-format on
