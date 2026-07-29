#ifndef CONFIGPAGE_ACC_H
#define CONFIGPAGE_ACC_H

#include "../encoderconfigpage.h"

class QLabel;
class QComboBox;
class FaacConfigPage;
class FdkaacConfigPage;
class QStackedWidget;

class ConfigPage_Acc : public EncoderConfigPage
{
    Q_OBJECT
public:
    explicit ConfigPage_Acc(QWidget *parent = nullptr);

    void load(const Profile &profile) override;
    void save(Profile *profile) override;

private:
    QLabel    *mProgLabel = nullptr;
    QComboBox *mProgCombo = nullptr;

    QStackedWidget   *mPages      = nullptr;
    FaacConfigPage   *mFaacPage   = nullptr;
    FdkaacConfigPage *mFdkaacPage = nullptr;

    void initSingle();
    void initDouble();

    void progComboChanged();
};
#endif // CONFIGPAGE_ACC_H
