// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Ported from https://code.qt.io/cgit/qt/qtbase.git/ at 2023.02.22

#include "movetotrash.h"
#ifdef Q_OS_MACOS // desktop macOS has a trash can

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))

#include <QFileInfo>
#include <QUrl>
#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSFileManager.h>

/*
    This implementation does not enable the "put back" option in Finder
    for the trashed object. The only way to get this is to use Finder automation,
    which would query the user for permission to access Finder using a modal,
    blocking dialog - which we definitely can't have in a console application.

    Using Finder would also play the trash sound, which we don't want either in
    such a core API; applications that want that can play the sound themselves.
*/
bool moveFileToTrash(const QString &fileName, QString *pathInTrash)
{
    QMacAutoReleasePool pool;

    QFileInfo      info(fileName);
    NSString      *filepath     = info.filePath().toNSString();
    NSURL         *fileurl      = [NSURL fileURLWithPath:filepath isDirectory:info.isDir()];
    NSURL         *resultingUrl = nil;
    NSError       *nserror      = nil;
    NSFileManager *fm           = [NSFileManager defaultManager];
    if ([fm trashItemAtURL:fileurl resultingItemURL:&resultingUrl error:&nserror] != YES) {
        // error = QSystemError(nserror.code, QSystemError::NativeError);
        return false;
    }
    if (pathInTrash) {
        *pathInTrash = QUrl::fromNSURL(resultingUrl).path();
    }
    return true;
}

#endif // QT_VERSION < 5.15.0
#endif // Q_OS_MACOS
