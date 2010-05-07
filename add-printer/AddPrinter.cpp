/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "AddPrinter.h"

#include "AddPrinterInterface.h"
#include "PageIntro.h"
#include "PageDestinations.h"

#include <KAssistantDialog>
#include <QLabel>
#include <QVBoxLayout>

#include <KCmdLineArgs>
#include <KDebug>

AddPrinter::AddPrinter()
 : KUniqueApplication()
{
    setQuitOnLastWindowClosed(true);
//     m_pqInterface = new AddPrinterInterface(this);
//     connect(m_pqInterface, SIGNAL(quit()), this, SLOT(quit()));
}

QWidget *createIntroPage()
{
    QWidget *page = new QWidget;
//     page->setTitle("Introduction");

    PageIntro *label = new PageIntro;
//     label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWidget *createRegistrationPage()
{
    QWidget *page = new QWidget;
//     page->setTitle("Introduction");

    PageDestinations *label = new PageDestinations;


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

int AddPrinter::newInstance()
{
    KAssistantDialog *wizard = new KAssistantDialog;
    wizard->addPage(createIntroPage(), i18n("Welcome to the Add New Printer Wizard"));
    PageDestinations *widget = new PageDestinations;
    wizard->addPage(widget, i18n("Select a Printer to Add"));
//     connect(widget, SIGNAL(canProceed(bool)), wizard, SLOT(enable))
//     wizard.addPage(createConclusionPage());

    wizard->setWindowTitle(i18n("Add a New Printer"));
    wizard->showButton(KDialog::Cancel, false);
    QSize size = wizard->minimumSizeHint();
    size += QSize(150, 0);
    (void)wizard->setMaximumSize(size);
    (void)wizard->setMinimumSize(size);
    wizard->show();
//     wizard->layout()->setContentsMargins(0,0,0,0);

    return 0;
}

AddPrinter::~AddPrinter()
{
}

#include "AddPrinter.moc"
