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

#include "messagebox.h"
#include "ui_messagebox.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include "icon.h"
#include <QDebug>

MessageBox::MessageBox(QWidget *parent) :
    MessageBox(MessageBox::Icon::NoIcon, "", "", StandardButton::NoButton, parent)
{
}

MessageBox::MessageBox(MessageBox::Icon icon, const QString &title, const QString &text, MessageBox::StandardButtons buttons, QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::MessageBox),
    mIcon(icon)
{
    ui->setupUi(this);
    ui->textBrowser->viewport()->setAutoFillBackground(false);

    setWindowTitle(title);
    setText(text);

    ui->textBrowser->setLineWrapMode(QTextEdit::NoWrap);
    ui->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
}

MessageBox::~MessageBox()
{
    delete ui;
}

MessageBox::StandardButton MessageBox::doOpen(QWidget *parent, MessageBox::Icon icon, const QString &title, const QString &text, MessageBox::StandardButtons buttons, MessageBox::StandardButton defaultButton)
{
    MessageBox box(icon, title, text, buttons, parent);

    QPushButton *btn = box.ui->buttonBox->button(QDialogButtonBox::StandardButton(defaultButton));
    if (btn) {
        btn->setDefault(true);
    }

    box.setWindowModality(Qt::WindowModal);
    return StandardButton(box.exec());
}

MessageBox::StandardButton MessageBox::warning(QWidget *parent, const QString &title, const QString &text, MessageBox::StandardButtons buttons, MessageBox::StandardButton defaultButton)
{
    return doOpen(parent, Icon::Warning, title, text, buttons, defaultButton);
}

MessageBox::StandardButton MessageBox::critical(QWidget *parent, const QString &title, const QString &text, MessageBox::StandardButtons buttons, MessageBox::StandardButton defaultButton)
{
    return doOpen(parent, Icon::Critical, title, text, buttons, defaultButton);
}

QString MessageBox::text() const
{
    return ui->textBrowser->document()->toHtml();
}

void MessageBox::setText(const QString &text)
{
    ui->textBrowser->setHtml(text);
}

QString MessageBox::css() const
{
    return ui->textBrowser->document()->defaultStyleSheet();
}

void MessageBox::setCss(const QString &css)
{
    ui->textBrowser->document()->setDefaultStyleSheet(css);
}

void MessageBox::setIcon(MessageBox::Icon icon)
{
    mIcon = icon;
}

void MessageBox::showEvent(QShowEvent *event)
{

    switch (mIcon) {
        case Icon::NoIcon:
            ui->iconLabel->clear();
            break;

        case Icon::Information:
            ui->iconLabel->clear();
            break;

        case Icon::Warning:
            ui->iconLabel->setPixmap(Pixmap("warning", 64, 64));
            break;

        case Icon::Critical:
            ui->iconLabel->setPixmap(Pixmap("error", 64, 64));
            break;

        case Icon::Question:
            ui->iconLabel->clear();
            break;
    }

    int width = ui->textBrowser->document()->idealWidth() + ui->textBrowser->contentsMargins().left() + ui->textBrowser->contentsMargins().right();
    width     = std::min(width, 800);
    ui->textBrowser->setMinimumWidth(width);
    adjustSize();
    QDialog::showEvent(event);
}
