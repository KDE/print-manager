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

#include "PrinterModel.h"

#include <QDateTime>
#include <QMimeData>
#include <QDBusConnection>
#include <QtDBus/QDBusInterface>

#include <KUser>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <KCupsRequest.h>

#include <cups/cups.h>

PrinterModel::PrinterModel(WId parentId, QObject *parent) :
    QStandardItemModel(parent),
    m_parentId(parentId)
{
    m_attributes << KCUPS_PRINTER_NAME;
    m_attributes << KCUPS_PRINTER_STATE;
    m_attributes << KCUPS_PRINTER_STATE_MESSAGE;
    m_attributes << KCUPS_PRINTER_IS_SHARED;
    m_attributes << KCUPS_PRINTER_IS_ACCEPTING_JOBS;
    m_attributes << KCUPS_PRINTER_TYPE;
    m_attributes << KCUPS_PRINTER_LOCATION;
    m_attributes << KCUPS_PRINTER_INFO;
    m_attributes << KCUPS_PRINTER_MAKE_AND_MODEL;
    m_attributes << KCUPS_PRINTER_COMMANDS;
    m_attributes << KCUPS_MARKER_CHANGE_TIME;
    m_attributes << KCUPS_MARKER_COLORS;
    m_attributes << KCUPS_MARKER_LEVELS;
    m_attributes << KCUPS_MARKER_NAMES;
    m_attributes << KCUPS_MARKER_TYPES;

    QStringList events;
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

    // Deprecated stuff that works better than the above
    connect(KCupsConnection::global(), SIGNAL(rhPrinterAdded(QString)),
            this, SLOT(insertUpdatePrinter(QString)));

    connect(KCupsConnection::global(), SIGNAL(rhPrinterRemoved(QString)),
            this, SLOT(printerRemoved(QString)));

    connect(KCupsConnection::global(), SIGNAL(rhQueueChanged(QString)),
            this, SLOT(insertUpdatePrinter(QString)));

    update();
}

void PrinterModel::getDestsFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    if (request) {
        if (request->hasError()) {
            emit error(request->error(), request->serverError(), request->errorMsg());
            if (request->error() == IPP_SERVICE_UNAVAILABLE) {
                // Check if the service is up again
                QTimer::singleShot(1000, this, SLOT(update()));
            }
            // clear the model after so that the proper widget can be shown
            clear();
        } else {
            KCupsPrinters printers = request->printers();
            for (int i = 0; i < printers.size(); ++i) {
                // If there is a printer and it's not the current one add it
                // as a new destination
                int dest_row = destRow(printers.at(i).name());
                if (dest_row == -1) {
                    // not found, insert new one
                    insertDest(i, printers.at(i));
                } else if (dest_row == i) {
                    // update the printer
                    updateDest(item(i), printers.at(i));
                } else {
                    // found at wrong position
                    // take it and insert on the right position
                    QList<QStandardItem *> row = takeRow(dest_row);
                    insertRow(i, row);
                    updateDest(item(i), printers.at(i));
                }
            }

            // remove old printers
            // The above code starts from 0 and make sure
            // dest == modelIndex(x) and if it's not the
            // case it either inserts or moves it.
            // so any item > num_jobs can be safely deleted
            while (rowCount() > printers.size()) {
                removeRow(rowCount() - 1);
            }

            emit error(IPP_OK, QString(), QString());
        }
        request->deleteLater();
    } else {
        kWarning() << "Should not be called from a non KCupsRequest class" << sender();
    }
}

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

void PrinterModel::update()
{
//                 kcmshell(6331) PrinterModel::update: (QHash(("printer-type", QVariant(int, 75534348) ) ( "marker-names" ,  QVariant(QStringList, ("Cyan", "Yellow", "Magenta", "Black") ) ) ( "printer-name" ,  QVariant(QString, "EPSON_Stylus_TX105") ) ( "marker-colors" ,  QVariant(QStringList, ("#00ffff", "#ffff00", "#ff00ff", "#000000") ) ) ( "printer-location" ,  QVariant(QString, "Luiz Vitor’s MacBook Pro") ) ( "marker-levels" ,  QVariant(QList<int>, ) ) ( "marker-types" ,  QVariant(QStringList, ("inkCartridge", "inkCartridge", "inkCartridge", "inkCartridge") ) ) ( "printer-is-shared" ,  QVariant(bool, true) ) ( "printer-state-message" ,  QVariant(QString, "") ) ( "printer-commands" ,  QVariant(QStringList, ("Clean", "PrintSelfTestPage", "ReportLevels") ) ) ( "marker-change-time" ,  QVariant(int, 1267903160) ) ( "printer-state" ,  QVariant(int, 3) ) ( "printer-info" ,  QVariant(QString, "EPSON Stylus TX105") ) ( "printer-make-and-model" ,  QVariant(QString, "EPSON TX105 Series") ) )  )
    // Get destinations with these attributes
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(getDestsFinished()));
    request->getPrinters(m_attributes);
}

void PrinterModel::insertDest(int pos, const KCupsPrinter &printer)
{
    // Create the printer item
    QStandardItem *stdItem = new QStandardItem(printer.name());
    stdItem->setData(printer.name(), DestName);
    stdItem->setIcon(printer.icon());
    // update the item
    updateDest(stdItem, printer);

    // insert the printer Item
    insertRow(pos, stdItem);
}

void PrinterModel::updateDest(QStandardItem *destItem, const KCupsPrinter &printer)
{
    // store if the printer is the network default
    bool isDefault = printer.isDefault();
    if (isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    QString status = destStatus(printer.state(), printer.stateMsg());
    if (status != destItem->data(DestStatus)) {
        destItem->setData(status, DestStatus);
    }

    // store if the printer is shared
    bool shared = printer.isShared();
    if (shared != destItem->data(DestIsShared)) {
        destItem->setData(shared, DestIsShared);
    }

    // store if the printer is accepting jobs
    bool accepting = printer.isAcceptingJobs();
    if (accepting != destItem->data(DestIsAcceptingJobs)) {
        destItem->setData(accepting, DestIsAcceptingJobs);
    }

    // store if the printer is a class
    // the printer-type param is a flag
    bool isClass = printer.isClass();
    if (isClass != destItem->data(DestIsClass)) {
        destItem->setData(isClass, DestIsClass);
    }

    // store if the printer type
    // the printer-type param is a flag
    uint printerType = printer.type();
    if (printerType != destItem->data(DestType)) {
        destItem->setData(printerType, DestType);
    }

    // store the printer location
    QString location = printer.location();
    if (location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    if (destItem->data(DestName).toString() != destItem->text()){
        if (destItem->text() != destItem->data(DestName).toString()){
            destItem->setText(destItem->data(DestName).toString());
        }
    }

    // store the printer description
    QString description = printer.info();
    if (description != destItem->data(DestDescription).toString()){
        destItem->setData(description, DestDescription);
    }

    // store the printer kind
    QString kind = printer.makeAndModel();
    if (kind != destItem->data(DestKind)) {
        destItem->setData(kind, DestKind);
    }

    // store the printer commands
    QStringList commands = printer.commands();
    if (commands != destItem->data(DestCommands)) {
        destItem->setData(commands, DestCommands);
    }

    int markerChangeTime = printer.markerChangeTime();
    if (markerChangeTime != destItem->data(DestMarkerChangeTime)) {
        destItem->setData(printer.markerChangeTime(), DestMarkerChangeTime);
        QVariantHash markers;
        markers["marker-change-time"] = printer.markerChangeTime();
        markers["marker-colors"] = printer.argument("marker-colors");
        markers["marker-levels"] = printer.argument("marker-levels");
        markers["marker-names"] = printer.argument("marker-names");
        markers["marker-types"] = printer.argument("marker-types");
        destItem->setData(markers, DestMarkers);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrinterModel::destStatus(KCupsPrinter::Status state, const QString &message) const
{
    switch (state) {
    case KCupsPrinter::Idle:
        if (message.isEmpty()){
            return i18n("Idle");
        } else {
            return i18n("Idle - '%1'", message);
        }
    case KCupsPrinter::Printing:
        if (message.isEmpty()){
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case KCupsPrinter::Stoped:
        if (message.isEmpty()){
            return i18n("Paused");
        } else {
            return i18n("Paused - '%1'", message);
        }
    default :
        if (message.isEmpty()){
            return i18n("Unknown");
        } else {
            return i18n("Unknown - '%1'", message);
        }
    }
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


void PrinterModel::insertUpdatePrinter(const QString &printerName)
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(insertUpdatePrinterFinished()));
    // TODO how do we know if it's a class if this DBus signal
    // does not tell us
    request->getPrinterAttributes(printerName, false, m_attributes);
}

void PrinterModel::insertUpdatePrinter(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)

    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    insertUpdatePrinter(printerName);
}

void PrinterModel::insertUpdatePrinterFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    if (!request->hasError()) {
        foreach (const KCupsPrinter &printer, request->printers()) {
            // If there is a printer and it's not the current one add it
            // as a new destination
            int dest_row = destRow(printer.name());
            if (dest_row == -1) {
                // not found, insert new one
                insertDest(0, printer);
            } else {
                // update the printer
                updateDest(item(dest_row), printer);
            }
        }
    }
    request->deleteLater();
}

void PrinterModel::printerRemoved(const QString &printerName)
{
    kDebug() << printerName;

    // Look for the removed printer
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        removeRows(dest_row, 1);
    }
}

void PrinterModel::printerRemoved(const QString &text,
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
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    // Look for the removed printer
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        removeRows(dest_row, 1);
    }
}

void PrinterModel::printerStateChanged(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}
void PrinterModel::printerStopped(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerRestarted(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerShutdown(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerModified(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    kDebug() << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

#include "PrinterModel.moc"
