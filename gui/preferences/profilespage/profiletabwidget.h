#ifndef PROFILETABWIDGET_H
#define PROFILETABWIDGET_H

#include <QTabWidget>
#include "profiles.h"
#include <memory>
#include "formats_out/encoderconfigpage.h"
#include "../controls.h"

namespace Ui {
class ProfileTabWidget;
}

class ProfileTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit ProfileTabWidget(QWidget *parent = nullptr);
    ~ProfileTabWidget();

    void fromProfile(const Profile &profile);
    void toProfile(Profile *profile) const;

private:
    Ui::ProfileTabWidget *ui;

    std::unique_ptr<EncoderConfigPage> mEncoderWidget = nullptr;

    void recreateEncoderWidget(const Profile &profile);
};

using BitsPerSampleCombobox = EnumCombobox<int>;
using SampleRateCombobox    = EnumCombobox<SampleRate>;
using GainTypeCombobox      = EnumCombobox<GainType>;

#endif // PROFILETABWIDGET_H
