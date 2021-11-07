#ifndef CUEGROUPBOX_H
#define CUEGROUPBOX_H

#include <QGroupBox>
#include "types.h"
#include "profiles.h"
#include "controls.h"

namespace Ui {
class CueGroupBox;
}

class CueGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CueGroupBox(QWidget *parent = nullptr);
    ~CueGroupBox();

    void fromProfile(const Profile &profile);
    void toProfile(Profile *profile) const;

private:
    Ui::CueGroupBox *ui;

    void refresh();
    bool mSupportEmbeddedCue = false;
};

using PreGapTypeComboBox = EnumCombobox<PreGapType>;

#endif // CUEGROUPBOX_H
