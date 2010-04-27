/***************************************************************************
 *   Copyright (C) 2010 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
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

#include "PrintQueueInterface.h"
#include "printqueueadaptor.h"

#include "PrintQueueUi.h"
#include "QCups.h"
#include <cups/cups.h>

#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <KWindowSystem>

#include <KDebug>

PrintQueueInterface::PrintQueueInterface(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Creating Helper";
    (void) new PrintQueueAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.PrintQueue")) {
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

PrintQueueInterface::~PrintQueueInterface()
{
}

void PrintQueueInterface::ShowQueue(const QString &destName)
{
    if (destName.isEmpty()) {
        emit quit();
        return;
    }

    if(!m_uis.contains(destName)) {
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        cups_dest_t *dest = cupsGetDest(destName.toUtf8(), NULL, num_dests, dests);
        if (dest == NULL) {
            kWarning() << "Printer not found" << destName;
            cupsFreeDests(num_dests, dests);
            emit quit();
            return;
        }

        // store if the printer is a class
        const char *value;
        bool isClass = false;
        value = cupsGetOption("printer-type", dest->num_options, dest->options);
        if (value) {
            // the printer-type param is a flag
            isClass = QString::fromUtf8(value).toInt() & CUPS_PRINTER_CLASS;
        }
        cupsFreeDests(num_dests, dests);

        PrintQueueUi *ui = new PrintQueueUi(destName, isClass);
        connect(m_updateUi, SIGNAL(timeout()),
                ui, SLOT(update()));
        connect(ui, SIGNAL(finished()),
                this, SLOT(RemoveQueue()));
        ui->show();
        m_uis[destName] = ui;
    }
    KWindowSystem::forceActiveWindow(m_uis[destName]->winId());
}

bool PrintQueueInterface::canQuit()
{
    return m_uis.isEmpty();
}

void PrintQueueInterface::RemoveQueue()
{
    PrintQueueUi *ui = (PrintQueueUi*) sender();
    m_uis.remove(m_uis.key(ui));
}

#include "PrintQueueInterface.moc"
