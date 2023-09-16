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
#include <QScrollBar>
#include "icon.h"
#include <QDebug>

MessageBox::MessageBox(QWidget *parent) :
    QDialog(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    ui->textBrowser->viewport()->setAutoFillBackground(false);
    ui->textBrowser->setLineWrapMode(QTextEdit::NoWrap);

    setWindowTitle(QObject::tr("Flacon", "Error"));
    setWindowModality(Qt::WindowModal);
}

MessageBox::~MessageBox()
{
    delete ui;
}

QString MessageBox::text() const
{
    return ui->textLabel->text();
}

void MessageBox::setText(const QString &text)
{
    ui->textLabel->setText(text);
}

QString MessageBox::description() const
{
    return ui->textBrowser->document()->toHtml();
}

void MessageBox::setDescription(const QString &value)
{
    ui->textBrowser->setHtml(value);
}

QString MessageBox::css() const
{
    return ui->textBrowser->document()->defaultStyleSheet();
}

void MessageBox::setCss(const QString &css)
{
    ui->textBrowser->document()->setDefaultStyleSheet(css);
}

QLabel *MessageBox::textLabel() const
{
    return ui->textLabel;
}

QTextBrowser *MessageBox::descriptionBrowser() const
{
    return ui->textBrowser;
}

MessageBox::StandardButtons MessageBox::standardButtons() const
{
    return StandardButtons(int(ui->buttonBox->standardButtons()));
}

void MessageBox::setStandardButtons(StandardButtons buttons)
{
    ui->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
}

void MessageBox::showEvent(QShowEvent *event)
{
    ui->iconLabel->setVisible(ui->iconLabel->pixmap());

    resize(sizeHint().width(), size().height());
    resize(sizeHint());
    QDialog::showEvent(event);
}

void MessageBox::setIcon(const QPixmap &icon)
{
    ui->iconLabel->setPixmap(icon);
}

QSize MessageBox::sizeHint() const
{
    int width  = ui->textBrowser->document()->idealWidth() + (this->width() - ui->textBrowser->viewport()->width()); // + ui->textBrowser->verticalScrollBar()->width() + 0;
    int height = std::min(ui->textBrowser->document()->size().height() + (this->height() - ui->textBrowser->viewport()->height()), 400.0);
    return QSize(width, height);
}

QuestionBox::QuestionBox(QWidget *parent) :
    MessageBox(parent)
{
    setStandardButtons(StandardButton::Yes | StandardButton::No);
}

WarningBox::WarningBox(QWidget *parent) :
    MessageBox(parent)
{
    setStandardButtons(StandardButton::Ok);
    setIcon(Pixmap("warning", 64, 64));
}

CriticalBox::CriticalBox(QWidget *parent) :
    MessageBox(parent)
{
    setStandardButtons(StandardButton::Ok);
    setIcon(Pixmap("error", 64, 64));
}
