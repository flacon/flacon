#ifndef OUTTAGSPAGE_H
#define OUTTAGSPAGE_H

#include <QWidget>

namespace Ui {
class OutTagsPage;
}

class OutTagsPage : public QWidget
{
    Q_OBJECT

public:
    explicit OutTagsPage(QWidget *parent = nullptr);
    ~OutTagsPage();

    bool isWriteSingleDiskNum() const;
    void setWriteSingleDiskNum(bool value);

private:
    Ui::OutTagsPage *ui;
};

#endif // OUTTAGSPAGE_H
