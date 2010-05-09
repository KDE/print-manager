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

#include "AddPrinterAssistant.h"

#include "PageIntro.h"
#include "PageDestinations.h"

#include <QLabel>
#include <QVBoxLayout>

#include <KDebug>

AddPrinterAssistant::AddPrinterAssistant()
 : KAssistantDialog()
{
    setWindowTitle(i18n("Add a New Printer"));
    showButton(KDialog::Cancel, false);

    addPage(createIntroPage(), i18n("Welcome to the Add New Printer Wizard"));
    PageDestinations *widget = new PageDestinations;
    addPage(widget, i18n("Select a Printer to Add"));
//     connect(widget, SIGNAL(canProceed(bool)), wizard, SLOT(enable))
//     wizard.addPage(createConclusionPage());




//     wizard->layout()->setContentsMargins(0,0,0,0);
    QSize size = minimumSizeHint();
    size += QSize(150, 0);
    setMaximumSize(size);
    setMinimumSize(size);
}

QWidget* AddPrinterAssistant::createIntroPage()
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

QWidget* AddPrinterAssistant::createRegistrationPage()
{
    QWidget *page = new QWidget;
//     page->setTitle("Introduction");

    PageDestinations *label = new PageDestinations;


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

void AddPrinterAssistant::next()
{
//     QModelIndex nextIndex=d->getNext(d->pageModel->index(currentPage()));
//     if (nextIndex.isValid())
//         setCurrentPage(d->pageModel->item(nextIndex));
}

AddPrinterAssistant::~AddPrinterAssistant()
{
}

#include "AddPrinterAssistant.moc"
