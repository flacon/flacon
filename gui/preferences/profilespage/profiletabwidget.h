#ifndef PROFILETABWIDGET_H
#define PROFILETABWIDGET_H

#include <QTabWidget>
#include "profiles.h"
#include <memory>
#include "formats_out/encoderconfigpage.h"

namespace Ui {
class ProfileTabWidget;
}

class ProfileTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit ProfileTabWidget(QWidget *parent = nullptr);
    ~ProfileTabWidget();

    Profile profile() const;
    void    setProfile(const Profile &profile);

    QString profileName() const { return mProfile.name(); }
    void    setProfileName(const QString &value);

private:
    Ui::ProfileTabWidget *ui;

    Profile                            mProfile;
    std::unique_ptr<EncoderConfigPage> mEncoderWidget = nullptr;
};

#endif // PROFILETABWIDGET_H
