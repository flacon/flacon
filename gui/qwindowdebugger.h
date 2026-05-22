/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2026
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

#ifndef QWINOWDEBUGGER_H
#define QWINOWDEBUGGER_H
#ifndef NDEBUG

#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <iostream>
#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QClipboard>
#include <QMetaProperty>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTimer>

class QWindowDebugger : public QObject
{
public:
    static void start()
    {
        static QWindowDebugger instance;
    }

    virtual bool eventFilter(QObject *watched, QEvent *event);

protected:
    QWindowDebugger();

private:
    void showMenu();

    void printQss() const;
    void updateQss(const QString &qssFileName) const;
    void saveQss(const QString &qssFileName) const;
    void showWidgetInfo(const QWidget *widget) const;

    void buildQssMenu(QMenu &menu);

    static void copyToClipboard(const QString &text);
};

inline QWindowDebugger::QWindowDebugger() :
    QObject()
{
    qApp->installEventFilter(this);
}

inline bool QWindowDebugger::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev->key() == Qt::Key_F1 and ev->modifiers().testFlag(Qt::ShiftModifier)) {
            showMenu();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

inline void QWindowDebugger::copyToClipboard(const QString &text)
{
    qobject_cast<QApplication *>(qApp)->clipboard()->setText(text);
}

inline void QWindowDebugger::buildQssMenu(QMenu &menu)
{
    QString qssFileName = qApp->applicationFilePath() + ".css";

    QAction *act = nullptr;

    act = new QAction("Print QSS", &menu);
    connect(act, &QAction::triggered, [this]() { this->printQss(); });
    menu.addAction(act);

    act = new QAction(QString("Save QSS to %1").arg(qssFileName), &menu);
    connect(act, &QAction::triggered, [this, qssFileName]() { saveQss(qssFileName); });
    menu.addAction(act);

    act = new QAction(QString("Update QSS from %1").arg(qssFileName), &menu);
    connect(act, &QAction::triggered, [this, qssFileName]() { updateQss(qssFileName); });
    menu.addAction(act);
}

inline void QWindowDebugger::showMenu()
{
    QPoint   mousePos = QCursor::pos();
    QWidget *widget   = qobject_cast<QApplication *>(qApp)->widgetAt(mousePos);

    if (!widget) {
        return;
    }

    bool        valid = true;
    QStringList path;
    QStringList classes;
    QStringList cssPath;
    for (QWidget *w = widget; w != nullptr; w = w->parentWidget()) {
        valid = valid && !w->objectName().isEmpty();
        path.prepend(w->objectName());
        classes.prepend(w->metaObject()->className());
        cssPath.prepend(QString("%1#%2").arg(w->metaObject()->className()).arg(w->objectName()));
    }

    QMenu menu;
    menu.setStyleSheet("color: default; font-size: 10pt;");

    buildQssMenu(menu);
    menu.addSeparator();

    QAction *act = nullptr;

    if (!valid) {
        act = new QAction("The path contains gaps!", &menu);
        menu.addAction(act);
    }

    act = new QAction(QString("Information"), &menu);
    connect(act, &QAction::triggered, [this, widget]() { showWidgetInfo(widget); });
    menu.addAction(act);

    menu.addSeparator();

    if (!widget->objectName().isEmpty()) {
        act = new QAction(QString("Copy Object Name: %1").arg(widget->objectName()), &menu);
        connect(act, &QAction::triggered, [widget]() { copyToClipboard(widget->objectName()); });
        menu.addAction(act);

        menu.addSeparator();
    }

    act = new QAction(QString("Copy CSS PATH: %1").arg(cssPath.join(" ")), &menu);
    connect(act, &QAction::triggered, [cssPath]() { qobject_cast<QApplication *>(qApp)->clipboard()->setText(cssPath.join(" ")); });
    menu.addAction(act);

    act = new QAction(QString("Copy PATH: %1").arg(path.join(".")), &menu);
    connect(act, &QAction::triggered, [path]() { qobject_cast<QApplication *>(qApp)->clipboard()->setText(path.join(".")); });
    menu.addAction(act);

    act = new QAction(QString("Copy CLASSES: %1").arg(classes.join(".")), &menu);
    connect(act, &QAction::triggered, [classes]() { qobject_cast<QApplication *>(qApp)->clipboard()->setText(classes.join(".")); });
    menu.addAction(act);

    menu.exec(mousePos);
}

inline void QWindowDebugger::printQss() const
{
    std::cout << qobject_cast<QApplication *>(qApp)->styleSheet().toStdString() << std::endl;
}

inline void QWindowDebugger::updateQss(const QString &qssFileName) const
{
    if (!QFile::exists(qssFileName)) {
        QMessageBox::warning(nullptr, "Update QSS style", QString("File not found: %1").arg(qssFileName));
        return;
    }
    QFile file(qssFileName);

    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(nullptr, "Update QSS style", QString("Can't update QSS style: %1").arg(file.errorString()));
        return;
    }

    qobject_cast<QApplication *>(qApp)->setStyleSheet(QLatin1String(file.readAll()));
    file.close();
}

inline void QWindowDebugger::saveQss(const QString &qssFileName) const
{
    QFile file(qssFileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(nullptr, "Save QSS", QString("Can't save QSS file: %1").arg(file.errorString()));
        return;
    }

    file.write(qobject_cast<QApplication *>(qApp)->styleSheet().toUtf8().data());

    file.close();
}

inline void QWindowDebugger::showWidgetInfo(const QWidget *widget) const
{
    QString text;
    text += "<html>";
    text += QString("<b>Class: %1</b><br>").arg(widget->metaObject()->className());
    text += QString("<b>Name: %1</b>").arg(widget->objectName());

    const QString TWO_COLUMNS("<tr><td>%1</td><td>%2</td></tr>");

    text += "<br>";
    text += "<table border=0 cellspacing=0 cellpadding=4 style='with: 100%;'>";
    text += "<tr><th>Name</th><th>Value</th></tr>";

    text += TWO_COLUMNS.arg("Font size (px)").arg(widget->font().pixelSize());
    text += TWO_COLUMNS.arg("Font size (point)").arg(widget->font().pointSize());

    text += "</table>";
    text += "<br>";

    text += "<table border=0 cellspacing=0 cellpadding=4 style='with: 100%;'>";
    text += "<tr><th>Name</th><th>Type</th><th>Value</th></tr>";

    QMap<QString, QMetaProperty> props;
    for (int i = 0; i < widget->metaObject()->propertyCount(); ++i) {
        auto property = widget->metaObject()->property(i);
        props.insert(property.name(), property);
    }

    int n = 0;
    for (QString name : props.keys()) {
        auto property = props[name];
        n++;
        QString style;
        if (n % 2 == 0) {
            style = "background-color: #F7F4F7;";
        }

        text += QString("<tr style='%4'><td>%1</td><td>%2</td><td>%3</td></tr>")
                        .arg(name)
                        .arg(property.typeName())
                        .arg(widget->property(name.toLatin1()).toString())
                        .arg(style);
    }
    text += "</table>";
    text += "</html>";

    QDialog *dialog = new QDialog();
    dialog->setStyleSheet("color: default; font-size: 10pt;");
    dialog->setWindowTitle(QString("Information about: %1 %2").arg(widget->metaObject()->className()).arg(widget->objectName()));
    dialog->resize(600, 900);
    dialog->connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);

    QTextBrowser *browser = new QTextBrowser(dialog);
    browser->setHtml(text);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(browser);
    dialog->setLayout(layout);
    dialog->show();
}

#endif
#endif // QWINOWDEBUGGER_H
