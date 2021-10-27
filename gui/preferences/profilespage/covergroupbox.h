#ifndef COVERGROUPBOX_H
#define COVERGROUPBOX_H

#include <QGroupBox>
#include "../types.h"

namespace Ui {
class CoverGroupBox;
}

class CoverGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CoverGroupBox(QWidget *parent = nullptr);
    ~CoverGroupBox();

    CoverOptions coverOptions() const;
    void         setCoverOptions(const CoverOptions &value);

private:
    Ui::CoverGroupBox *ui;

    bool mKeepSize = true;

    void refresh();
};

#endif // COVERGROUPBOX_H
