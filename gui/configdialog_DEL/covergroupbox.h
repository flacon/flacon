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

    CoverMode coverMode() const;
    void      setCoverMode(CoverMode value);

    int  imageSize();
    void setImageSize(int value);

private:
    Ui::CoverGroupBox *ui;

    bool mKeepSize = true;
};

#endif // COVERGROUPBOX_H
