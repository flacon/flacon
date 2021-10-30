#include "programspage.h"
#include "ui_programspage.h"
#include "settings.h"
#include "controls.h"
#include <QLabel>

ProgramsPage::ProgramsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProgramsPage)
{
    ui->setupUi(this);
}

ProgramsPage::~ProgramsPage()
{
    delete ui;
}

void ProgramsPage::load()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList progs = QStringList::fromSet(Settings::i()->programs());
#else
    // After 5.14.0, QT has stated range constructors are available and preferred.
    // See: https://doc.qt.io/qt-5/qlist.html#fromSet
    QSet<QString> program_set = Settings::i()->programs();
    QStringList   progs       = QStringList(program_set.begin(), program_set.end());
#endif
    progs.sort();

    int row = 0;
    foreach (QString prog, progs) {
        ProgramEdit *edit = new ProgramEdit(prog, this);
        mProgramEdits << edit;

        QLabel *label = new QLabel(tr("%1:", "Template for the program name label on the preferences form. %1 is a program name.").arg(prog));
        label->setBuddy(edit);
#ifdef Q_OS_MAC
        label->setAlignment(Qt::AlignRight);
#endif
        ui->progsLayout->addWidget(label, row, 0);
        ui->progsLayout->addWidget(edit, row, 1);
        connect(ui->progScanButton, &QPushButton::clicked, edit, &ProgramEdit::find);
        row++;
    }

    ui->progsArea->setStyleSheet("QScrollArea, #scrollAreaWidgetContents { background-color: transparent;}");

    for (ProgramEdit *edit : mProgramEdits) {
        edit->setText(Settings::i()->value("Programs/" + edit->programName()).toString());
    }
}

void ProgramsPage::save()
{
    for (ProgramEdit *edit : mProgramEdits) {
        Settings::i()->setValue("Programs/" + edit->programName(), edit->text());
    }
}
