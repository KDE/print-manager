/***************************************************************************
 *   Copyright (C) 2010-2018 Daniel Nicoletti <dantti12@gmail.com>         *
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

#include "ConfigurePrinterInterface.h"
#include "configureprinteradaptor.h"

#include "ConfigureDialog.h"
#include "Debug.h"

#include <KCupsRequest.h>
#include <KCupsPrinter.h>

#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <KWindowSystem>
#include <QDialog>

ConfigurePrinterInterface::ConfigurePrinterInterface(QObject *parent) :
    QObject(parent)
{
    qCDebug(PM_CONFIGURE_PRINTER) << "Creating Helper";
    (void) new ConfigurePrinterAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.ConfigurePrinter"))) {
        qCDebug(PM_CONFIGURE_PRINTER) << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject(QLatin1String("/"), this)) {
        qCDebug(PM_CONFIGURE_PRINTER) << "unable to register service interface to dbus";
        return;
    }

    // setup the timer that updates the UIs
    m_updateUi = new QTimer(this);
    m_updateUi->setInterval(1000);
    m_updateUi->start();
}

ConfigurePrinterInterface::~ConfigurePrinterInterface()
{
}

void ConfigurePrinterInterface::ConfigurePrinter(const QString &destName)
{
    if (!m_uis.contains(destName)) {
        // Reserve this since the CUPS call might take a long time
        m_uis[destName] = nullptr;

        // Get destinations with these attributes
        QPointer<KCupsRequest> request = new KCupsRequest;
        request->getPrinters({ KCUPS_PRINTER_NAME, KCUPS_PRINTER_TYPE });
        request->waitTillFinished();
        if (!request) {
            return;
        }

        bool found = false;
        KCupsPrinter printer;
        const KCupsPrinters printers = request->printers();
        for (const KCupsPrinter &p : printers) {
            if (p.name() == destName) {
                printer = p;
                found = true;
                break;
            }
        }
        request->deleteLater();

        if (found) {
            auto ui = new ConfigureDialog(printer.name(), printer.isClass());
            connect(m_updateUi, &QTimer::timeout, ui, static_cast<void(ConfigureDialog::*)()>(&ConfigureDialog::update));
            connect(ui, &ConfigureDialog::finished, this, &ConfigurePrinterInterface::RemovePrinter);
            ui->show();
            m_uis[printer.name()] = ui;
        } else {
            // Remove the reservation
            m_uis.remove(destName);

            // if no destination was found and we aren't showing
            // a queue quit the app
            if (m_uis.isEmpty()) {
                 emit quit();
            }
            return;
        }
    }

    // Check it it's not reserved
    if (m_uis.value(destName)) {
        KWindowSystem::forceActiveWindow(m_uis.value(destName)->winId());
    }
}

void ConfigurePrinterInterface::RemovePrinter()
{
    auto ui = qobject_cast<QWidget*>(sender());
    if (ui) {
        m_uis.remove(m_uis.key(ui));
    }

    // if no destination was found and we aren't showing
    // a queue quit the app
    if (m_uis.isEmpty()) {
         emit quit();
    }
}

#include "moc_ConfigurePrinterInterface.cpp"
