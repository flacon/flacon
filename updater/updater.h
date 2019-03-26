/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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


#ifndef UPDATER_H
#define UPDATER_H

#ifdef MAC_UPDATER

#include <QObject>
#include <QUrl>
#include <QDateTime>

class Updater: public QObject
{
    Q_OBJECT
public:
    static Updater &sharedUpdater();

    /*!
     The URL of the appcast used to download update information.

     Setting this property will persist in the host bundle's user defaults.
     If you don't want persistence, you may want to consider instead implementing
     SUUpdaterDelegate::feedURLStringForUpdater: or SUUpdaterDelegate::feedParametersForUpdater:sendingSystemProfile:

     This property must be called on the main thread.
     */
    QUrl feedURL() const;

    /*!
     A property indicating whether or not to check for updates automatically.

     Setting this property will persist in the host bundle's user defaults.
     The update schedule cycle will be reset in a short delay after the property's new value is set.
     This is to allow reverting this property without kicking off a schedule change immediately
     */
    bool automaticallyChecksForUpdates() const;

    /*!
        Returns the date of last update check.

        \returns \c Null date if no check has been performed.
     */
    QDateTime lastUpdateCheckDate() const;

public slots:
    /*!
    Explicitly checks for updates and displays a progress dialog while doing so.

    This method is meant for a main menu item.
    Connect any menu item to this slot, and Sparkle will check for updates
    and report back its findings verbosely when it is invoked.
    */
    void checkForUpdates(const QString &id = "");

    /*!
     Checks for updates, but does not display any UI unless an update is found.

     This is meant for programmatically initating a check for updates. That is,
     it will display no UI unless it actually finds an update, in which case it
     proceeds as usual.

     If automatic downloading of updates it turned on and allowed, however,
     this will invoke that behavior, and if an update is found, it will be downloaded
     in the background silently and will be prepped for installation.

     This will not find updates that the user has opted into skipping.
     */
    void checkForUpdatesInBackground();

    /*!
     The URL of the appcast used to download update information.

     Setting this property will persist in the host bundle's user defaults.
     If you don't want persistence, you may want to consider instead implementing
     SUUpdaterDelegate::feedURLStringForUpdater: or SUUpdaterDelegate::feedParametersForUpdater:sendingSystemProfile:

     This property must be called on the main thread.
     */
    void setFeedURL(const QUrl &url);
    void setFeedURL(const char *url);

    /*!
     A property indicating whether or not to check for updates automatically.

     Setting this property will persist in the host bundle's user defaults.
     The update schedule cycle will be reset in a short delay after the property's new value is set.
     This is to allow reverting this property without kicking off a schedule change immediately
     */
    void setAutomaticallyChecksForUpdates(bool enable);

protected:
    Updater();
    virtual ~Updater();

private:
        class Private;
        Private* d;
};

#endif
#endif // UPDATER_H
