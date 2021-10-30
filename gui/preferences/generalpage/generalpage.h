#ifndef GENERALPAGE_H
#define GENERALPAGE_H

#include <QWidget>

namespace Ui {
class GeneralPage;
}

class GeneralPage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralPage(QWidget *parent = nullptr);
    ~GeneralPage();

    QString tmpDir() const;
    void    setTmpDir(const QString &value);

    QString defaultCodepage() const;
    void    setDefaultCodepage(const QString &value);

    uint encoderThreadsCount() const;
    void setEncoderThreadsCount(uint value);

    QString cddbHost() const;
    void    setCddbHost(const QString &value);

private:
    Ui::GeneralPage *ui;

    void showTmpDirDialog();
};

#endif // GENERALPAGE_H
