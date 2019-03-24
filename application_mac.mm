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


//bool Browser::IsDarkMode() {
//  NSString *mode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
//  return [mode isEqualToString: @"Dark"];
//}

#include "application.h"
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSDistributedNotificationCenter.h>
#include <Foundation/NSString.h>

#include <QDebug>

/************************************************

 ************************************************/
void Application::init()
{
    [
        [NSDistributedNotificationCenter defaultCenter]
        addObserverForName: @"AppleInterfaceThemeChangedNotification"
        object: nil
        queue: nil
        usingBlock: ^ (NSNotification *)
        {
            emit visualModeChanged();
        }
    ];
}


/************************************************

 ************************************************/
void Application::free()
{

}


/************************************************

 ************************************************/
bool Application::isDarkVisualMode() const
{
    NSString *mode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return [mode isEqualToString: @"Dark"];
}
