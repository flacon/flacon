#ifndef UPDATEPAGE_H
#define UPDATEPAGE_H

#include <QWidget>

namespace Ui {
class UpdatePage;
}

class UpdatePage : public QWidget
{
    Q_OBJECT

public:
    explicit UpdatePage(QWidget *parent = nullptr);
    ~UpdatePage();

    void load();
    void save();

private:
    Ui::UpdatePage *ui;

    void updateLastUpdateLbl();
};

#endif // UPDATEPAGE_H
