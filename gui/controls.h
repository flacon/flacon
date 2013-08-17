/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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
#include <QSet>

enum MultiValuesState
{
    MultiValuesEmpty,
    MultiValuesSingle,
    MultiValuesMulti
};

/************************************************

 ************************************************/
class OutPatternButton: public QToolButton
{
    Q_OBJECT
public:
    explicit OutPatternButton(QWidget * parent=0);
    void addPattern(const QString &pattern, const QString &title);

signals:
    void paternSelected(const QString &pattern);

private slots:
    void patternTriggered();

};




/************************************************

 ************************************************/
class MultiValuesSpinBox: public QSpinBox
{
    Q_OBJECT
public:
    explicit MultiValuesSpinBox(QWidget *parent = 0);
    bool multi() const { return mMultiState == MultiValuesMulti; }
    void stepBy(int steps);

public slots:
    void setMultiValue(QSet<int> value);

protected:
    QString textFromValue(int val) const;

private:
    MultiValuesState mMultiState;
};



/************************************************

 ************************************************/
class MultiValuesLineEdit: public QLineEdit
{
    Q_OBJECT
public:
    explicit MultiValuesLineEdit(QWidget *parent = 0);
    bool multi() const { return mMultiState == MultiValuesMulti; }

public slots:
    void setMultiValue(QSet<QString> value);

private:
    MultiValuesState mMultiState;
};


/************************************************

 ************************************************/
class TagLineEdit: public MultiValuesLineEdit
{
    Q_OBJECT
public:
    explicit TagLineEdit(QWidget *parent = 0): MultiValuesLineEdit(parent) {}

    QString tagName() const { return mTagName; }
    void setTagName(const QString &tagName) { mTagName = tagName; }

private:
    QString mTagName;
};

/************************************************

 ************************************************/
class MultiValuesComboBox: public QComboBox
{
    Q_OBJECT
public:
    MultiValuesComboBox(QWidget *parent = 0);
    bool multi() const { return mMultiState == MultiValuesMulti; }

public slots:
    void setMultiValue(QSet<QString> value);

private:
    MultiValuesState mMultiState;
};



/************************************************

 ************************************************/
class CodePageComboBox: public MultiValuesComboBox
{
    Q_OBJECT
public:
    CodePageComboBox(QWidget *parent = 0);

private:
    void addCodecName(const QString &title, const QString &codecName);
};


/************************************************

 ************************************************/
#define YearSpinBox         MultiValuesSpinBox


#define ActionPushButton    QPushButton
#define ItemsLabel          QLabel


/************************************************

 ************************************************/
class ProgramEdit: public QLineEdit
{
    Q_OBJECT
public:
    ProgramEdit(const QString &programName, QWidget *parent = 0);

    QString programName() const { return mProgramName;}

public slots:
    void find();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void openDialog();

private:
    QString mProgramName;
    QToolButton *mBtn;
};


#endif // CONTROLS_H
