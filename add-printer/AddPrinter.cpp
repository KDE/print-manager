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

#include "AddPrinter.h"

#include "AddPrinterAssistant.h"

#include <KCupsRequest.h>
#include <KWindowSystem>

#include <QPointer>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(PM_ADD_PRINTER)

AddPrinter::AddPrinter(int &argc, char **argv) :
    QApplication(argc, argv)
{
    setQuitOnLastWindowClosed(true);
}

AddPrinter::~AddPrinter()
{
}

void AddPrinter::addPrinter(qulonglong wid)
{
    auto wizard = new AddPrinterAssistant();
    wizard->initAddPrinter();
    show(wizard, wid);
}

void AddPrinter::addClass(qulonglong wid)
{
    auto wizard = new AddPrinterAssistant();
    wizard->initAddClass();
    show(wizard, wid);
}

void AddPrinter::changePPD(qulonglong wid, const QString &name)
{
    // Fist we need to get the printer attributes
    QPointer<KCupsRequest> request = new KCupsRequest;
    const QStringList attr({
                               KCUPS_PRINTER_TYPE, // needed to know if it's a remote printer
                               KCUPS_PRINTER_MAKE_AND_MODEL,
                               KCUPS_DEVICE_URI
                           });
    request->getPrinterAttributes(name, false, attr);
    request->waitTillFinished();
    if (request) {
        if (!request->hasError() && request->printers().size() == 1) {
            const KCupsPrinter printer = request->printers().first();
            if (printer.type() & CUPS_PRINTER_REMOTE) {
                qCWarning(PM_ADD_PRINTER) << "Ignoring request, can not change PPD of remote printer" << name;
            } else {
                auto wizard = new AddPrinterAssistant();
                wizard->initChangePPD(name, printer.deviceUri(), printer.makeAndModel());
                show(wizard, wid);
            }
        } else {
            qWarning() << "Ignoring request, printer not found" << name << request->errorMsg();
        }
        request->deleteLater();
    }
}

void AddPrinter::newPrinterFromDevice(qulonglong wid, const QString &name, const QString &device_id)
{
    // Example of data
    // "direct"
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    // "Samsung SCX-4200 Series"
    // "Samsung SCX-4200 Series"
    // "usb://Samsung/SCX-4200%20Series"
    // ""

    //        printer = "Samsung SCX-3400 Series";
    //        deviceId = "MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;STATUS:BUSY;";

    //        printer = "Samsung SCX-4200 Series";
    //        deviceId = "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;";

    //        printer = "HP PSC 1400 series";
    //        deviceId = "MFG:HP;MDL:PSC 1400 series;DES:;CMD:LDL,MLC,PML,DYN;";

    auto wizard = new AddPrinterAssistant();
    wizard->initAddPrinter(name, device_id);
    show(wizard, wid);
}

void AddPrinter::show(QWidget *widget, qulonglong wid) const
{
    widget->show();
    KWindowSystem::forceActiveWindow(widget->winId());
    KWindowSystem::setMainWindow(widget, wid);
}

#include "moc_AddPrinter.cpp"
