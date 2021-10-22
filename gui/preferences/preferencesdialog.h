#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QMainWindow>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QMainWindow
{
    Q_OBJECT

public:
    static PreferencesDialog *createAndShow(QWidget *parent = nullptr);
    static PreferencesDialog *createAndShow(const QString &profileId, QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::PreferencesDialog *ui;

    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void done(bool accept);
    void load();
    void save();
    void fixLayout();
};

#endif // PREFERENCESDIALOG_H
