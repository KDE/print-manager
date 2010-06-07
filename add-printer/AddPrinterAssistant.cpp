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
#include "PageChoose.h"
#include "PageAddPrinter.h"

#include <QCups.h>

#include <KMessageBox>

#include <KDebug>

AddPrinterAssistant::AddPrinterAssistant()
 : KAssistantDialog()
{
    setWindowTitle(i18n("Add a New Printer"));
    setWindowIcon(KIcon("printer"));
    showButton(KDialog::Cancel, false);

    m_introPage = new KPageWidgetItem(new PageIntro, i18n("Welcome to the Add New Printer Assistant"));
    addPage(m_introPage);

    m_devicesPage = new KPageWidgetItem(new PageDestinations, i18n("Select a Printer to Add"));
    addPage(m_devicesPage);

    m_choosePage = new KPageWidgetItem(new PageChoose, i18n("Configure your connection"));
    addPage(m_choosePage);

//     m_serialPage = new KPageWidgetItem(new PageSerial, i18n("Select a Printer to Add"));
//     addPage(m_serialPage);

    m_addPrinterPage = new KPageWidgetItem(new PageAddPrinter, i18n("Please describe you printer"));
    addPage(m_addPrinterPage);

    setDefaultButton(KDialog::User2); // next
    setDefaultButton(KDialog::User1); // finished
    QSize size = minimumSizeHint();
    size += QSize(150, 0);
    setMaximumSize(size);
    setMinimumSize(size);
}

void AddPrinterAssistant::back()
{
    KAssistantDialog::back();
    if (!qobject_cast<GenericPage*>(currentPage()->widget())->isValid()) {
        back();
    }
}

void AddPrinterAssistant::next()
{
    next(currentPage());
}

void AddPrinterAssistant::next(KPageWidgetItem *currentPage)
{
    // Each page has all it's settings and previous pages
    // settings stored, so when going backwards
    // we don't set (or even unset values),
    // and we only call setValues on the next page if
    // the currentPage() has changes.
    // And if it hasChanges() we get it's values and
    // pass it to the next page so it "clans up" and
    // start as it was the first time
    QHash<QString, QVariant> args = qobject_cast<GenericPage*>(currentPage->widget())->values();
    if (currentPage == m_introPage) {
        qobject_cast<GenericPage*>(m_devicesPage->widget())->setValues(args);
        setCurrentPage(m_devicesPage);
    } else if (currentPage == m_devicesPage) {
        qobject_cast<GenericPage*>(m_choosePage->widget())->setValues(args);
        setCurrentPage(m_choosePage);
    } else if (currentPage == m_choosePage) {
        //TODO the serial page will provite the connection that the uri provides
        qobject_cast<GenericPage*>(m_addPrinterPage->widget())->setValues(args);
        setCurrentPage(m_addPrinterPage);
    }
}

void AddPrinterAssistant::setCurrentPage(KPageWidgetItem *page)
{
    // if after setting the values the page is still valid show
    // it up, if not call next with it so we can find the next page
    if (qobject_cast<GenericPage*>(page->widget())->isValid()) {
        KAssistantDialog::setCurrentPage(page);
        GenericPage *currPage = qobject_cast<GenericPage*>(currentPage()->widget());
        GenericPage *nextPage = qobject_cast<GenericPage*>(page->widget());
        disconnect(currPage, SIGNAL(allowProceed(bool)), this, SLOT(enableNextButton(bool)));
        if (page == m_addPrinterPage) {
            enableNextButton(false);
            connect(nextPage, SIGNAL(allowProceed(bool)), this, SLOT(enableFinishButton(bool)));
            enableFinishButton(nextPage->canProceed());
        } else {
            connect(nextPage, SIGNAL(allowProceed(bool)), this, SLOT(enableNextButton(bool)));
            enableNextButton(nextPage->canProceed());
        }
    } else {
        next(page);
    }
}

void AddPrinterAssistant::slotButtonClicked(int button)
{
    if (button == KDialog::User1) {
        QHash<QString, QVariant> args = qobject_cast<GenericPage*>(currentPage()->widget())->values();
        kDebug() << args;
        QCups::Result *result;
        bool isClass = !args.take("add-new-printer").toBool();
        if (isClass) {
            result = QCups::addClass(args);
        } else {
            return;
        }
        result->waitTillFinished();
        if (result->hasError()) {
            kDebug() << result->lastError();
            KMessageBox::detailedSorry(this,
                                       isClass ? i18n("Failed to add class") : i18n("Failed to add printer"),
                                       result->lastErrorString(),
                                       i18n("Failed"));
        } else {
            KAssistantDialog::slotButtonClicked(button);
        }
        result->deleteLater();
    } else {
        KAssistantDialog::slotButtonClicked(button);
    }
}

void AddPrinterAssistant::enableNextButton(bool enable)
{
    enableButton(KDialog::User2, enable);
}

void AddPrinterAssistant::enableFinishButton(bool enable)
{
    enableButton(KDialog::User1, enable);
}

AddPrinterAssistant::~AddPrinterAssistant()
{
}

#include "AddPrinterAssistant.moc"
