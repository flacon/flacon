/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2019
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

#include "icon.h"
#include <QIconEngine>
#include <QApplication>
#include <QDebug>

class IconEngine : public QIconEngine
{
public:
    /************************************************
     *
     ************************************************/
    IconEngine() :
        QIconEngine()
    {
    }

    /************************************************
     *
     ************************************************/
    IconEngine(const IconEngine &other) :
        QIconEngine(other),
        mIconDark(other.mIconDark),
        mIconLight(other.mIconLight)
    {
    }

    /************************************************
     *
     ************************************************/
    QIconEngine *clone() const override
    {
        return new IconEngine(*this);
    }

    /************************************************
     *
     ************************************************/
    void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        mIconLight.addFile(fileName, size, mode, state);
        QString d = fileName;
        d.replace("/light/", "/dark/");
        mIconDark.addFile(d, size, mode, state);
    }

    /************************************************
     *
     ************************************************/
    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override
    {
        mIconLight.addPixmap(pixmap, mode, state);
    }

    /************************************************
     *
     ************************************************/
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
    {
        icon().paint(painter, rect, Qt::AlignCenter, mode, state);
    }

    /************************************************
     *
     ************************************************/
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        return icon().pixmap(size, mode, state);
    }

    static bool mDarkMode;

private:
    QIcon &icon()
    {
        if (mDarkMode)
            return mIconDark;
        else
            return mIconLight;
    }

    QIcon mIconDark;
    QIcon mIconLight;
};

bool IconEngine::mDarkMode = false;

Icon::Icon() :
    QIcon(new IconEngine())
{
}

Icon::Icon(const QString &fileName) :
    QIcon(new IconEngine())
{
    const auto sizes = { 16, 22, 24, 32, 48, 64, 128, 256, 512 };
    for (auto size : sizes) {
        addFile(QStringLiteral(":icons/light/%1/%2.png")
                        .arg(size, 3, 10, QChar('0'))
                        .arg(fileName),
                QSize(size, size), QIcon::Normal);

        addFile(QStringLiteral(":icons/light/%1/%2-disabled.png")
                        .arg(size, 3, 10, QChar('0'))
                        .arg(fileName),
                QSize(size, size), QIcon::Disabled);
    }
}

/************************************************
 *
 ************************************************/
Icon::Icon(const Icon &other) :
    QIcon(other)
{
}

/************************************************
 *
 ************************************************/
bool Icon::isDarkMode()
{
    return IconEngine::mDarkMode;
}

/************************************************
 *
 ************************************************/
void Icon::setDarkMode(bool dark)
{
    IconEngine::mDarkMode = dark;
}

Pixmap::Pixmap(const QString &name, QSize size) :
    QPixmap(Icon(name).pixmap(size * qApp->devicePixelRatio()))
{
    setDevicePixelRatio(qApp->devicePixelRatio());
}

Pixmap::Pixmap(const QString &name, int width, int height) :
    Pixmap(name, QSize(width, height))
{
}
