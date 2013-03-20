/***************************************************************************
 *   Copyright (C) 2010-2012 Daniel Nicoletti <dantti12@gmail.com>         *
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
#include <KCupsRequest.h>
#include <KCupsPrinter.h>

#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <KWindowSystem>
#include <KDialog>

#include <KDebug>

ConfigurePrinterInterface::ConfigurePrinterInterface(QObject *parent) :
    QObject(parent)
{
    kDebug() << "Creating Helper";
    (void) new ConfigurePrinterAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.ConfigurePrinter")) {
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

ConfigurePrinterInterface::~ConfigurePrinterInterface()
{
}

void ConfigurePrinterInterface::ConfigurePrinter(const QString &destName)
{
    if (!m_uis.contains(destName)) {
        // Reserve this since the CUPS call might take a long time
        m_uis[destName] = 0;

        QStringList att;
        att << KCUPS_PRINTER_NAME;
        att << KCUPS_PRINTER_TYPE;
        // Get destinations with these attributes
        QPointer<KCupsRequest> request = new KCupsRequest;
        request->getPrinters(att);
        request->waitTillFinished();
        if (!request) {
            return;
        }

        bool found = false;
        KCupsPrinter printer;
        KCupsPrinters printers = request->printers();
        for (int i = 0; i < printers.size(); i++) {
            if (printers.at(i).name() == destName) {
                printer = printers.at(i);
                found = true;
                break;
            }
        }
        request->deleteLater();

        if (found) {
            ConfigureDialog *ui = new ConfigureDialog(printer.name(), printer.isClass());
            connect(m_updateUi, SIGNAL(timeout()),
                    ui, SLOT(update()));
            connect(ui, SIGNAL(finished()),
                    this, SLOT(RemovePrinter()));
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
    QWidget *ui = qobject_cast<QWidget*>(sender());
    if (ui) {
        m_uis.remove(m_uis.key(ui));
    }

    // if no destination was found and we aren't showing
    // a queue quit the app
    if (m_uis.isEmpty()) {
         emit quit();
    }
}

#include "ConfigurePrinterInterface.moc"
