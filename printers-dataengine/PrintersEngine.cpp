/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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
 
#include "PrintersEngine.h"

#include "PrintersService.h"
  
#include <Plasma/DataContainer>
#include <QStringBuilder>

#include <QDBusConnection>

#include <KCupsRequest.h>
#include <KCupsJob.h>
 
PrintersEngine::PrintersEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)
}

void PrintersEngine::init()
{
    m_printerAttributes |= KCupsPrinter::PrinterName;
    m_printerAttributes |= KCupsPrinter::PrinterInfo;
    m_printerAttributes |= KCupsPrinter::PrinterState;
    m_printerAttributes |= KCupsPrinter::PrinterStateMessage;
    // to get proper icons
    m_printerAttributes |= KCupsPrinter::PrinterType;

    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("QueueChanged"),
                                         this,
                                         SLOT(updatePrinter(QString)));

    // This is emitted when a printer is added
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterAdded"),
                                         this,
                                         SLOT(updatePrinter(QString)));

    // This is emitted when a printer is removed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterRemoved"),
                                         this,
                                         SLOT(removeSource(QString)));

    // Get all available printers
    getPrinters();
}

Plasma::Service* PrintersEngine::serviceForSource(const QString &source)
{
    return new PrintersService(this, source);
}

void PrintersEngine::getPrinters()
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(getPrintersFinished()));
    request->getPrinters(m_printerAttributes);
}

void PrintersEngine::getPrintersFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request) {
        return;
    }

    QStringList printersStrList;
    foreach (const KCupsPrinter &printer, request->printers()) {
        updatePrinterSource(printer);
        printersStrList << printer.name();
    }

    // Remove the printers that are not available anymore
    foreach (const QString &source, sources()) {
        if (!printersStrList.contains(source)) {
            removeSource(source);
        }
    }

    request->deleteLater();
}

void PrintersEngine::updatePrinterSource(const KCupsPrinter &printer)
{
    QString source = printer.name();
    Data sourceData = query(source);
    bool changed = false;

    if (sourceData[QLatin1String("printerName")] != printer.name()) {
        sourceData[QLatin1String("printerName")] = printer.name();
        changed = true;
    }
    if (sourceData[QLatin1String("info")] != printer.info()) {
        sourceData[QLatin1String("info")] = printer.info();
        changed = true;
    }
    QString state;
    switch (printer.state()) {
    case KCupsPrinter::Idle:
        state = QLatin1String("idle");
        break;
    case KCupsPrinter::Printing:
        state = QLatin1String("printing");
        break;
    case KCupsPrinter::Stoped:
        state = QLatin1String("stopped");
        break;
    default:
        state = QLatin1String("unknown");
    }
    if (sourceData[QLatin1String("stateEnum")] != state) {
        sourceData[QLatin1String("stateEnum")] = state;
        changed = true;
    }
    if (sourceData[QLatin1String("stateMessage")] != printer.stateMsg()) {
        sourceData[QLatin1String("stateMessage")] = printer.stateMsg();
        changed = true;
    }
    if (sourceData[QLatin1String("iconName")] != printer.iconName()) {
        sourceData[QLatin1String("iconName")] = printer.iconName();
        changed = true;
    }

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(source, sourceData);
    }
}

void PrintersEngine::updatePrinter(const QString &printer)
{
    KCupsPrinter::Attributes attr;
    attr |= KCupsPrinter::PrinterInfo;
    attr |= KCupsPrinter::PrinterType;
    attr |= KCupsPrinter::PrinterState;
    attr |= KCupsPrinter::PrinterStateMessage;

    KCupsRequest *request = new KCupsRequest;
    // TODO we set is class to false, but what if it was a class?
    request->getPrinterAttributes(printer, false, attr);
    connect(request, SIGNAL(finished()), this, SLOT(updatePrinterFinished()));
}

void PrintersEngine::updatePrinterFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request) {
        return;
    }
    if (request->hasError() || request->printers().isEmpty()) {
        // In case of an error force an update
        getPrinters();
    } else {
        // Add/Update our printer
        foreach (const KCupsPrinter &printer, request->printers()) {
            updatePrinterSource(printer);
        }
    }
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(plasma_engine_printers, PrintersEngine)
 
// this is needed since PrintersEngine is a QObject
#include "PrintersEngine.moc"
