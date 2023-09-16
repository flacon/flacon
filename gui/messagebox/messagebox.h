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
#include <QTextBrowser>

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
    ~MessageBox();

    QString text() const;
    void    setText(const QString &text);

    QString description() const;
    void    setDescription(const QString &value);

    QString css() const;
    void    setCss(const QString &css);

    QLabel       *textLabel() const;
    QTextBrowser *descriptionBrowser() const;

    StandardButtons standardButtons() const;
    void            setStandardButtons(StandardButtons buttons);

    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent *event) override;

    void setIcon(const QPixmap &icon);

private:
    Ui::MessageBox *ui;
};

class WarningBox : public MessageBox
{
public:
    explicit WarningBox(QWidget *parent = nullptr);
};

class CriticalBox : public MessageBox
{
public:
    explicit CriticalBox(QWidget *parent = nullptr);
};

class QuestionBox : public MessageBox
{
public:
    explicit QuestionBox(QWidget *parent = nullptr);
};

#endif // MESSAGEBOX_H
