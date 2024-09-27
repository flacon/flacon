#include "logview.h"
#include "ui_logview.h"
#include "debug.h"

LogView::LogView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogView)
{
    ui->setupUi(this);

    ui->logBrowser->setText(debugMessages().join("\n"));
}

LogView::~LogView()
{
    delete ui;
}
