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

#include "movetotrash.h"

#ifndef Q_OS_MACOS
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))

#include <sys/stat.h>
#include <QDir>
#include <QStorageInfo>
#include <unistd.h>
#include <QStandardPaths>
#include <QUrl>
#include <QDateTime>
#include <QDebug>

static QString freeDesktopTrashLocation(const QString &sourcePath);
static QString rootPath();

bool moveFileToTrash(const QString &fileName, QString *pathInTrash)
{
    const QFileInfo sourceInfo(fileName);
    if (!sourceInfo.exists()) {
        // error = QSystemError(ENOENT, QSystemError::StandardLibraryError);
        return false;
    }
    const QString sourcePath = sourceInfo.absoluteFilePath();

    QDir trashDir(freeDesktopTrashLocation(sourcePath));
    if (!trashDir.exists())
        return false;
    /*
        "A trash directory contains two subdirectories, named info and files."
    */
    const auto filesDir = "files";
    const auto infoDir  = "info";
    trashDir.mkdir(filesDir);
    int savedErrno = errno;
    trashDir.mkdir(infoDir);
    if (!savedErrno)
        savedErrno = errno;
    if (!trashDir.exists(filesDir) || !trashDir.exists(infoDir)) {
        // error = QSystemError(savedErrno, QSystemError::StandardLibraryError);
        return false;
    }
    /*
        "The $trash/files directory contains the files and directories that were trashed.
         The names of files in this directory are to be determined by the implementation;
         the only limitation is that they must be unique within the directory. Even if a
         file with the same name and location gets trashed many times, each subsequent
         trashing must not overwrite a previous copy."
    */
    const QString trashedName       = sourceInfo.isDir()
                  ? QDir(sourcePath).dirName()
                  : sourceInfo.fileName();
    QString       uniqueTrashedName = u'/' + trashedName;
    QString       infoFileName;
    int           counter = 0;
    QFile         infoFile;
    auto          makeUniqueTrashedName = [trashedName, &counter]() -> QString {
        return QString::asprintf("/%ls-%04d", qUtf16Printable(trashedName), ++counter);
    };
    do {
        while (QFile::exists(trashDir.filePath(filesDir) + uniqueTrashedName))
            uniqueTrashedName = makeUniqueTrashedName();
        /*
            "The $trash/info directory contains an "information file" for every file and directory
             in $trash/files. This file MUST have exactly the same name as the file or directory in
             $trash/files, plus the extension ".trashinfo"
             [...]
             When trashing a file or directory, the implementation MUST create the corresponding
             file in $trash/info first. Moreover, it MUST try to do this in an atomic fashion,
             so that if two processes try to trash files with the same filename this will result
             in two different trash files. On Unix-like systems this is done by generating a
             filename, and then opening with O_EXCL. If that succeeds the creation was atomic
             (at least on the same machine), if it fails you need to pick another filename."
        */
        infoFileName = trashDir.filePath(infoDir)
                + uniqueTrashedName + ".trashinfo";
        infoFile.setFileName(infoFileName);
        if (!infoFile.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Text))
            uniqueTrashedName = makeUniqueTrashedName();
    } while (!infoFile.isOpen());

    const QString targetPath = trashDir.filePath(filesDir) + uniqueTrashedName;
    // const QFileSystemEntry target(targetPath);

    QString            infoPath;
    const QStorageInfo storageInfo(sourcePath);
    if (storageInfo.isValid() && storageInfo.rootPath() != rootPath() && storageInfo != QStorageInfo(QDir::home())) {
        infoPath = sourcePath.mid(storageInfo.rootPath().length());
        if (infoPath.front() == u'/')
            infoPath = infoPath.mid(1);
    }
    else {
        infoPath = sourcePath;
    }

    /*
        We might fail to rename if source and target are on different file systems.
        In that case, we don't try further, i.e. copying and removing the original
        is usually not what the user would expect to happen.
    */
    if (!QFile::rename(sourceInfo.filePath(), targetPath)) {
        infoFile.close();
        infoFile.remove();
        return false;
    }

    QByteArray info =
            "[Trash Info]\n"
            "Path="
            + QUrl::toPercentEncoding(infoPath, "/") + "\n"
                                                       "DeletionDate="
            + QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss").toUtf8()
            + "\n";
    infoFile.write(info);
    infoFile.close();

    if (pathInTrash) {
        *pathInTrash = targetPath;
    }
    return true;
}

static QString rootPath()
{
    return "/";
}

static QString freeDesktopTrashLocation(const QString &sourcePath)
{
    auto makeTrashDir = [](const QDir &topDir, const QString &trashDir) -> QString {
        auto ownerPerms = QFileDevice::ReadOwner
                | QFileDevice::WriteOwner
                | QFileDevice::ExeOwner;
        QString targetDir = topDir.filePath(trashDir);
        // deliberately not using mkpath, since we want to fail if topDir doesn't exist
        if (topDir.mkdir(trashDir))
            QFile::setPermissions(targetDir, ownerPerms);
        if (QFileInfo(targetDir).isDir())
            return targetDir;
        return QString();
    };
    auto isSticky = [](const QFileInfo &fileInfo) -> bool {
        struct stat st;
        if (stat(QFile::encodeName(fileInfo.absoluteFilePath()).constData(), &st) == 0)
            return st.st_mode & S_ISVTX;

        return false;
    };

    QString            trash;
    const QStorageInfo sourceStorage(sourcePath);
    const QStorageInfo homeStorage(QDir::home());
    // We support trashing of files outside the users home partition
    if (sourceStorage != homeStorage) {
        const QString dotTrash = ".Trash";
        QDir          topDir(sourceStorage.rootPath());
        /*
            Method 1:
            "An administrator can create an $topdir/.Trash directory. The permissions on this
            directories should permit all users who can trash files at all to write in it;
            and the “sticky bit” in the permissions must be set, if the file system supports
            it.
            When trashing a file from a non-home partition/device, an implementation
            (if it supports trashing in top directories) MUST check for the presence
            of $topdir/.Trash."
        */
        const QString userID = QString::number(::getuid());
        if (topDir.cd(dotTrash)) {
            const QFileInfo trashInfo(topDir.path());

            // we MUST check that the sticky bit is set, and that it is not a symlink
            if (trashInfo.isSymLink()) {
                // we SHOULD report the failed check to the administrator
                qCritical("Warning: '%s' is a symlink to '%s'",
                          trashInfo.absoluteFilePath().toLocal8Bit().constData(),
                          trashInfo.symLinkTarget().toLatin1().constData());
            }
            else if (!isSticky(trashInfo)) {
                // we SHOULD report the failed check to the administrator
                qCritical("Warning: '%s' doesn't have sticky bit set!",
                          trashInfo.absoluteFilePath().toLocal8Bit().constData());
            }
            else if (trashInfo.isDir()) {
                /*
                    "If the directory exists and passes the checks, a subdirectory of the
                     $topdir/.Trash directory is to be used as the user's trash directory
                     for this partition/device. The name of this subdirectory is the numeric
                     identifier of the current user ($topdir/.Trash/$uid).
                     When trashing a file, if this directory does not exist for the current user,
                     the implementation MUST immediately create it, without any warnings or
                     delays for the user."
                */
                trash = makeTrashDir(topDir, userID);
            }
        }
        /*
            Method 2:
            "If an $topdir/.Trash directory is absent, an $topdir/.Trash-$uid directory is to be
             used as the user's trash directory for this device/partition. [...] When trashing a
             file, if an $topdir/.Trash-$uid directory does not exist, the implementation MUST
             immediately create it, without any warnings or delays for the user."
        */
        if (trash.isEmpty()) {
            topDir                     = QDir(sourceStorage.rootPath());
            const QString userTrashDir = dotTrash + u'-' + userID;
            trash                      = makeTrashDir(topDir, userTrashDir);
        }
    }
    /*
        "If both (1) and (2) fail [...], the implementation MUST either trash the
         file into the user's “home trash” or refuse to trash it."

         We trash the file into the user's home trash.

        "Its name and location are $XDG_DATA_HOME/Trash"; $XDG_DATA_HOME is what
        QStandardPaths returns for GenericDataLocation. If that doesn't exist, then
        we are not running on a freedesktop.org-compliant environment, and give up.
    */
    if (trash.isEmpty()) {
        QDir topDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        trash       = makeTrashDir(topDir, "Trash");
        if (!QFileInfo(trash).isDir()) {
            qWarning("Unable to establish trash directory in %s",
                     topDir.path().toLocal8Bit().constData());
        }
    }

    return trash;
}

#endif // QT_VERSION < 5.15.0
#endif // not Q_OS_MACOS
