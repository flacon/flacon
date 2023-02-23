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

#ifndef SYNC_H
#define SYNC_H

#include <type_traits>
#include <QThread>

/************************************************
 *
 * **********************************************/
template <class Receiver, typename Lambda>
static void invoke(Receiver *receiver, Lambda &&lambda)
{
    static_assert(std::is_base_of<QObject, Receiver>::value, "No Q_OBJECT in the class with the slot");
    Q_STATIC_ASSERT_X(QtPrivate::HasQ_OBJECT_Macro<Receiver>::Value, "No Q_OBJECT in the class with the slot");

    if (receiver->thread() == QThread::currentThread()) {
        return lambda();
    }

    // We set receiver as context (third parameter), so lambda will be placed in a event loop of receiver.
    // See https://doc.qt.io/qt-5/qobject.html#connect-5
    QObject runner;
    runner.connect(&runner, &QObject::destroyed, receiver, std::forward<Lambda>(lambda), Qt::BlockingQueuedConnection);
}

/************************************************
 *
 * **********************************************/
template <class Receiver, typename R, typename... Args>
R invoke(Receiver *receiver, R (Receiver::*slot)(Args...) const, Args... args)
{
    if constexpr (std::is_void_v<R>) {
        invoke(receiver, [receiver, slot, args...]() { std::mem_fn(slot)(receiver, args...); });
    }
    else {
        R res;
        invoke(receiver, [receiver, slot, &res, args...]() { res = std::mem_fn(slot)(receiver, args...); });
        return res;
    }
}

/************************************************
 *
 * **********************************************/
template <class Receiver, typename R, typename... Args>
R invoke(Receiver *receiver, R (Receiver::*slot)(Args...), Args... args)
{
    if constexpr (std::is_void_v<R>) {
        invoke(receiver, [receiver, slot, args...]() { std::mem_fn(slot)(receiver, args...); });
    }
    else {
        R res;
        invoke(receiver, [receiver, slot, &res, args...]() { res = std::mem_fn(slot)(receiver, args...); });
        return res;
    }
}

#endif // SYNC_H
