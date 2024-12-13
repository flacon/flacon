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

#include "movie.h"
#include "icon.h"
#include <QDebug>

static constexpr int MOVIE_INTERVAL_MS = 100;

Movie::Movie(QSize size, QObject *parent) :
    QObject(parent),
    mSize(size)
{
    mTimer.setInterval(MOVIE_INTERVAL_MS);
    connect(&mTimer, &QTimer::timeout, this, &Movie::nextFrame);
}

void Movie::loadFrame(const QString &fileName)
{
    mFrames << Pixmap(fileName, mSize);
}

QPixmap Movie::currentPixmap() const
{
    return mFrames.at(mCurrentFrame);
}

void Movie::setRunning(bool value)
{
    if (value) {
        start();
    }
    else {
        stop();
    }
}

void Movie::start()
{
    if (mFrames.isEmpty()) {
        return;
    }

    if (isRunning()) {
        return;
    }

    mTimer.start();
    emit updated();
}

void Movie::stop()
{
    if (!isRunning()) {
        return;
    }

    mTimer.stop();
}

void Movie::nextFrame()
{
    if (isRunning()) {
        mCurrentFrame = (mCurrentFrame + 1) % mFrames.count();
        emit updated();
    }
}
