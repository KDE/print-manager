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

#include "PrintQueue.h"

#include "PrintQueueUi.h"

#include <KCupsRequest.h>

#include <QPointer>
#include <QTimer>

#include <KWindowSystem>
#include <KCmdLineArgs>
#include <KDebug>

PrintQueue::PrintQueue() :
    KUniqueApplication()
{
}

int PrintQueue::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count()) {
        for (int i = 0; i < args->count(); ++i) {
            showQueue(args->arg(i));
        }
    } else {
        kDebug() << "called with no args";
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, this, SLOT(removeQueue()));
    }

    return 0;
}

PrintQueue::~PrintQueue()
{
}

void PrintQueue::showQueue(const QString &destName)
{
    kDebug() << destName;
    if (!m_uis.contains(destName)) {
        // Reserve this since the CUPS call might take a long time
        m_uis[destName] = 0;

        QStringList attr;
        attr << KCUPS_PRINTER_NAME;
        attr << KCUPS_PRINTER_TYPE;
        // Get destinations with these attributes
        QPointer<KCupsRequest> request = new KCupsRequest;
        request->getPrinters(attr);
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
            PrintQueueUi *ui = new PrintQueueUi(printer);
            connect(ui, SIGNAL(finished()),
                    this, SLOT(removeQueue()));
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

void PrintQueue::removeQueue()
{
    QWidget *ui = qobject_cast<QWidget*>(sender());
    if (ui) {
        m_uis.remove(m_uis.key(ui));
    }

    // if no destination was found and we aren't showing
    // a queue quit the app
    if (m_uis.isEmpty()) {
         quit();
    }
}


#include "PrintQueue.moc"
