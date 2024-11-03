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

#include "controls.h"
#include "project.h"
#include "icon.h"
#include "patternexpander.h"
#include "extprogram.h"

#include <QtGlobal>
#include <QMenu>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QFileDialog>
#include <QApplication>
#include <QCompleter>
#include <QStringListModel>
#include <QCheckBox>
#include <QPaintEvent>
#include <QStandardPaths>
#include <QToolBar>
#include <QResizeEvent>
#include <QPoint>

static constexpr int HISTORY_COUNT = 10;

/************************************************
 *
 ************************************************/
ToolButton::ToolButton(const QIcon &icon, QWidget *parent) :
    QToolButton(parent)
{
    setAutoRaise(true);
    setStyleSheet("QToolButton{ border: none; }");
    setFixedWidth(ToolButton::sizeHint().width());
    setIcon(icon);
    setPopupMode(QToolButton::InstantPopup);
}

/************************************************
 *
 ************************************************/
ToolButton::ToolButton(QWidget *parent) :
    ToolButton(Icon("pattern-button"), parent)
{
}

/************************************************
 *
 ************************************************/
void ToolButton::paintEvent(QPaintEvent *event)
{
    if (icon().isNull())
        return;

    QPainter painter(this);

    QRect rect = event->rect();
    rect.setSize(iconSize());
    rect.moveCenter(event->rect().center());
    icon().paint(&painter, rect);
}

/************************************************
 *
 ************************************************/
void ToolButton::mousePressEvent(QMouseEvent *event)
{
    if (mBuddy) {
        mBuddy->setFocus();
    }

    QToolButton::mousePressEvent(event);
}

/************************************************
 *
 ************************************************/
void ToolButton::changeEvent(QEvent *event)
{
    // Workaround, already created menu doesn't change
    // your color when theme change in MacOS.
    if (event->type() == QEvent::PaletteChange) {
        if (menu())
            menu()->setStyle(menu()->style());
    }
}

/************************************************
 *
 ************************************************/
QLineEdit *ToolButton::buddyLineEdit()
{
    if (!buddy())
        return nullptr;

    QLineEdit *edit = qobject_cast<QLineEdit *>(buddy());
    if (edit)
        return edit;

    QComboBox *cbx = qobject_cast<QComboBox *>(buddy());
    if (cbx)
        return cbx->lineEdit();

    return nullptr;
}

/************************************************
 *
 ************************************************/
void ToolButton::setBuddy(QComboBox *buddy)
{
    mBuddy = buddy;
}

/************************************************
 *
 ************************************************/
void ToolButton::setBuddy(QLineEdit *buddy)
{
    mBuddy = buddy;
}

/************************************************

 ************************************************/
OutPatternButton::OutPatternButton(QWidget *parent) :
    ToolButton(parent)
{
    setMenu(new QMenu(this));
    mSeparator = menu()->addSeparator();
}

/************************************************

 ************************************************/
void OutPatternButton::addPattern(const QString &pattern, const QString &title)
{
    QAction *act = new QAction(title + QStringLiteral(" (%1)").arg(pattern), this);
    act->setData(pattern);
    connect(act, &QAction::triggered,
            this, &OutPatternButton::patternTriggered);

    menu()->insertAction(mSeparator, act);
}

/************************************************

 ************************************************/
void OutPatternButton::addFullPattern(const QString &pattern, const QString &title)
{
    QAction *act = new QAction(title, this);
    act->setData(pattern);
    connect(act, &QAction::triggered,
            this, &OutPatternButton::fullPatternTriggered);
    menu()->addAction(act);
}

/************************************************
 *
 ************************************************/
void OutPatternButton::addStandardPatterns()
{
    addPattern("%n", tr("Insert \"Track number\""));
    addPattern("%N", tr("Insert \"Total number of tracks\""));
    addPattern("%a", tr("Insert \"Album Performer\""));
    addPattern("%A", tr("Insert \"Album title\""));
    addPattern("%t", tr("Insert \"Track title\""));
    addPattern("%y", tr("Insert \"Year\""));
    addPattern("%g", tr("Insert \"Genre\""));
    addPattern("%d", tr("Insert \"Disc number\""));
    addPattern("%D", tr("Insert \"Total number of discs\""));

    const static char *patterns[] = {
        "%a/{%y - }%A/%n - %t",
        "%a -{ %y }%A/%n - %t",
        "{%y }%A - %a/%n - %t",
        "%a/%A/%n - %t",
        "%a - %A/%n - %t",
        "%A - %a/%n - %t"
    };

    for (auto pattern : patterns) {

        addFullPattern(pattern,
                       tr("Use \"%1\"", "Predefined out file pattern, string like 'Use \"%a/%A/%n - %t\"'")
                                       .arg(pattern)
                               + "  ( " + PatternExpander::example(pattern) + ".wav )");
    }
}

/************************************************

 ************************************************/
void OutPatternButton::patternTriggered()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;

    QString text = act->data().toString();
    emit    paternSelected(text);

    QLineEdit *edit = buddyLineEdit();
    if (!edit)
        return;

    edit->insert(text);
    emit edit->textEdited(text);
    emit edit->editingFinished();
}

/************************************************

 ************************************************/
void OutPatternButton::fullPatternTriggered()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;

    QString text = act->data().toString();
    emit    fullPaternSelected(text);

    QLineEdit *edit = buddyLineEdit();
    if (!edit)
        return;

    edit->setText(text);
    emit edit->textEdited(text);
    emit edit->editingFinished();
}

/************************************************
 *
 ************************************************/
OutDirButton::OutDirButton(QWidget *parent) :
    ToolButton(Icon("pattern-button"), parent)
{
    fillMenu();
}

/************************************************
 *
 ************************************************/
void OutDirButton::fillMenu()
{
    QMenu *menu = new QMenu(this);
    setMenu(menu);

    {
        QAction *act = new QAction(menu);
        act->setText(tr("Select directory…", "Menu item for output direcory button"));
        connect(act, &QAction::triggered,
                this, &OutDirButton::openSelectDirDialog);
        menu->addAction(act);
    }

    menu->addSeparator();

    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (!dir.isEmpty() && dir != QDir::homePath()) {
            QAction *act = new QAction(menu);
            act->setText(tr("Standard music location", "Menu item for output direcory button"));
            connect(act, &QAction::triggered, this, [this, dir]() { setDirectory(dir); });
            menu->addAction(act);
        }
    }

    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        if (!dir.isEmpty()) {
            QAction *act = new QAction(menu);
            act->setText(tr("Desktop", "Menu item for output direcory button"));
            connect(act, &QAction::triggered, this, [this, dir]() { setDirectory(dir); });
            menu->addAction(act);
        }
    }

    {
        QAction *act = new QAction(menu);
        act->setText(tr("Same directory as CUE file", "Menu item for output direcory button"));
        connect(act, &QAction::triggered, this, [this]() { setDirectory(""); });
        menu->addAction(act);
    }
}

/************************************************
 *
 ************************************************/
void OutDirButton::openSelectDirDialog()
{
    QLineEdit *edit = buddyLineEdit();
    if (!edit)
        return;

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select result directory"), edit->text());
    if (!dir.isEmpty())
        setDirectory(dir);
}

/************************************************
 *
 ************************************************/
void OutDirButton::setDirectory(const QString &directory)
{
    QLineEdit *edit = buddyLineEdit();
    if (!edit)
        return;

    QString dir = directory;
    dir.replace(QDir::homePath(), "~");
    edit->setText(dir);
    emit edit->editingFinished();
}

/************************************************

 ************************************************/
CodePageComboBox::CodePageComboBox(QWidget *parent) :
    MultiValuesComboBox(parent)
{
    addItem(tr("Auto detect", "Codepage auto detection"), CODEC_AUTODETECT);
    insertSeparator(9999);

    addCodecName(tr("Unicode (UTF-8)"), TextCodecUtf8::NAME);
    addCodecName(tr("Unicode (UTF-16LE)"), TextCodecUtf16Le::NAME);
    addCodecName(tr("Unicode (UTF-16BE)"), TextCodecUtf16Be::NAME);

    insertSeparator(9999);

    addCodecName(tr("Cyrillic (Win-1251)"), TextCodecWindows1251::NAME);
    addCodecName(tr("Cyrillic (CP-866)"), TextCodecIbm866::NAME);

    insertSeparator(9999);

    addCodecName(tr("Latin-1 (ISO-8859-1)"), TextCodecIso8859_1::NAME);
    addCodecName(tr("Latin-2 (ISO-8859-2)"), TextCodecIso8859_2::NAME);
    addCodecName(tr("Latin-3 (ISO-8859-3)"), TextCodecIso8859_3::NAME);
    addCodecName(tr("Latin-4 (ISO-8859-4)"), TextCodecIso8859_4::NAME);
    addCodecName(tr("Latin-5 (ISO-8859-5)"), TextCodecIso8859_5::NAME);
    addCodecName(tr("Latin-6 (ISO-8859-6)"), TextCodecIso8859_6::NAME);
    addCodecName(tr("Latin-7 (ISO-8859-7)"), TextCodecIso8859_7::NAME);
    addCodecName(tr("Latin-8 (ISO-8859-8)"), TextCodecIso8859_8::NAME);
    addCodecName(tr("Latin-9 (ISO-8859-9)"), TextCodecIso8859_9::NAME);
    addCodecName(tr("Latin-10 (ISO-8859-10)"), TextCodecIso8859_10::NAME);

    addCodecName(tr("Latin-13 (ISO-8859-13)"), TextCodecIso8859_13::NAME);
    addCodecName(tr("Latin-14 (ISO-8859-14)"), TextCodecIso8859_14::NAME);
    addCodecName(tr("Latin-15 (ISO-8859-15)"), TextCodecIso8859_15::NAME);
    addCodecName(tr("Latin-16 (ISO-8859-16)"), TextCodecIso8859_16::NAME);

    insertSeparator(9999);
    addCodecName(tr("Windows 1250"), TextCodecWindows1250::NAME);
    addCodecName(tr("Windows 1252"), TextCodecWindows1252::NAME);
    addCodecName(tr("Windows 1253"), TextCodecWindows1253::NAME);
    addCodecName(tr("Windows 1254"), TextCodecWindows1254::NAME);
    addCodecName(tr("Windows 1255"), TextCodecWindows1255::NAME);
    addCodecName(tr("Windows 1256"), TextCodecWindows1256::NAME);
    addCodecName(tr("Windows 1257"), TextCodecWindows1257::NAME);
    addCodecName(tr("Windows 1258"), TextCodecWindows1258::NAME);

    insertSeparator(9999);
    addCodecName(tr("Simplified Chinese (GB18030)"), TextCodecGb18030::NAME);
    addCodecName(tr("Traditional Chinese (BIG5)"), TextCodecBig5::NAME);
    addCodecName(tr("Japanese (CP932)"), TextCodecShiftJis::NAME);
}

/************************************************

 ************************************************/
QString CodePageComboBox::codePage() const
{
    return currentData().toString();
}

/************************************************

 ************************************************/
void CodePageComboBox::setCodePage(const QString &value)
{
    for (int i = 0; i < count(); ++i) {
        if (this->itemData(i).toString() == value) {
            setCurrentIndex(i);
            return;
        }
    }
}

/************************************************

 ************************************************/
void CodePageComboBox::addCodecName(const QString &title, const QString &codecName)
{
    addItem(title, codecName);
}

/************************************************

 ************************************************/
MultiValuesSpinBox::MultiValuesSpinBox(QWidget *parent) :
    QSpinBox(parent),
    mMultiState(MultiValuesEmpty)
{
}

/************************************************
 *
 ************************************************/
static QString getTagEditText(const QSet<QString> &values)
{
    switch (values.count()) {
        case 0:
            return "";
        case 1:
            return *(values.constBegin());
        default:
            return "";
    }
}

/************************************************
 *
 ************************************************/
static QString getTagEditPlaceHolder(const QSet<QString> &values)
{
    switch (values.count()) {
        case 0:
            return "";
        case 1:
            return "";
        default:
            return QObject::tr("Multiple values");
    }
}

/************************************************

 ************************************************/
void MultiValuesSpinBox::stepBy(int steps)
{
    // The QSpinBox::stepBy resets the lineEdit.isModified value.
    // So we blockSignals, set modified and then emit the signals manually.
    this->blockSignals(true);
    if (mMultiState != MultiValuesSingle && steps > 0) {
        mMultiState = MultiValuesSingle;
        QSpinBox::stepBy(0);
    }
    else {
        QSpinBox::stepBy(steps);
    }
    this->blockSignals(false);

    lineEdit()->setModified(true);
    emit valueChanged(this->value());
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    // valueChanged marked deprecated after version 5.14.x
    emit valueChanged(lineEdit()->text());
#else
    // replacement for QSpinBox:valueChanged(QString)
    // https://doc.qt.io/qt-5/qspinbox.html#textChanged
    emit textChanged(lineEdit()->text());
#endif
}

/************************************************

 ************************************************/
void MultiValuesSpinBox::setMultiValue(QSet<int> value)
{
    if (value.count() == 0) {
        mMultiState = MultiValuesEmpty;
        QSpinBox::setValue(minimum());
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else if (value.count() == 1) {
        mMultiState = MultiValuesSingle;
        QSpinBox::setValue(*(value.constBegin()));
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else {
        mMultiState = MultiValuesMulti;
        QSpinBox::setValue(minimum());
        if (lineEdit())
            lineEdit()->setPlaceholderText(tr("Multiple values"));
    }
}

/************************************************
 *
 ************************************************/
void MultiValuesSpinBox::setModified(bool modified)
{
    lineEdit()->setModified(modified);
}

/************************************************

 ************************************************/
QString MultiValuesSpinBox::textFromValue(int val) const
{
    switch (mMultiState) {
        case MultiValuesEmpty:
            return "";

        case MultiValuesSingle:
            return QSpinBox::textFromValue(val);

        case MultiValuesMulti:
            return "";
    }

    return "";
}

/************************************************

 ************************************************/
MultiValuesLineEdit::MultiValuesLineEdit(QWidget *parent) :
    QLineEdit(parent),
    mCompleterModel(new QStringListModel(this))
{
    setCompleter(new QCompleter(this));
    completer()->setModel(mCompleterModel);
    completer()->setCaseSensitivity(Qt::CaseInsensitive);
    completer()->setCompletionMode(QCompleter::PopupCompletion);
}

/************************************************

 ************************************************/
void MultiValuesLineEdit::setMultiValue(QSet<QString> value)
{
    QString str = value.count() == 1 ? *(value.constBegin()) : "";

    if (text() != str) {
        QLineEdit::setText(str);
    }

    setPlaceholderText(value.count() > 1 ? tr("Multiple values") : "");
    mCompleterModel->setStringList(value.values());
}

/************************************************

 ************************************************/
MultiValuesComboBox::MultiValuesComboBox(QWidget *parent) :
    QComboBox(parent),
    mMultiState(MultiValuesEmpty)
{
}

/************************************************

 ************************************************/
void MultiValuesComboBox::setMultiValue(QSet<QString> value)
{
    QSet<QString> v = value;
    v.remove("");

    if (v.count() == 0) {
        mMultiState = MultiValuesEmpty;
        setCurrentIndex(-1);
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else if (v.count() == 1) {
        int n = this->findData(*(v.begin()));
        setCurrentIndex(n);
        if (n > -1)
            mMultiState = MultiValuesSingle;
        else
            mMultiState = MultiValuesEmpty;

        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else {
        mMultiState = MultiValuesMulti;
        setCurrentIndex(-1);
        if (lineEdit())
            lineEdit()->setPlaceholderText(tr("Multiple values"));
    }
}

/************************************************

 ************************************************/
ProgramEdit::ProgramEdit(ExtProgram *program, QWidget *parent) :
    QLineEdit(parent),
    mProgram(program)
{
    setText(mProgram->path());

    mBtn = new QToolButton(this);
    mBtn->setText("…");
    mBtn->setIcon(Icon("folder"));
    mBtn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    mBtn->setCursor(Qt::ArrowCursor);

    connect(mBtn, &QToolButton::clicked, this, &ProgramEdit::openDialog);
}

/************************************************

 ************************************************/
void ProgramEdit::find()
{
    if (text().isEmpty()) {
        setText(mProgram->find());
    }
}

/************************************************

 ************************************************/
void ProgramEdit::resizeEvent(QResizeEvent *)
{
    int   frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QRect btnRect    = QRect(QPoint(0, 0), QSize(rect().height(), rect().height()));

    btnRect.moveCenter(rect().center());
    btnRect.moveRight(rect().right());

    btnRect.adjust(frameWidth + 4, frameWidth + 4, -frameWidth - 4, -frameWidth - 4);

    mBtn->setGeometry(btnRect);
}

/************************************************

 ************************************************/
void ProgramEdit::openDialog()
{
    QString flt = tr("%1 program",
                     "This is part of filter for 'select program' dialog. %1 is a name of required program. Example: 'flac program (flac)'")
                          .arg(mProgram->name())
            + QStringLiteral(" (%1);;").arg(mProgram->name()) + tr("All files", "This is part of filter for 'select program' dialog. 'All files (*)'") + " (*)";

    QString fileName = !text().isEmpty() ? text() : "/usr/bin/" + mProgram->name();

    fileName = QFileDialog::getOpenFileName(this, tr("Select program file"), fileName, flt);
    if (!fileName.isEmpty())
        setText(fileName);
}

/************************************************

 ************************************************/
HistoryComboBox::HistoryComboBox(QWidget *parent) :
    QComboBox(parent),
    mModel(new QStringListModel(this)),
    mDeleteItemAct(nullptr)
{
    setEditable(true);
    setModel(mModel);
    this->completer()->setCaseSensitivity(Qt::CaseSensitive);

    setInsertPolicy(QComboBox::NoInsert);
    setMaxCount(10);

    connect(this->lineEdit(), &QLineEdit::editingFinished,
            this, &HistoryComboBox::addToHistory);

    connect(&mDeleteItemAct, &QAction::triggered,
            this, &HistoryComboBox::deleteItem);

    connect(this, qOverload<int>(&HistoryComboBox::activated), this, [this]() {
        if (this->lineEdit()) {
            emit lineEdit()->textEdited(this->lineEdit()->text());
        }
    });
}

/************************************************

 ************************************************/
QStringList HistoryComboBox::history() const
{
    return mModel->stringList();
}

/************************************************

 ************************************************/
void HistoryComboBox::setHistory(const QStringList &value)
{
    QStringList hist = value;
    hist.removeAll("");

    if (!currentText().isEmpty() && !hist.contains(this->currentText())) {
        hist.insert(0, currentText());
    }

    mModel->setStringList(hist);
}

/************************************************
 *
 ************************************************/
void HistoryComboBox::deleteItem()
{
    QStringList hist = this->history();
    QString     s    = currentText();
    hist.removeOne(s);

    lineEdit()->setText(hist.isEmpty() ? "" : hist.first());
    setHistory(hist);

    emit currentIndexChanged(currentIndex());
}

/************************************************
 *
 ************************************************/
void HistoryComboBox::addToHistory()
{
    QString s = currentText();
    if (s.isEmpty())
        return;

    QStringList hist = history();

    if (!hist.isEmpty() && hist.first() == s)
        return;

    hist.removeAll(s);
    hist.prepend(s);

    while (hist.count() > HISTORY_COUNT)
        hist.removeLast();

    setHistory(hist);
}

/************************************************
 *
 ************************************************/
ActionsButton::ActionsButton(QWidget *parent) :
    ToolButton(parent)
{
    connect(this, &QToolButton::clicked,
            this, &ActionsButton::popupMenu);
}

/************************************************
 *
 ************************************************/
void ActionsButton::popupMenu()
{
    QPoint p = parentWidget()->mapToGlobal(this->pos());
    p.ry() += height();
    mMenu.popup(p);
}

/************************************************
 *
 ************************************************/
OutDirComboBox::OutDirComboBox(QWidget *parent) :
    HistoryComboBox(parent)
{
    setEditable(true);
    lineEdit()->setPlaceholderText(tr("Same directory as CUE file", "Placeholder for output direcory combobox"));
}

/************************************************
 *
 ************************************************/
MultiValuesTextEdit::MultiValuesTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
}

/************************************************
 *
 ************************************************/
bool MultiValuesTextEdit::isModified() const
{
    return this->document()->isModified();
}

/************************************************
 *
 ************************************************/
void MultiValuesTextEdit::setMultiValue(QSet<QString> value)
{
    setPlainText(getTagEditText(value));
    setPlaceholderText(getTagEditPlaceHolder(value));
}

/************************************************
 *
 ************************************************/
ErrorBox::ErrorBox(QWidget *parent) :
    QMessageBox(parent)
{
    setIcon(QMessageBox::Critical);
    setTextFormat(Qt::RichText);
    setStandardButtons(QMessageBox::Ok);
}

/************************************************
 *
 ************************************************/
void ErrorBox::setMessages(const QStringList &messages)
{
    mMessgaes = messages;
    if (mMessgaes.count() == 1) {
        setText(mMessgaes.first());
        return;
    }

    QString txt;
    foreach (QString s, mMessgaes) {
        s.replace("\n", "<p>");
        txt += QStringLiteral("<li>%1</li>").arg(s);
    }

    setText("<ul>" + txt + "</ul>");
}

/************************************************
 *
 ************************************************/
void ErrorBox::addMessage(const QString &message)
{
    QStringList msgs = mMessgaes;
    msgs << message;
    this->setMessages(msgs);
}

/************************************************

************************************************/
void Controls::arangeTollBarButtonsWidth(QToolBar *toolBar)
{
    int w = 0;

    foreach (QAction *act, toolBar->actions()) {
        QToolButton *btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(act));
        if (btn) {
            w = qMax(w, btn->sizeHint().width());
        }
    }

    foreach (QAction *act, toolBar->actions()) {
        QToolButton *btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(act));
        if (btn) {
            btn->setMinimumWidth(w);
        }
    }
}
