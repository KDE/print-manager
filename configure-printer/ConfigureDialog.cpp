/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#include "ConfigureDialog.h"
#include "PrinterPage.h"

#include "ModifyPrinter.h"
#include "PrinterBehavior.h"
#include "PrinterOptions.h"

#include "KCupsRequest.h"

#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>

#include <QList>
#include <QPointer>

Q_DECLARE_METATYPE(QList<int>)

ConfigureDialog::ConfigureDialog(const QString &destName, bool isClass, QWidget *parent) :
    KPageDialog(parent)
{
    setFaceType(List);
    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    setWindowTitle(destName);
    setWindowIcon(KIcon("configure"));
    enableButtonApply(false);
    // Needed so we have our dialog size saved
    setAttribute(Qt::WA_DeleteOnClose);

    QStringList attr;
    KPageWidgetItem *page;

    modifyPrinter = new ModifyPrinter(destName, isClass, true, this);
    PrinterBehavior *printerBehavior = new PrinterBehavior(destName, isClass, this);
    attr << modifyPrinter->neededValues();
    attr << printerBehavior->neededValues();
    attr << KCUPS_PRINTER_TYPE; // needed to know if it's a remote printer
    attr << KCUPS_PRINTER_MAKE_AND_MODEL;

    KCupsPrinter printer;
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->getPrinterAttributes(destName, isClass, attr);
    request->waitTillFinished();
    if (!request) {
        return;
    }
    if (!request->hasError() && !request->printers().isEmpty()){
        printer = request->printers().first();
    }
//    kDebug() << "VALUES" << printer.a rgument();
//    kDebug() << "marker" << values["marker-levels"].value<QList<int> >();

    request->deleteLater();

    //     kDebug() << values;
    if (printer.type() & CUPS_PRINTER_LOCAL) {
        kDebug() << "CUPS_PRINTER_LOCAL";
    }
    isClass = printer.isClass();
    bool isRemote = false;
    if (printer.type() & CUPS_PRINTER_REMOTE) {
        kDebug() << "CUPS_PRINTER_REMOTE";
        isRemote = true;
    }
    if (printer.type() & CUPS_PRINTER_BW) {
        kDebug() << "CUPS_PRINTER_BW";
    }
    if (printer.type() & CUPS_PRINTER_COLOR) {
        kDebug() << "CUPS_PRINTER_COLOR";
    }
    if (printer.type() & CUPS_PRINTER_MFP) {
        kDebug() << "CUPS_PRINTER_MFP";
    }

    modifyPrinter->setRemote(isRemote);
    modifyPrinter->setValues(printer);
    page = new KPageWidgetItem(modifyPrinter, i18n("Modify Printer"));
    page->setHeader(i18n("Configure"));
    page->setIcon(KIcon("dialog-information"));
    // CONNECT this signal ONLY to the first Page
    connect(modifyPrinter, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    addPage(page);

    if (!isClass) {
        // At least on localhost:631 modify printer does not show printer options
        // for classes
        printerOptions = new PrinterOptions(destName, isClass, isRemote, this);
        page = new KPageWidgetItem(printerOptions, i18n("Printer Options"));
        page->setHeader(i18n("Set the Default Printer Options"));
        page->setIcon(KIcon("view-pim-tasks"));
        addPage(page);
        connect(modifyPrinter, SIGNAL(ppdChanged()), this, SLOT(ppdChanged()));
        modifyPrinter->setCurrentMake(printerOptions->currentMake());
        modifyPrinter->setCurrentMakeAndModel(printerOptions->currentMakeAndModel());
    }

    printerBehavior->setRemote(isRemote);
    printerBehavior->setValues(printer);
    page = new KPageWidgetItem(printerBehavior, i18n("Banners, Policies and Allowed Users"));
    page->setHeader(i18n("Banners, Policies and Allowed Users"));
    page->setIcon(KIcon("feed-subscribe"));
    addPage(page);

    // connect this after ALL pages were added, otherwise the slot will be called
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            SLOT(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)));

    KConfig config("print-manager");
    KConfigGroup configureDialog(&config, "ConfigureDialog");
    restoreDialogSize(configureDialog);
}

void ConfigureDialog::ppdChanged()
{
    printerOptions->reloadPPD();
    modifyPrinter->setCurrentMake(printerOptions->currentMake());
    modifyPrinter->setCurrentMakeAndModel(printerOptions->currentMakeAndModel());
}

ConfigureDialog::~ConfigureDialog()
{
    KConfig config("print-manager");
    KConfigGroup configureDialog(&config, "ConfigureDialog");
    saveDialogSize(configureDialog);
}

void ConfigureDialog::currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before)
{
    PrinterPage *currentPage = qobject_cast<PrinterPage*>(current->widget());
    PrinterPage *beforePage = qobject_cast<PrinterPage*>(before->widget());

    // Check if the before page has changes
    savePage(beforePage);
    if (beforePage) {
        disconnect(beforePage, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    }

    // connect the changed signal to the new page and check if it has changes
    connect(currentPage, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    enableButtonApply(currentPage->hasChanges());
}

void ConfigureDialog::slotButtonClicked(int button)
{
    PrinterPage *page = qobject_cast<PrinterPage *>(currentPage()->widget());
    if (button == KDialog::Ok) {
        page->save();
        accept();
    } else if (button == KDialog::Apply) {
        page->save();
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void ConfigureDialog::closeEvent(QCloseEvent *event)
{
    PrinterPage *page = qobject_cast<PrinterPage*>(currentPage()->widget());
    if (savePage(page)) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool ConfigureDialog::savePage(PrinterPage *page)
{
    if (page->hasChanges()) {
        int ret;
        ret = KMessageBox::warningYesNoCancel(this,
                                               i18n("The current page has changes.\n"
                                                    "Do you want to save them?"));
        if (ret == KMessageBox::Yes) {
            page->save();
        } else if (ret == KMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}
