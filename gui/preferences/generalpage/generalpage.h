/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
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

#ifndef GENERALPAGE_H
#define GENERALPAGE_H

#include <QWidget>
#include "types.h"

namespace Ui {
class GeneralPage;
}

class GeneralPage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralPage(QWidget *parent = nullptr);
    ~GeneralPage();

    QString tmpDir() const;
    void    setTmpDir(const QString &value);

    uint encoderThreadsCount() const;
    void setEncoderThreadsCount(uint value);

    bool isSplitTrackTitle() const;
    void setSplitTrackTitle(bool value);

    ProxyType proxyType() const;
    void      setProxyType(ProxyType value);

    QString proxyHost() const;
    void    setProxyHost(const QString &value);

    uint proxyPort() const;
    void setProxyPort(uint value);

    QString proxyUserName() const;
    void    setProxyUserName(const QString &value);

    QString proxyPassword() const;
    void    setProxyPassword(const QString &value);

private:
    Ui::GeneralPage *ui;

    void showTmpDirDialog();
    void initProxyTypeComboBox();
};

#endif // GENERALPAGE_H
