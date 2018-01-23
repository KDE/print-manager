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

#include "PrinterModel.h"

#include "Debug.h"

#include <QDateTime>
#include <QMimeData>
#include <QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QPointer>

#include <KUser>
#include <KLocalizedString>
#include <KMessageBox>

#include <KCupsRequest.h>

#include <cups/cups.h>

PrinterModel::PrinterModel(QObject *parent) :
    QStandardItemModel(parent),
    m_unavailable(true)
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

    QHash<int, QByteArray> roles = roleNames();
    roles[DestStatus] = "stateMessage";
    roles[DestName] = "printerName";
    roles[DestState] = "printerState";
    roles[DestIsDefault] = "isDefault";
    roles[DestIsShared] = "isShared";
    roles[DestIsAcceptingJobs] = "isAcceptingJobs";
    roles[DestIsPaused] = "isPaused";
    roles[DestIsClass] = "isClass";
    roles[DestLocation] = "location";
    roles[DestDescription] = "info";
    roles[DestKind] = "kind";
    roles[DestType] = "type";
    roles[DestCommands] = "commands";
    roles[DestMarkerChangeTime] = "markerChangeTime";
    roles[DestMarkers] = "markers";
    roles[DestIconName] = "iconName";
    roles[DestRemote] = "remote";
    setRoleNames(roles);

    // This is emitted when a printer is added
    connect(KCupsConnection::global(), &KCupsConnection::printerAdded, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(), &KCupsConnection::printerModified, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(), &KCupsConnection::printerStateChanged, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(), &KCupsConnection::printerStopped, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(), &KCupsConnection::printerRestarted, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(), &KCupsConnection::printerShutdown, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::printerDeleted, this, &PrinterModel::printerRemoved);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrinterModel::serverChanged);

    // Deprecated stuff that works better than the above
    connect(KCupsConnection::global(), &KCupsConnection::rhPrinterAdded, this, &PrinterModel::insertUpdatePrinterName);
    connect(KCupsConnection::global(), &KCupsConnection::rhPrinterRemoved, this, &PrinterModel::printerRemovedName);
    connect(KCupsConnection::global(), &KCupsConnection::rhQueueChanged, this, &PrinterModel::insertUpdatePrinterName);

    connect(this, &PrinterModel::rowsInserted, this, &PrinterModel::slotCountChanged);
    connect(this, &PrinterModel::rowsRemoved, this, &PrinterModel::slotCountChanged);
    connect(this, &PrinterModel::modelReset, this, &PrinterModel::slotCountChanged);

    update();
}

void PrinterModel::getDestsFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    // When there is no printer IPP_NOT_FOUND is returned
    if (request->hasError() && request->error() != IPP_NOT_FOUND) {
        // clear the model after so that the proper widget can be shown
        clear();

        emit error(request->error(), request->serverError(), request->errorMsg());
        if (request->error() == IPP_SERVICE_UNAVAILABLE && !m_unavailable) {
            m_unavailable = true;
            emit serverUnavailableChanged(m_unavailable);
        }
    } else {
        if (m_unavailable) {
            m_unavailable = false;
            emit serverUnavailableChanged(m_unavailable);
        }

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
}

void PrinterModel::slotCountChanged()
{
    emit countChanged(rowCount());
}

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

int PrinterModel::count() const
{
    return rowCount();
}

bool PrinterModel::serverUnavailable() const
{
    return m_unavailable;
}

void PrinterModel::pausePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->pausePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::resumePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->resumePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::rejectJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->rejectJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::acceptJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->acceptJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::update()
{
//                 kcmshell(6331) PrinterModel::update: (QHash(("printer-type", QVariant(int, 75534348) ) ( "marker-names" ,  QVariant(QStringList, ("Cyan", "Yellow", "Magenta", "Black") ) ) ( "printer-name" ,  QVariant(QString, "EPSON_Stylus_TX105") ) ( "marker-colors" ,  QVariant(QStringList, ("#00ffff", "#ffff00", "#ff00ff", "#000000") ) ) ( "printer-location" ,  QVariant(QString, "Luiz Vitor’s MacBook Pro") ) ( "marker-levels" ,  QVariant(QList<int>, ) ) ( "marker-types" ,  QVariant(QStringList, ("inkCartridge", "inkCartridge", "inkCartridge", "inkCartridge") ) ) ( "printer-is-shared" ,  QVariant(bool, true) ) ( "printer-state-message" ,  QVariant(QString, "") ) ( "printer-commands" ,  QVariant(QStringList, ("Clean", "PrintSelfTestPage", "ReportLevels") ) ) ( "marker-change-time" ,  QVariant(int, 1267903160) ) ( "printer-state" ,  QVariant(int, 3) ) ( "printer-info" ,  QVariant(QString, "EPSON Stylus TX105") ) ( "printer-make-and-model" ,  QVariant(QString, "EPSON TX105 Series") ) )  )
    // Get destinations with these attributes
    KCupsRequest *request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterModel::getDestsFinished);
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
    KCupsPrinter::Status state = printer.state();
    if (state != destItem->data(DestState)) {
        destItem->setData(state, DestState);
    }
    qCDebug(LIBKCUPS) << state << printer.name();

    // store if the printer is accepting jobs
    bool accepting = printer.isAcceptingJobs();
    if (accepting != destItem->data(DestIsAcceptingJobs)) {
        destItem->setData(accepting, DestIsAcceptingJobs);
    }

    // store the printer status message
    QString status = destStatus(state, printer.stateMsg(), accepting);
    if (status != destItem->data(DestStatus)) {
        destItem->setData(status, DestStatus);
    }

    bool paused = (state == KCupsPrinter::Stopped || !accepting);
    if (paused != destItem->data(DestIsPaused)) {
        destItem->setData(paused, DestIsPaused);
    }

    // store if the printer is shared
    bool shared = printer.isShared();
    if (shared != destItem->data(DestIsShared)) {
        destItem->setData(shared, DestIsShared);
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
        destItem->setData(printerType & CUPS_PRINTER_REMOTE, DestRemote);
    }

    // store the printer location
    QString location = printer.location();
    if (location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    // store the printer icon name
    QString iconName = printer.iconName();
    if (iconName != destItem->data(DestIconName).toString()) {
        destItem->setData(iconName, DestIconName);
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

QString PrinterModel::destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const
{
    switch (state) {
    case KCupsPrinter::Idle:
        if (message.isEmpty()){
            return isAcceptingJobs ? i18n("Idle") : i18n("Idle, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Idle - '%1'", message) : i18n("Idle, rejecting jobs - '%1'", message);
        }
    case KCupsPrinter::Printing:
        if (message.isEmpty()){
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case KCupsPrinter::Stopped:
        if (message.isEmpty()){
            return isAcceptingJobs ? i18n("Paused") : i18n("Paused, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Paused - '%1'", message) : i18n("Paused, rejecting jobs - '%1'", message);
        }
    default :
        if (message.isEmpty()){
            return i18n("Unknown");
        } else {
            return i18n("Unknown - '%1'", message);
        }
    }
}

void PrinterModel::clear()
{
    removeRows(0, rowCount());
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


void PrinterModel::insertUpdatePrinterName(const QString &printerName)
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterModel::insertUpdatePrinterFinished);
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

    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    insertUpdatePrinterName(printerName);
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

void PrinterModel::printerRemovedName(const QString &printerName)
{
    qCDebug(LIBKCUPS) << printerName;

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
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    // Look for the removed printer
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        removeRows(dest_row, 1);
    }
}

void PrinterModel::printerStateChanged(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}
void PrinterModel::printerStopped(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerRestarted(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerShutdown(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerModified(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::serverChanged(const QString &text)
{
    qCDebug(LIBKCUPS) << text;
    update();
}
