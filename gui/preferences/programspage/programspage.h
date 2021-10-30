#ifndef PROGRAMSPAGE_H
#define PROGRAMSPAGE_H

#include <QWidget>
#include <QList>

class ProgramEdit;

namespace Ui {
class ProgramsPage;
}

class ProgramsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProgramsPage(QWidget *parent = nullptr);
    ~ProgramsPage();

    void load();
    void save();

private:
    Ui::ProgramsPage *ui;

    QList<ProgramEdit *> mProgramEdits;

    void init();
};

#endif // PROGRAMSPAGE_H
