#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QMainWindow>
#include <QCloseEvent>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QMainWindow
{
    Q_OBJECT

public:
    static PreferencesDialog *createAndShow(QWidget *parent = nullptr);
    static PreferencesDialog *createAndShow(const QString &profileId, QWidget *parent = nullptr);

    static void fixLayout(const QWidget *parent);

signals:
    void finished();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::PreferencesDialog *ui;

    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void load();
    void save();

    void initToolBar();
    void showProfile(const QString &profileId);
    void done(bool accept);
};

#endif // PREFERENCESDIALOG_H
