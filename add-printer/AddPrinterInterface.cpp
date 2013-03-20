/***************************************************************************
 *   Copyright (C) 2010 Daniel Nicoletti <dantti12@gmail.com>              *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "AddPrinterInterface.h"
#include "addprinteradaptor.h"

#include "AddPrinterAssistant.h"

#include <KCupsRequest.h>

#include <QApplication>
#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <KWindowSystem>
#include <KDialog>

#include <KDebug>

AddPrinterInterface::AddPrinterInterface(QObject *parent) :
    QObject(parent)
{
    kDebug() << "Creating Helper";
    (void) new AddPrinterAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.AddPrinter")) {
        kDebug() << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kDebug() << "unable to register service interface to dbus";
        return;
    }

    // setup the timer that updates the UIs
    m_updateUi = new QTimer(this);
    m_updateUi->setInterval(1000);
    m_updateUi->start();
}

AddPrinterInterface::~AddPrinterInterface()
{
}

void AddPrinterInterface::AddPrinter(qulonglong wid)
{
    AddPrinterAssistant *wizard = new AddPrinterAssistant();
    wizard->initAddPrinter();
    show(wizard, wid);
}

void AddPrinterInterface::AddClass(qulonglong wid)
{
    AddPrinterAssistant *wizard = new AddPrinterAssistant();
    wizard->initAddClass();
    show(wizard, wid);
}

void AddPrinterInterface::ChangePPD(qulonglong wid, const QString &name)
{
    // Fist we need to get the printer attributes
    QPointer<KCupsRequest> request = new KCupsRequest;
    QStringList attr;
    attr << KCUPS_PRINTER_TYPE; // needed to know if it's a remote printer
    attr << KCUPS_PRINTER_MAKE_AND_MODEL;
    attr << KCUPS_DEVICE_URI;
    request->getPrinterAttributes(name, false, attr);
    request->waitTillFinished();
    if (request) {
        if (!request->hasError() && request->printers().size() == 1) {
            KCupsPrinter printer = request->printers().first();
            if (printer.type() & CUPS_PRINTER_REMOTE) {
                kWarning() << "Ignoring request, can not change PPD of remote printer" << name;
            } else {
                AddPrinterAssistant *wizard = new AddPrinterAssistant();
                wizard->initChangePPD(name, printer.deviceUri(), printer.makeAndModel());
                show(wizard, wid);
            }
        } else {
            kWarning() << "Ignoring request, printer not found" << name << request->errorMsg();
        }
        request->deleteLater();
    }
}

void AddPrinterInterface::NewPrinterFromDevice(qulonglong wid, const QString &name, const QString &device_id)
{
    AddPrinterAssistant *wizard = new AddPrinterAssistant();
    wizard->initAddPrinter(name, device_id);
    show(wizard, wid);
}

void AddPrinterInterface::RemoveQueue()
{
    QWidget *ui = qobject_cast<QWidget*>(sender());
    m_uis.remove(m_uis.key(ui));
}

void AddPrinterInterface::show(QWidget *widget, qulonglong wid) const
{
//    connect(QApplication::instance(), SIGNAL(),
//            widget, SLOT();
    widget->show();
    KWindowSystem::forceActiveWindow(widget->winId());
    KWindowSystem::setMainWindow(widget, wid);
}
