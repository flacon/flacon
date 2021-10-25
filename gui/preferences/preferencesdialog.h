#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    static PreferencesDialog *createAndShow(QWidget *parent = nullptr);
    static PreferencesDialog *createAndShow(const QString &profileId, QWidget *parent = nullptr);

public slots:
    void done(int res) override;

private:
    Ui::PreferencesDialog *ui;

    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void load();
    void save();
    void fixLayout();
    void initToolBar();
    void showProfile(const QString &profileId);
};

#endif // PREFERENCESDIALOG_H
