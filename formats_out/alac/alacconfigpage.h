#ifndef ALACCONFIGPAGE_H
#define ALACCONFIGPAGE_H

#include <QWidget>
#include "../encoderconfigpage.h"

namespace Ui {
class AlacConfigPage;
}

class AlacConfigPage : public EncoderConfigPage
{
    Q_OBJECT

public:
    explicit AlacConfigPage(QWidget *parent = nullptr);
    ~AlacConfigPage();

    void load(const Profile &profile) override;
    void save(Profile *profile) override;

private:
    Ui::AlacConfigPage *ui;
};

#endif // ALACCONFIGPAGE_H
