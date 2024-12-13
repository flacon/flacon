/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2024
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

#ifndef MOVIE_H
#define MOVIE_H

#include <QObject>
#include "icon.h"
#include <QPixmap>
#include <QTimer>

class Movie : public QObject
{
    Q_OBJECT
public:
    explicit Movie(QSize size, QObject *parent = nullptr);

    void loadFrame(const QString &fileName);

    QPixmap currentPixmap() const;

    int currentFrame() const { return mCurrentFrame; }

    bool isRunning() const { return mTimer.isActive(); }
    void setRunning(bool value);

    void start();
    void stop();

signals:
    void updated();

private:
    QSize          mSize;
    QTimer         mTimer;
    int            mCurrentFrame = 0;
    QList<QPixmap> mFrames;

    void nextFrame();
};

#endif // MOVIE_H
