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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "ui_aboutdialog.h"

struct AboutInfoItem
{
    QString name;
    QString url;
    QString description;
};

class AboutInfo : public QList<AboutInfoItem>
{
public:
    AboutInfo();
    QString asString() const;
    void    add(const QString &name, const QString &url, const QString &description = nullptr);
};

class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString titleText() const;
    QString descriptionText() const;
    QString copyrightText() const;
    QString bugTrackerText() const;
    QString homepageText() const;
    QString licenseText() const;

    AboutInfo authorsInfo() const;
    AboutInfo thanksInfo() const;
    QString   translationsText() const;
    AboutInfo programsInfo() const;
};

#endif // ABOUTDIALOG_H
