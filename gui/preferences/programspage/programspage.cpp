/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "programspage.h"
#include "ui_programspage.h"
#include "settings.h"
#include "controls.h"
#include "extprogram.h"
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
    QList<ExtProgram *> progs = ExtProgram::allPrograms();

    std::sort(progs.begin(), progs.end(), [](const ExtProgram *a, const ExtProgram *b) -> bool {
        return a->name() > b->name();
    });

    int row = 0;
    foreach (ExtProgram *prog, progs) {
        ProgramEdit *edit = new ProgramEdit(prog, this);
        mProgramEdits << edit;

        QLabel *label = new QLabel(tr("%1:", "Template for the program name label on the preferences form. %1 is a program name.").arg(prog->name()));

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
}

void ProgramsPage::save()
{
    for (ProgramEdit *edit : mProgramEdits) {
        edit->program()->setPath(edit->text());
    }
}
