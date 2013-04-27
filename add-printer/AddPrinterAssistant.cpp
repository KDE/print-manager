/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti12@gmail.com                                                    *
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

#include "PageDestinations.h"
#include "PageChoosePrinters.h"
#include "PageChoosePPD.h"
#include "PageAddPrinter.h"

#include <QHostInfo>

#include <KCupsRequest.h>

#include <KLocale>
#include <KPushButton>
#include <KPixmapSequence>

#include <KDebug>

AddPrinterAssistant::AddPrinterAssistant() :
    KAssistantDialog(),
    m_devicesPage(0),
    m_chooseClassPage(0),
    m_choosePPDPage(0),
    m_addPrinterPage(0)
{
    setWindowTitle(i18nc("@title:window", "Add a New Printer"));
    setWindowIcon(KIcon("printer"));
    showButton(KDialog::Cancel, false);
    setDefaultButton(KDialog::User2); // next
    setDefaultButton(KDialog::User1); // finished
    // Needed so we have our dialog size saved
    setAttribute(Qt::WA_DeleteOnClose);

    // Configure the help button to be flat, disabled and empty
    button(KDialog::Help)->setFlat(true);
    button(KDialog::Help)->setEnabled(false);
    button(KDialog::Help)->setIcon(QIcon());
    button(KDialog::Help)->setText(QString());

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_busySeq->setWidget(button(KDialog::Help));

    // Restore the dialog size
    KConfig config("print-manager");
    KConfigGroup configGroup(&config, "AddPrinterAssistant");
    restoreDialogSize(configGroup);
}

AddPrinterAssistant::~AddPrinterAssistant()
{
    KConfig config("print-manager");
    KConfigGroup configGroup(&config, "AddPrinterAssistant");
    saveDialogSize(configGroup);
}

void AddPrinterAssistant::initAddPrinter(const QString &printer, const QString &deviceId)
{
    // setup our hash args with the information if we are
    // adding a new printer or a class
    QVariantHash args;
    args[ADDING_PRINTER] = true;

    KPageWidgetItem *currentPage;
    if (deviceId.isNull()) {
        m_devicesPage = new KPageWidgetItem(new PageDestinations(args), i18nc("@title:window", "Select a Printer to Add"));
        addPage(m_devicesPage);
        currentPage = m_devicesPage;

        m_choosePPDPage = new KPageWidgetItem(new PageChoosePPD, i18nc("@title:window", "Pick a Driver"));
        addPage(m_choosePPDPage);
    } else {
        args[KCUPS_DEVICE_URI] = printer;
        args[KCUPS_DEVICE_ID] = deviceId;
        args[KCUPS_DEVICE_LOCATION] = QHostInfo::localHostName();

        m_choosePPDPage = new KPageWidgetItem(new PageChoosePPD(args), i18nc("@title:window", "Pick a Driver"));
        addPage(m_choosePPDPage);
        currentPage = m_choosePPDPage;
    }

    m_addPrinterPage = new KPageWidgetItem(new PageAddPrinter, i18nc("@title:window", "Please describe you printer"));
    addPage(m_addPrinterPage);

    // Set this later so that all m_*Pages are created
    setCurrentPage(currentPage);
}

void AddPrinterAssistant::initAddClass()
{
    // setup our hash args with the information if we are
    // adding a new printer or a class
    QVariantHash args;
    args[ADDING_PRINTER] = false;
    args[KCUPS_DEVICE_LOCATION] = QHostInfo::localHostName();

    KPageWidgetItem *currentPage;
    m_chooseClassPage = new KPageWidgetItem(new PageChoosePrinters(args), i18nc("@title:window", "Configure your connection"));
    addPage(m_chooseClassPage);
    currentPage = m_chooseClassPage;

    m_addPrinterPage = new KPageWidgetItem(new PageAddPrinter, i18nc("@title:window", "Please describe you printer"));
    addPage(m_addPrinterPage);

    // Set this later so that all m_*Pages are created
    setCurrentPage(currentPage);
}

void AddPrinterAssistant::initChangePPD(const QString &printer, const QString &deviceUri, const QString &makeAndModel)
{
    // setup our hash args with the information if we are
    // adding a new printer or a class
    QVariantHash args;
    args[ADDING_PRINTER] = true;
    args[KCUPS_DEVICE_URI] = deviceUri;
    args[KCUPS_PRINTER_NAME] = printer;
    args[KCUPS_PRINTER_MAKE_AND_MODEL] = makeAndModel;

    m_choosePPDPage = new KPageWidgetItem(new PageChoosePPD(args), i18nc("@title:window", "Pick a Driver"));
    addPage(m_choosePPDPage);
    setCurrentPage(m_choosePPDPage);
}

void AddPrinterAssistant::back()
{
    KAssistantDialog::back();
    GenericPage *currPage;
    currPage = qobject_cast<GenericPage*>(currentPage()->widget());
    enableNextButton(currPage->canProceed());
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
    QVariantHash args = qobject_cast<GenericPage*>(currentPage->widget())->values();
    if (currentPage == m_devicesPage) {
        qobject_cast<GenericPage*>(m_choosePPDPage->widget())->setValues(args);
        setCurrentPage(m_choosePPDPage);
    } else if (currentPage == m_chooseClassPage ||currentPage == m_choosePPDPage) {
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
        // Disconnect the current page slots
        disconnect(currPage, SIGNAL(allowProceed(bool)), this, SLOT(enableNextButton(bool)));
        disconnect(currPage, SIGNAL(allowProceed(bool)), this, SLOT(enableFinishButton(bool)));
        disconnect(currPage, SIGNAL(startWorking()), m_busySeq, SLOT(start()));
        disconnect(currPage, SIGNAL(stopWorking()), m_busySeq, SLOT(stop()));
        disconnect(currPage, SIGNAL(proceed()), this, SLOT(next()));

        // Connect next page signals
        connect(currPage, SIGNAL(startWorking()), m_busySeq, SLOT(start()));
        connect(currPage, SIGNAL(stopWorking()), m_busySeq, SLOT(stop()));
        connect(nextPage, SIGNAL(proceed()), this, SLOT(next()));

        // check the working property
        if (nextPage->isWorking()) {
            m_busySeq->start();
        } else {
            m_busySeq->stop();
        }

        // When ChangePPD() is called addPrinterPage is zero
        if (page == m_addPrinterPage || m_addPrinterPage == 0) {
            connect(nextPage, SIGNAL(allowProceed(bool)), this, SLOT(enableFinishButton(bool)));
            enableNextButton(false);
            enableFinishButton(nextPage->canProceed());
        } else {
            connect(nextPage, SIGNAL(allowProceed(bool)), this, SLOT(enableNextButton(bool)));
            enableNextButton(nextPage->canProceed());
        }
    } else {
        // In case page is not valid try the next one
        next(page);
    }
}

void AddPrinterAssistant::showEvent(QShowEvent *event)
{
    KAssistantDialog::showEvent(event);
    enableNextButton(false);
    enableFinishButton(false);
}

void AddPrinterAssistant::slotButtonClicked(int button)
{
    // Finish Button
    if (button == KDialog::User1) {
        GenericPage *page = qobject_cast<GenericPage*>(currentPage()->widget());
        enableFinishButton(false);
        if (page->finishClicked()) {
            KAssistantDialog::slotButtonClicked(button);
        } else {
            enableFinishButton(true);
        }
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
