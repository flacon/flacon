#ifndef FDKAACCONFIGPAGE_H
#define FDKAACCONFIGPAGE_H

#include "../../encoderconfigpage.h"

namespace Ui {
class FdkaacConfigPage;
}

class FdkaacConfigPage : public EncoderConfigPage
{
    Q_OBJECT
public:
    explicit FdkaacConfigPage(QWidget *parent = nullptr);
    ~FdkaacConfigPage();

    bool useVbr() const;
    int  vbrQuality() const;
    int  cbrBitrate() const;

    void load(const Profile &profile) override;
    void save(Profile *profile) override;

private:
    Ui::FdkaacConfigPage *ui;

    void initQualitySlider();
    void initCbrBitrateComboBox();

    void refresh();
};

#endif // FDKAACCONFIGPAGE_H
