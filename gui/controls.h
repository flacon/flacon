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

#ifndef CONTROLS_H
#define CONTROLS_H

#include <QSpinBox>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSet>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>
#include "types.h"
#include "disc.h"

class QStringListModel;
class QToolBar;
class ExtProgram;

enum MultiValuesState {
    MultiValuesEmpty,
    MultiValuesSingle,
    MultiValuesMulti
};

/************************************************
 *
 ************************************************/
class ToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit ToolButton(QWidget *parent = nullptr);
    virtual ~ToolButton() { }

    /// Returns this label's buddy, or nullptr if no buddy is currently set.
    QWidget *buddy() const { return mBuddy; }

    /// Sets this label's buddy to buddy.
    ///
    /// When the user presses the shortcut key indicated by this label,
    /// the keyboard focus is transferred to the label's buddy widget.
    void setBuddy(QComboBox *buddy);
    void setBuddy(QLineEdit *buddy);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

    QLineEdit *buddyLineEdit();

private:
    QWidget *mBuddy = nullptr;
};

/************************************************

 ************************************************/
class OutPatternButton : public ToolButton
{
    Q_OBJECT
public:
    explicit OutPatternButton(QWidget *parent = nullptr);
    void addPattern(const QString &pattern, const QString &title);
    void addFullPattern(const QString &pattern, const QString &title);

    void addStandardPatterns();

signals:
    void paternSelected(const QString &pattern);
    void fullPaternSelected(const QString &pattern);

private slots:
    void patternTriggered();
    void fullPatternTriggered();

private:
    QAction *mSeparator;
};

/************************************************
 *
 ************************************************/
class OutDirButton : public ToolButton
{
    Q_OBJECT
public:
    explicit OutDirButton(QWidget *parent = nullptr);

private slots:
    void openSelectDirDialog();
    void setDirectory(const QString &directory);

private:
    void fillMenu();
};

/************************************************
 *
 ************************************************/
class ActionsButton : public ToolButton
{
    Q_OBJECT
public:
    explicit ActionsButton(QWidget *parent = nullptr);
    QMenu *menu() { return &mMenu; }

private slots:
    void popupMenu();

private:
    QMenu mMenu;
};

/************************************************

 ************************************************/
class MultiValuesSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MultiValuesSpinBox(QWidget *parent = nullptr);
    bool multi() const { return mMultiState == MultiValuesMulti; }
    void stepBy(int steps);

    bool isModified() const { return lineEdit()->isModified(); }

public slots:
    void setMultiValue(QSet<int> value);
    void setModified(bool modified);

protected:
    QString textFromValue(int val) const;

private:
    MultiValuesState mMultiState;
};

/************************************************

 ************************************************/
class MultiValuesLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MultiValuesLineEdit(QWidget *parent = nullptr);

    QString multiValuesText() const { return mMultiValuesText; }
    void    setMultiValuesText(const QString &value) { mMultiValuesText = value; }

public slots:
    void setMultiValue(QSet<QString> value);

private:
    QStringListModel *mCompleterModel;
    QString           mMultiValuesText;
};

/************************************************

 ************************************************/
class TrackTagLineEdit : public MultiValuesLineEdit
{
    Q_OBJECT
public:
    using MultiValuesLineEdit::MultiValuesLineEdit;

    TrackTags::TagId tagId() const { return mTagId; }
    void             setTagId(TrackTags::TagId value) { mTagId = value; }

    void loadFromTracks(const TrackPtrList &tracks);
    void writeToTracks(const TrackPtrList &tracks);

private:
    TrackTags::TagId mTagId = TrackTags::TagId(-1);
};

/************************************************

 ************************************************/
class AlbumTagLineEdit : public MultiValuesLineEdit
{
    Q_OBJECT
public:
    using MultiValuesLineEdit::MultiValuesLineEdit;

    AlbumTags::TagId tagId() const { return mTagId; }
    void             setTagId(AlbumTags::TagId value) { mTagId = value; }

    void loadFromDisks(const DiskList &disks);
    void writeToDisks(const DiskList &disks);

private:
    AlbumTags::TagId mTagId = AlbumTags::TagId(-1);
};

/************************************************

 ************************************************/
class MultiValuesTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MultiValuesTextEdit(QWidget *parent = nullptr);

    bool    isModified() const;
    QString text() const { return this->toPlainText(); }

    QString multiValuesText() const { return mMultiValuesText; }
    void    setMultiValuesText(const QString &value) { mMultiValuesText = value; }

public slots:
    void setMultiValue(QSet<QString> value);

private:
    QString mMultiValuesText;
};

/************************************************

 ************************************************/
class TrackTagTextEdit : public MultiValuesTextEdit
{
    Q_OBJECT
public:
    using MultiValuesTextEdit::MultiValuesTextEdit;

    TrackTags::TagId tagId() const { return mTagId; }
    void             setTagId(TrackTags::TagId value) { mTagId = value; }

    void loadFromTracks(const TrackPtrList &tracks);
    void writeToTracks(const TrackPtrList &tracks);

private:
    TrackTags::TagId mTagId = TrackTags::TagId(-1);
};

/************************************************

 ************************************************/
class MultiValuesComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit MultiValuesComboBox(QWidget *parent = nullptr);
    bool multi() const { return mMultiState == MultiValuesMulti; }

public slots:
    void setMultiValue(QSet<QString> value);

private:
    MultiValuesState mMultiState;
};

/************************************************

 ************************************************/
class CodePageComboBox : public MultiValuesComboBox
{
    Q_OBJECT
public:
    explicit CodePageComboBox(QWidget *parent = nullptr);

    QString codePage() const;
    void    setCodePage(const QString &value);

private:
    void addCodecName(const QString &title, const QString &codecName);
};

/************************************************

 ************************************************/
#define YearSpinBox MultiValuesSpinBox

#define ActionPushButton QPushButton
#define ItemsLabel QLabel

/************************************************

 ************************************************/
class ProgramEdit : public QLineEdit
{
    Q_OBJECT
public:
    ProgramEdit(ExtProgram *program, QWidget *parent = nullptr);

    ExtProgram *program() const { return mProgram; }

public slots:
    void find();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void openDialog();

private:
    ExtProgram  *mProgram;
    QToolButton *mBtn;
};

/************************************************
 *
 ************************************************/
class HistoryComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit HistoryComboBox(QWidget *parent = nullptr);
    QStringList history() const;
    void        setHistory(const QStringList &value);

    QAction *deleteItemAction() { return &mDeleteItemAct; }

private slots:
    void addToHistory();
    void deleteItem();

private:
    QStringListModel *mModel;
    QAction           mDeleteItemAct;
};

/************************************************
 *
 ************************************************/
class OutDirComboBox : public HistoryComboBox
{
    Q_OBJECT
public:
    explicit OutDirComboBox(QWidget *parent = nullptr);
};

/************************************************
 *
 ************************************************/
template <typename T>
class EnumCombobox : public QComboBox
{
public:
    using QComboBox::QComboBox;

    T    value() const { return T(currentData().toInt()); }
    void setValue(const T &val) { setCurrentIndex(qMax(0, findData(int(val)))); }

    void addItem(const QString &atext, const T &value) { QComboBox::addItem(atext, int(value)); }
    // void QComboBox::addItem(const QString &atext, const QVariant &auserData)
};

/************************************************
 *
 ************************************************/
class ErrorBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit ErrorBox(QWidget *parent = nullptr);

    QStringList messages() const { return mMessgaes; }
    void        setMessages(const QStringList &messages);
    void        addMessage(const QString &message);

private:
    QStringList mMessgaes;
};

namespace Controls {

void arangeTollBarButtonsWidth(QToolBar *toolBar);

} // namespace
#endif // CONTROLS_H
