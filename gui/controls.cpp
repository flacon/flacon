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
#include "settings.h"

#include <QMenu>
#include <QDebug>
#include <QTextCodec>
#include <QPaintEvent>
#include <QPainter>
#include <QFileDialog>
#include <QApplication>
#include <QCompleter>
#include <QStringListModel>


/************************************************

 ************************************************/
OutPatternButton::OutPatternButton(QWidget * parent):
    QToolButton(parent)
{
    mSeparator = mMenu.addSeparator();
    connect(this, SIGNAL(clicked(bool)),
            this, SLOT(popupMenu()));
}


/************************************************

 ************************************************/
void OutPatternButton::addPattern(const QString &pattern, const QString &title)
{
    QAction *act = new QAction(title, this);
    act->setData(pattern);
    connect(act, SIGNAL(triggered()), this, SLOT(patternTriggered()));
    mMenu.insertAction(mSeparator, act);
}


/************************************************

 ************************************************/
void OutPatternButton::addFullPattern(const QString &pattern, const QString &title)
{
    QString example = Track::calcFileName(pattern, 14, 13, "Help", "Yesterday", "The Beatles", "Pop", "1965", "flac");
    QAction *act = new QAction(title + "  ( " + example + " )", this);
    act->setData(pattern);
    connect(act, SIGNAL(triggered()), this, SLOT(fullPatternTriggered()));
    mMenu.addAction(act);
}


/************************************************

 ************************************************/
void OutPatternButton::patternTriggered()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
        emit paternSelected(act->data().toString());
}


/************************************************

 ************************************************/
void OutPatternButton::fullPatternTriggered()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
        emit fullPaternSelected(act->data().toString());
}


/************************************************
 *
 ************************************************/
void OutPatternButton::popupMenu()
{
    QPoint p = parentWidget()->mapToGlobal(this->pos());
    p.ry() += height();
    mMenu.popup(p);
}


/************************************************

 ************************************************/
CodePageComboBox::CodePageComboBox(QWidget *parent):
    MultiValuesComboBox(parent)
{
    addItem(tr("Auto detect", "Codepage auto detection"),  CODEC_AUTODETECT);
    insertSeparator(9999);

    addCodecName(tr("Unicode (UTF-8)"),     "UTF-8");
    addCodecName(tr("Unicode (UTF-16LE)"),  "UTF-16LE");
    addCodecName(tr("Unicode (UTF-16BE)"),  "UTF-16BE");

    insertSeparator(9999);

    addCodecName(tr("Cyrillic (Win-1251)"), "windows-1251");
    addCodecName(tr("Cyrillic (CP-866)"),   "IBM866");

    insertSeparator(9999);

    addCodecName(tr("Latin-1 (ISO-8859-1)"),   "ISO-8859-1");
    addCodecName(tr("Latin-2 (ISO-8859-2)"),   "ISO-8859-2");
    addCodecName(tr("Latin-3 (ISO-8859-3)"),   "ISO-8859-3");
    addCodecName(tr("Latin-4 (ISO-8859-4)"),   "ISO-8859-4");
    addCodecName(tr("Latin-5 (ISO-8859-5)"),   "ISO-8859-5");
    addCodecName(tr("Latin-6 (ISO-8859-6)"),   "ISO-8859-6");
    addCodecName(tr("Latin-7 (ISO-8859-7)"),   "ISO-8859-7");
    addCodecName(tr("Latin-8 (ISO-8859-8)"),   "ISO-8859-8");
    addCodecName(tr("Latin-9 (ISO-8859-9)"),   "ISO-8859-9");
    addCodecName(tr("Latin-10 (ISO-8859-10)"), "ISO-8859-10");

    addCodecName(tr("Latin-13 (ISO-8859-13)"), "ISO-8859-13");
    addCodecName(tr("Latin-14 (ISO-8859-14)"), "ISO-8859-14");
    addCodecName(tr("Latin-15 (ISO-8859-15)"), "ISO-8859-15");
    addCodecName(tr("Latin-16 (ISO-8859-16)"), "ISO-8859-16");

    insertSeparator(9999);
    addCodecName(tr("Windows 1250"), "windows-1250");
    addCodecName(tr("Windows 1252"), "windows-1252");
    addCodecName(tr("Windows 1253"), "windows-1253");
    addCodecName(tr("Windows 1254"), "windows-1254");
    addCodecName(tr("Windows 1255"), "windows-1255");
    addCodecName(tr("Windows 1256"), "windows-1256");
    addCodecName(tr("Windows 1257"), "windows-1257");
    addCodecName(tr("Windows 1258"), "windows-1258");

}


/************************************************

 ************************************************/
void CodePageComboBox::addCodecName(const QString &title, const QString &codecName)
{
    if (QTextCodec::availableCodecs().contains(codecName.toLatin1()))
        addItem(title, codecName);
}


/************************************************

 ************************************************/
MultiValuesSpinBox::MultiValuesSpinBox(QWidget *parent):
    QSpinBox(parent),
    mMultiState(MultiValuesEmpty)
{
}


/************************************************
 *
 ************************************************/
static MultiValuesState getTagEditState(const QSet<QString> values)
{
    switch (values.count())
    {
    case 0:  return MultiValuesEmpty;
    case 1:  return MultiValuesSingle;
    default: return MultiValuesMulti;
    }
}


/************************************************
 *
 ************************************************/
static QString getTagEditText(const QSet<QString> values)
{
    switch (values.count())
    {
    case 0:  return "";
    case 1:  return *(values.constBegin());
    default: return "";
    }
}


/************************************************
 *
 ************************************************/
static QString getTagEditPlaceHolder(const QSet<QString> values)
{
    switch (values.count())
    {
    case 0:  return "";
    case 1:  return "";
    default: return QObject::tr("Multiple values");
    }
}


/************************************************

 ************************************************/
void MultiValuesSpinBox::stepBy(int steps)
{
    // The QSpinBox::stepBy resets the lineEdit.isModified value.
    // So we blockSignals, set modified and then emit the signals manually.
    this->blockSignals(true);
    if (mMultiState != MultiValuesSingle && steps > 0)
    {
        mMultiState = MultiValuesSingle;
        QSpinBox::stepBy(0);
    }
    else
    {
        QSpinBox::stepBy(steps);
    }
    this->blockSignals(false);

    lineEdit()->setModified(true);
    emit valueChanged(this->value());
    emit valueChanged(lineEdit()->text());
}



/************************************************

 ************************************************/
void MultiValuesSpinBox::setMultiValue(QSet<int> value)
{
    if (value.count() == 0)
    {
        mMultiState = MultiValuesEmpty;
        QSpinBox::setValue(minimum());
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else if (value.count() == 1)
    {
        mMultiState = MultiValuesSingle;
        QSpinBox::setValue(*(value.constBegin()));
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else
    {
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
    switch (mMultiState)
    {
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
MultiValuesLineEdit::MultiValuesLineEdit(QWidget *parent):
    QLineEdit(parent),
    mMultiState(MultiValuesEmpty),
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
    if (value.count() == 0)
    {
        mMultiState = MultiValuesEmpty;
        QLineEdit::setText("");
        setPlaceholderText("");
        mCompleterModel->setStringList(QStringList());
    }

    else if (value.count() == 1)
    {
        mMultiState = MultiValuesEmpty;
        QLineEdit::setText(*(value.constBegin()));
        setPlaceholderText("");
        mCompleterModel->setStringList(value.toList());
    }

    else
    {
        mMultiState = MultiValuesMulti;
        QLineEdit::setText("");
        setPlaceholderText(tr("Multiple values"));
        mCompleterModel->setStringList(value.toList());
    }
}

/************************************************
 *
 ************************************************/

TagLineEdit::TagLineEdit(QWidget *parent):
    MultiValuesLineEdit(parent),
    mTagId(TagId())
{
}


/************************************************

 ************************************************/
MultiValuesComboBox::MultiValuesComboBox(QWidget *parent):
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

    if (v.count() == 0)
    {
        mMultiState = MultiValuesEmpty;
        setCurrentIndex(-1);
        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else if (v.count() == 1)
    {
        int n = this->findData(*(v.begin()));
        setCurrentIndex(n);
        if (n >-1)
            mMultiState = MultiValuesSingle;
        else
            mMultiState = MultiValuesEmpty;

        if (lineEdit())
            lineEdit()->setPlaceholderText("");
    }

    else
    {
        mMultiState = MultiValuesMulti;
        setCurrentIndex(-1);
        if (lineEdit())
            lineEdit()->setPlaceholderText(tr("Multiple values"));
    }
}


/************************************************

 ************************************************/
ProgramEdit::ProgramEdit(const QString &programName, QWidget *parent):
    QLineEdit(parent),
    mProgramName(programName)
{
    mBtn = new QToolButton(this);
    mBtn->setText("…");
    mBtn->setIcon(loadIcon("folder"));
    mBtn->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    mBtn->setCursor(Qt::ArrowCursor);

    connect(mBtn, SIGNAL(clicked()), this, SLOT(openDialog()));
}


/************************************************

 ************************************************/
void ProgramEdit::find()
{
    if (text().isEmpty())
        setText(settings->findProgram(mProgramName));
}


/************************************************

 ************************************************/
void ProgramEdit::resizeEvent(QResizeEvent *)
{
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QRect btnRect = QRect(QPoint(0,0), QSize(rect().height(), rect().height()));

    btnRect.moveCenter(rect().center());
    btnRect.moveRight(rect().right());

    btnRect.adjust(frameWidth + 4, frameWidth + 4 , -frameWidth - 4, -frameWidth - 4);

    mBtn->setGeometry(btnRect);
}


/************************************************

 ************************************************/
void ProgramEdit::openDialog()
{
    QString flt = tr("%1 program",
                     "This is part of filter for 'select program' dialog. %1 is a name of required program. Example: 'flac program (flac)'"
                     ).arg(mProgramName)  +
                    QString(" (%1);;").arg(mProgramName) +
                  tr("All files", "This is part of filter for 'select program' dialog. 'All files (*)'") +
                    " (*)";

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select program file"), "/usr/bin/", flt);
    if (!fileName.isEmpty())
        setText(fileName);
}


/************************************************

 ************************************************/
HistoryComboBox::HistoryComboBox(QWidget *parent):
    QComboBox(parent),
    mDeleteItemAct(nullptr)
{
    setInsertPolicy(QComboBox::InsertAtTop);
    setMaxCount(10);
    setAutoCompletionCaseSensitivity(Qt::CaseSensitive);

    connect(&mDeleteItemAct, &QAction::triggered,
            this, &HistoryComboBox::deleteItem);
}


/************************************************

 ************************************************/
QStringList HistoryComboBox::history() const
{
    QStringList res;
    for (int i=0; i<count(); ++i)
        res << itemText(i);

    return res;
}


/************************************************

 ************************************************/
void HistoryComboBox::setHistory(const QStringList &value)
{
    clear();
    foreach(QString s, value)
        addItem(s);
}


/************************************************

 ************************************************/
void HistoryComboBox::focusOutEvent(QFocusEvent *e)
{
    QKeyEvent key_press(QKeyEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QString(), false, 0 );
    QApplication::sendEvent(this, &key_press);

    QKeyEvent key_release(QKeyEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, QString(), false, 0 );
    QApplication::sendEvent(this, &key_release);

    QComboBox::focusOutEvent(e);
}


/************************************************
 *
 ************************************************/
void HistoryComboBox::deleteItem()
{
    QStringList hist = this->history();
    QString s = currentText();
    hist.removeOne(s);

    lineEdit()->setText(hist.isEmpty() ? "" : hist.first());
    setHistory(hist);

    emit currentIndexChanged(currentText());
    emit currentIndexChanged(currentIndex());
}


/************************************************
 *
 ************************************************/
ActionsButton::ActionsButton(QWidget *parent):
    QToolButton(parent)
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
OutDirComboBox::OutDirComboBox(QWidget *parent):
    HistoryComboBox(parent)
{

}


/************************************************
 *
 ************************************************/
TagSpinBox::TagSpinBox(QWidget *parent):
    MultiValuesSpinBox(parent),
    mTagId(TagId())
{
}


/************************************************
 *
 ************************************************/
MultiValuesTextEdit::MultiValuesTextEdit(QWidget *parent):
    QPlainTextEdit(parent),
    mMultiState(MultiValuesEmpty)
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
    mMultiState = getTagEditState(value);
    setPlainText(getTagEditText(value));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    setPlaceholderText(getTagEditPlaceHolder(value));
#endif
}


/************************************************
 *
 ************************************************/
TagTextEdit::TagTextEdit(QWidget *parent):
    MultiValuesTextEdit(parent),
    mTagId(TagId())
{

}
