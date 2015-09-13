#ifndef CUEDISKSELECTDIALOG_H
#define CUEDISKSELECTDIALOG_H

#include <QDialog>
class CueReader;
class QModelIndex;

namespace Ui {
class CueDiskSelectDialog;
}

class CueDiskSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CueDiskSelectDialog(const CueReader &cue, int selectedDisk = 0, QWidget *parent = 0);
    ~CueDiskSelectDialog();

    static int getDiskNumber(const CueReader &cue, int selectedDisk = 0);

    int diskNumber();
private slots:
    void treeDoubleClicked(const QModelIndex &index);

private:
    Ui::CueDiskSelectDialog *ui;
    const CueReader &mCue;
};

#endif // CUEDISKSELECTDIALOG_H
