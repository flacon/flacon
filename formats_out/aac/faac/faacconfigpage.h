#ifndef FAACCONFIGPAGE_H
#define FAACCONFIGPAGE_H

#include "../../encoderconfigpage.h"

namespace Ui {
class FaacConfigPage;
}

class FaacConfigPage : public EncoderConfigPage
{
    Q_OBJECT

public:
    explicit FaacConfigPage(QWidget *parent = nullptr);
    ~FaacConfigPage();

    static QHash<QString, QVariant> defaultParameters();

    virtual void load(const Profile &profile) override;
    virtual void save(Profile *profile) override;

private:
    void useQualityChecked(bool checked);

    Ui::FaacConfigPage *ui;
};

#endif // FAACCONFIGPAGE_H
