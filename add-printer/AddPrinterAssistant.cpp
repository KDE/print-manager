/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

#include <KCupsRequest.h>

#include <QHostInfo>
#include <QPushButton>

#include <KLocalizedString>
#include <KPixmapSequence>
#include <KIconLoader>
#include <KSharedConfig>
#include <KWindowConfig>

AddPrinterAssistant::AddPrinterAssistant() :
    KAssistantDialog(),
    m_devicesPage(0),
    m_chooseClassPage(0),
    m_choosePPDPage(0),
    m_addPrinterPage(0)
{
    setWindowTitle(i18nc("@title:window", "Add a New Printer"));
    setWindowIcon(QIcon::fromTheme("printer"));
    buttonBox()->removeButton(buttonBox()->button(QDialogButtonBox::Cancel));
    // Needed so we have our dialog size saved
    setAttribute(Qt::WA_DeleteOnClose);

    QPushButton * helpButton = buttonBox()->addButton(QDialogButtonBox::Help);
    // Configure the help button to be flat, disabled and empty
    helpButton->setFlat(true);
    helpButton->setEnabled(false);
    helpButton->setIcon(QIcon());
    helpButton->setText(QString());

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KIconLoader::global()->loadPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_busySeq->setWidget(helpButton);

    connect(finishButton(), &QPushButton::clicked, this, &AddPrinterAssistant::slotFinishButtonClicked);

    // Restore the dialog size
    KConfigGroup configGroup(KSharedConfig::openConfig("print-manager"), "AddPrinterAssistant");
    KWindowConfig::restoreWindowSize(windowHandle(), configGroup);
}

AddPrinterAssistant::~AddPrinterAssistant()
{
    KConfigGroup configGroup(KSharedConfig::openConfig("print-manager"), "AddPrinterAssistant");
    KWindowConfig::saveWindowSize(windowHandle(), configGroup);
}

void AddPrinterAssistant::initAddPrinter(const QString &printer, const QString &deviceId)
{
    // setup our hash args with the information if we are
    // adding a new printer or a class
    QVariantHash args;
    args[ADDING_PRINTER] = true;

    KPageWidgetItem *currentPage;
    if (deviceId.isEmpty()) {
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
        disconnect(currPage, &GenericPage::allowProceed, this, &AddPrinterAssistant::enableNextButton);
        disconnect(currPage, &GenericPage::allowProceed, this, &AddPrinterAssistant::enableFinishButton);
        disconnect(currPage, &GenericPage::proceed, this, static_cast<void(AddPrinterAssistant::*)()>(&AddPrinterAssistant::next));
        disconnect(currPage, &GenericPage::startWorking, m_busySeq, &KPixmapSequenceOverlayPainter::start);
        disconnect(currPage, &GenericPage::stopWorking, m_busySeq, &KPixmapSequenceOverlayPainter::stop);

        // Connect next page signals
        connect(nextPage, &GenericPage::startWorking, m_busySeq, &KPixmapSequenceOverlayPainter::start);
        connect(nextPage, &GenericPage::stopWorking, m_busySeq, &KPixmapSequenceOverlayPainter::stop);
        connect(nextPage, &GenericPage::proceed, this, static_cast<void(AddPrinterAssistant::*)()>(&AddPrinterAssistant::next));

        // check the working property
        if (nextPage->isWorking()) {
            m_busySeq->start();
        } else {
            m_busySeq->stop();
        }

        // When ChangePPD() is called addPrinterPage is zero
        if (page == m_addPrinterPage || m_addPrinterPage == 0) {
            connect(nextPage, &GenericPage::allowProceed, this, &AddPrinterAssistant::enableFinishButton);
            enableNextButton(false);
            enableFinishButton(nextPage->canProceed());
        } else {
            connect(nextPage, &GenericPage::allowProceed, this, &AddPrinterAssistant::enableNextButton);
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

void AddPrinterAssistant::slotFinishButtonClicked()
{
    GenericPage *page = qobject_cast<GenericPage*>(currentPage()->widget());
    enableFinishButton(false);
    if (page->finishClicked()) {
        //KAssistantDialog::slotButtonClicked(button); // FIXME next() really?
        next();
    } else {
        enableFinishButton(true);
    }
}

void AddPrinterAssistant::enableNextButton(bool enable)
{
    nextButton()->setEnabled(enable);
}

void AddPrinterAssistant::enableFinishButton(bool enable)
{
    finishButton()->setEnabled(enable);
}
