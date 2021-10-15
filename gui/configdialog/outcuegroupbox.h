#ifndef OUTCUEGROUPBOX_H
#define OUTCUEGROUPBOX_H

#include <QGroupBox>
#include "types.h"

namespace Ui {
class OutCueGroupBox;
}

class OutCueGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit OutCueGroupBox(QWidget *parent = nullptr);
    ~OutCueGroupBox();

    QString perTrackCueFormat() const;
    void    setPerTrackCueFormat(const QString &value);

    bool isCreateCue() const;
    void setCreateCue(bool value);

    bool isEmbededCue() const;
    void setEmbededCue(bool value);

    PreGapType pregapType() const;
    void       setPregapType(PreGapType value);

    bool isSupportEmbededCue() const { return mSupportEmbededCue; }
    void setSupportEmbededCue(bool value);

private:
    Ui::OutCueGroupBox *ui;

    void refresh();
    bool mSupportEmbededCue = false;
};

#endif // OUTCUEGROUPBOX_H
