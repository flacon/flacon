#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2022
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

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class MessageBox;
}

class MessageBox : public QDialog
{
    Q_OBJECT

public:
    using Icon            = QMessageBox::Icon;
    using StandardButton  = QMessageBox::StandardButton;
    using StandardButtons = QMessageBox::StandardButtons;

    explicit MessageBox(QWidget *parent = nullptr);
    MessageBox(Icon icon, const QString &title, const QString &text,
               StandardButtons buttons = StandardButton::NoButton, QWidget *parent = nullptr,
               Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    ~MessageBox();

    QString text() const;
    void    setText(const QString &text);

    QString css() const;
    void    setCss(const QString &css);

    Icon icon() const { return mIcon; }
    void setIcon(Icon icon);

    static StandardButton warning(QWidget *parent, const QString &title, const QString &text,
                                  StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);

    static StandardButton critical(QWidget *parent, const QString &title, const QString &text,
                                   StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::MessageBox *ui;

    Icon mIcon = Icon::NoIcon;

    static StandardButton doOpen(QWidget *parent, Icon icon, const QString &title, const QString &text,
                                 StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);
};

#endif // MESSAGEBOX_H
