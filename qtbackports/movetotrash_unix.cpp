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

//#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))

////static
// bool QFileSystemEngine_moveFileToTrash(const QFileSystemEntry &source,
//                                         QFileSystemEntry &newLocation, QSystemError &error)
//{
//#ifdef QT_BOOTSTRAPPED
//     Q_UNUSED(source);
//     Q_UNUSED(newLocation);
//     error = QSystemError(ENOSYS, QSystemError::StandardLibraryError);
//     return false;
//#else
//     const QFileInfo sourceInfo(source.filePath());
//     if (!sourceInfo.exists()) {
//         error = QSystemError(ENOENT, QSystemError::StandardLibraryError);
//         return false;
//     }
//     const QString sourcePath = sourceInfo.absoluteFilePath();

//    QDir trashDir(freeDesktopTrashLocation(sourcePath));
//    if (!trashDir.exists())
//        return false;
//    /*
//        "A trash directory contains two subdirectories, named info and files."
//    */
//    const auto filesDir = "files"_L1;
//    const auto infoDir = "info"_L1;
//    trashDir.mkdir(filesDir);
//    int savedErrno = errno;
//    trashDir.mkdir(infoDir);
//    if (!savedErrno)
//        savedErrno = errno;
//    if (!trashDir.exists(filesDir) || !trashDir.exists(infoDir)) {
//        error = QSystemError(savedErrno, QSystemError::StandardLibraryError);
//        return false;
//    }
//    /*
//        "The $trash/files directory contains the files and directories that were trashed.
//         The names of files in this directory are to be determined by the implementation;
//         the only limitation is that they must be unique within the directory. Even if a
//         file with the same name and location gets trashed many times, each subsequent
//         trashing must not overwrite a previous copy."
//    */
//    const QString trashedName = sourceInfo.isDir()
//                              ? QDir(sourcePath).dirName()
//                              : sourceInfo.fileName();
//    QString uniqueTrashedName = u'/' + trashedName;
//    QString infoFileName;
//    int counter = 0;
//    QFile infoFile;
//    auto makeUniqueTrashedName = [trashedName, &counter]() -> QString {
//        return QString::asprintf("/%ls-%04d", qUtf16Printable(trashedName), ++counter);
//    };
//    do {
//        while (QFile::exists(trashDir.filePath(filesDir) + uniqueTrashedName))
//            uniqueTrashedName = makeUniqueTrashedName();
//        /*
//            "The $trash/info directory contains an "information file" for every file and directory
//             in $trash/files. This file MUST have exactly the same name as the file or directory in
//             $trash/files, plus the extension ".trashinfo"
//             [...]
//             When trashing a file or directory, the implementation MUST create the corresponding
//             file in $trash/info first. Moreover, it MUST try to do this in an atomic fashion,
//             so that if two processes try to trash files with the same filename this will result
//             in two different trash files. On Unix-like systems this is done by generating a
//             filename, and then opening with O_EXCL. If that succeeds the creation was atomic
//             (at least on the same machine), if it fails you need to pick another filename."
//        */
//        infoFileName = trashDir.filePath(infoDir)
//                     + uniqueTrashedName + ".trashinfo"_L1;
//        infoFile.setFileName(infoFileName);
//        if (!infoFile.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Text))
//            uniqueTrashedName = makeUniqueTrashedName();
//    } while (!infoFile.isOpen());

//    const QString targetPath = trashDir.filePath(filesDir) + uniqueTrashedName;
//    const QFileSystemEntry target(targetPath);

//    QString infoPath;
//    const QStorageInfo storageInfo(sourcePath);
//    if (storageInfo.isValid() && storageInfo.rootPath() != rootPath() && storageInfo != QStorageInfo(QDir::home())) {
//        infoPath = sourcePath.mid(storageInfo.rootPath().length());
//        if (infoPath.front() == u'/')
//            infoPath = infoPath.mid(1);
//    } else {
//        infoPath = sourcePath;
//    }

//    /*
//        We might fail to rename if source and target are on different file systems.
//        In that case, we don't try further, i.e. copying and removing the original
//        is usually not what the user would expect to happen.
//    */
//    if (!renameFile(source, target, error)) {
//        infoFile.close();
//        infoFile.remove();
//        return false;
//    }

//    QByteArray info =
//            "[Trash Info]\n"
//            "Path=" + QUrl::toPercentEncoding(infoPath, "/") + "\n"
//            "DeletionDate=" + QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"_L1).toUtf8()
//            + "\n";
//    infoFile.write(info);
//    infoFile.close();

//    newLocation = QFileSystemEntry(targetPath);
//    return true;
//#endif // QT_BOOTSTRAPPED
//}
//#endif // Q_OS_DARWIN
//#en
