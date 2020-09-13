/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2018
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


#include "application.h"
#include <QFileOpenEvent>
#include <QDebug>
#include <QStyle>


Application::Application(int &argc, char **argv):
    QApplication(argc, argv)
{
    init();
    mDarkMode = isDarkVisualMode();
}

Application::~Application()
{
    free();
}

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}


bool Application::event(QEvent *event)
{

    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *e = static_cast<QFileOpenEvent*>(event);
        emit openFile(e->file());
    }

#ifndef Q_OS_MAC
    if (event->type() == QEvent::ApplicationPaletteChange) {
        bool dark = isDarkVisualMode();
        if (dark != mDarkMode) {
            mDarkMode = isDarkVisualMode();
            emit visualModeChanged();
        }
    }
#endif
    return QApplication::event(event);
}

#ifndef Q_OS_MAC
bool Application::isDarkVisualMode() const
{
    return qApp->style()->standardPalette().window().color().lightness() <
           qApp->style()->standardPalette().windowText().color().lightness();
}
#endif
