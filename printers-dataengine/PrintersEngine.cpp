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
 
PrintersEngine::PrintersEngine(QObject *parent, const QVariantList &args) :
    Plasma::DataEngine(parent, args),
    m_subscriptionId(-1)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    m_printerAttributes |= KCupsPrinter::PrinterName;
    m_printerAttributes |= KCupsPrinter::PrinterInfo;
    m_printerAttributes |= KCupsPrinter::PrinterState;
    m_printerAttributes |= KCupsPrinter::PrinterStateMessage;
    // to get proper icons
    m_printerAttributes |= KCupsPrinter::PrinterType;

    createSubscription();
}

PrintersEngine::~PrintersEngine()
{
    KCupsRequest *request = new KCupsRequest;
    request->cancelDBusSubscription(m_subscriptionId);
}

void PrintersEngine::init()
{
    // This is emitted when a printer is added
    connect(KCupsConnection::global(),
            SIGNAL(printerAdded(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(),
            SIGNAL(printerModified(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(),
            SIGNAL(printerStateChanged(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(),
            SIGNAL(printerStopped(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(),
            SIGNAL(printerRestarted(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(),
            SIGNAL(printerShutdown(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(insertUpdatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(),
            SIGNAL(printerDeleted(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(printerRemoved(QString,QString,QString,uint,QString,bool)));

    // Get all available printers
    getPrinters();
}

Plasma::Service* PrintersEngine::serviceForSource(const QString &source)
{
    return new PrintersService(this, source);
}

void PrintersEngine::createSubscription()
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(createSubscriptionFinished()));
    QStringList events;
    events << "printer-added";
    events << "printer-deleted";
    events << "printer-state-changed";
    events << "printer-modified";
    request->createDBusSubscription(events);
}

void PrintersEngine::createSubscriptionFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request || request->hasError() || request->subscriptionId() < 0) {
        // in case of an error probe the server again in 1.5 seconds
        QTimer::singleShot(1000, this, SLOT(createSubscription()));
        request->deleteLater();
        m_subscriptionId = -1;
        return;
    }

    m_subscriptionId = request->subscriptionId();
    request->deleteLater();
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
    if (!request || request->hasError()) {
        // in case of an error probe the server again in 1.5 seconds
        QTimer::singleShot(1500, this, SLOT(getPrinters()));
        request->deleteLater();;
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

void PrintersEngine::insertUpdatePrinter(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    // REALLY? all these parameters just to say foo was added??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)

    KCupsPrinter::Attributes attr;
    attr |= KCupsPrinter::PrinterInfo;
    attr |= KCupsPrinter::PrinterType;
    attr |= KCupsPrinter::PrinterState;
    attr |= KCupsPrinter::PrinterStateMessage;

    KCupsRequest *request = new KCupsRequest;
    // TODO we set is class to false, but what if it was a class?
    request->getPrinterAttributes(printerName, false, attr);
    connect(request, SIGNAL(finished()), this, SLOT(insertUpdatePrinterFinished()));
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

void PrintersEngine::insertUpdatePrinterFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request) {
        return;
    }
    if (request->hasError() || request->printers().isEmpty()) {
        // In case of an error force an update of all printers
        getPrinters();
    } else {
        // Add/Update our printer
        foreach (const KCupsPrinter &printer, request->printers()) {
            updatePrinterSource(printer);
        }
    }
    request->deleteLater();;
}

void PrintersEngine::printerRemoved(const QString &text,
                                    const QString &printerUri,
                                    const QString &printerName,
                                    uint printerState,
                                    const QString &printerStateReasons,
                                    bool printerIsAcceptingJobs)
{
    // REALLY? all these parameters just to say foo was deleted??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)

    // Remove the printer source
    removeSource(printerName);
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(plasma_engine_printers, PrintersEngine)
 
// this is needed since PrintersEngine is a QObject
#include "PrintersEngine.moc"
