/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterModel.h"

#include "kcupslib_log.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDateTime>
#include <QMimeData>
#include <QPointer>

#include <KLocalizedString>
#include <KMessageBox>
#include <KUser>

#include <KCupsRequest.h>

#include <cups/cups.h>

PrinterModel::PrinterModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_attrs({KCUPS_PRINTER_NAME,
               KCUPS_PRINTER_STATE,
               KCUPS_PRINTER_STATE_MESSAGE,
               KCUPS_PRINTER_IS_SHARED,
               KCUPS_PRINTER_IS_ACCEPTING_JOBS,
               KCUPS_PRINTER_TYPE,
               KCUPS_PRINTER_LOCATION,
               KCUPS_PRINTER_INFO,
               KCUPS_PRINTER_MAKE_AND_MODEL,
               KCUPS_PRINTER_COMMANDS,
               KCUPS_MARKER_CHANGE_TIME,
               KCUPS_MARKER_COLORS,
               KCUPS_MARKER_LEVELS,
               KCUPS_MARKER_NAMES,
               KCUPS_MARKER_TYPES,
               KCUPS_DEVICE_URI,
               KCUPS_PRINTER_URI_SUPPORTED,
               KCUPS_MEMBER_NAMES})
{
    m_roles = QStandardItemModel::roleNames();
    m_roles[DestStatus] = "stateMessage";
    m_roles[DestName] = "printerName";
    m_roles[DestState] = "printerState";
    m_roles[DestIsDefault] = "isDefault";
    m_roles[DestIsShared] = "isShared";
    m_roles[DestIsAcceptingJobs] = "isAcceptingJobs";
    m_roles[DestIsPaused] = "isPaused";
    m_roles[DestIsClass] = "isClass";
    m_roles[DestLocation] = "location";
    m_roles[DestDescription] = "info";
    m_roles[DestKind] = "kind";
    m_roles[DestType] = "type";
    m_roles[DestCommands] = "commands";
    m_roles[DestMarkerChangeTime] = "markerChangeTime";
    m_roles[DestMarkers] = "markers";
    m_roles[DestIconName] = "iconName";
    m_roles[DestRemote] = "remote";
    m_roles[DestUri] = "printerUri";
    m_roles[DestUriSupported] = "uriSupported";
    m_roles[DestMemberNames] = "memberNames";

    // This is emitted when a printer is added
    connect(KCupsConnection::global(), &KCupsConnection::printerAdded,
            this,
            &PrinterModel::insertPrinter);

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(), &KCupsConnection::printerModified,
            this,
            &PrinterModel::updatePrinter);

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(), &KCupsConnection::printerStateChanged,
            this,
            &PrinterModel::updatePrinterState);

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(), &KCupsConnection::printerStopped,
            this,
            &PrinterModel::updatePrinterState);

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(), &KCupsConnection::printerRestarted,
            this,
            &PrinterModel::updatePrinterState);

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(), &KCupsConnection::printerShutdown,
            this,
            &PrinterModel::updatePrinterState);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::printerDeleted,
            this,
            &PrinterModel::removePrinter);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrinterModel::serverChanged);
}

void PrinterModel::getDestsFinished(KCupsRequest *request)
{
    // When there is no printer IPP_NOT_FOUND is returned
    if (request->hasError() && request->error() != IPP_NOT_FOUND) {
        clear();

        Q_EMIT error(request->error(), request->serverError(), request->errorMsg());
        if (request->error() == IPP_SERVICE_UNAVAILABLE && !m_unavailable) {
            m_unavailable = true;
            Q_EMIT serverUnavailableChanged(m_unavailable);
        }
    } else {
        if (m_unavailable) {
            m_unavailable = false;
            Q_EMIT serverUnavailableChanged(m_unavailable);
        }

        setDisplayLocationHint();
        setPrintersOnly();

        Q_EMIT error(IPP_OK, QString(), QString());
    }
    request->deleteLater();
}

void PrinterModel::setDisplayLocationHint()
{
    QStringList locList;

    // get location list
    for (int i = 0; i < rowCount(); i++) {
        const auto val = item(i)->data(DestLocation).toString();
        if (!val.isEmpty()) {
            locList.append(val);
        }
    }
    // only show the location if there is more than one printer
    // and at least two distinct locations exist
    locList.removeDuplicates();
    m_displayLocationHint = rowCount() > 1 && locList.count() > 1;
    Q_EMIT displayLocationHintChanged();
}

void PrinterModel::setPrintersOnly()
{
    bool only = true;
    for (int i = 0; i < rowCount(); i++) {
        if (item(i)->data(DestIsClass).toBool()) {
            only = false;
            break;
        }
    }

    if (only != m_printersOnly) {
        m_printersOnly = only;
        Q_EMIT printersOnlyChanged();
    }
}

bool PrinterModel::printersOnly() const
{
    return m_printersOnly;
}

bool PrinterModel::displayLocationHint() const
{
    return m_displayLocationHint;
}

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

bool PrinterModel::serverUnavailable() const
{
    return m_unavailable;
}

QHash<int, QByteArray> PrinterModel::roleNames() const
{
    return m_roles;
}

void PrinterModel::pausePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest(KCupsConnection::global());
    request->pausePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::resumePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest(KCupsConnection::global());
    request->resumePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::rejectJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest(KCupsConnection::global());
    request->rejectJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::acceptJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest(KCupsConnection::global());
    request->acceptJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::gotDevice(const QVariantMap &device)
{
    const KCupsPrinter printer(std::move(device));
    // Find if printer is already in the model
    int dest_row = destRow(printer.name());
    if (dest_row == -1) {
        // Create the printer item
        qCDebug(LIBKCUPS) << "Model INSERT:" << printer.name() << "uriSupported:" << printer.uriSupported();
        auto stdItem = new QStandardItem(printer.icon(), printer.name());
        stdItem->setData(printer.name(), DestName);
        // update the item
        updateDest(stdItem, printer);
        appendRow(stdItem);
    } else {
        // update the printer
        qCDebug(LIBKCUPS) << "Model UPDATE:" << printer.name() << "@Ndx:" << dest_row;
        updateDest(item(dest_row), printer);
    }
}

void PrinterModel::update()
{
    auto request = new KCupsRequest(KCupsConnection::global());
    connect(request, &KCupsRequest::deviceMap, this, &PrinterModel::gotDevice);
    connect(request, &KCupsRequest::finished, this, &PrinterModel::getDestsFinished);

    cups_ptype_t filterMask = m_searchIncludeDiscovered ? 0 : CUPS_PRINTER_DISCOVERED;
    request->getDestinations(m_searchTimeout, CUPS_PRINTER_LOCAL, filterMask);
}

void PrinterModel::updateDest(QStandardItem *destItem, const KCupsPrinter &printer)
{
    // store if the printer is the network default
    bool isDefault = printer.isDefault();
    if (destItem->data(DestIsDefault).isNull() || isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    KCupsPrinter::Status state = printer.state();
    if (state != destItem->data(DestState)) {
        destItem->setData(state, DestState);
    }

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
    // printer member names for type=class
    if (isClass) {
        const auto members = printer.memberNames();
        if (members != destItem->data(DestMemberNames)) {
            destItem->setData(members, DestMemberNames);
        }
    }

    // the printer-type param is a flag
    uint printerType = printer.type();
    if (printerType != destItem->data(DestType)) {
        destItem->setData(printerType, DestType);
        destItem->setData(printerType & CUPS_PRINTER_REMOTE, DestRemote);
    }

    // store the printer location
    QString location = printer.location();
    if (location.isEmpty() || location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    // store the printer icon name
    QString iconName = printer.iconName();
    if (iconName != destItem->data(DestIconName).toString()) {
        destItem->setData(iconName, DestIconName);
    }

    // set the display role to the printer name
    if (destItem->data(DestName).toString() != destItem->text()) {
        destItem->setText(destItem->data(DestName).toString());
    }

    // store the printer description
    QString description = printer.info();
    if (description.isEmpty() || description != destItem->data(DestDescription).toString()) {
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

    // store the printer URI
    QString uri = printer.deviceUri();
    if (uri != destItem->data(DestUri).toString()) {
        destItem->setData(uri, DestUri);
    }

    QString us = printer.uriSupported();
    if (us != destItem->data(DestUriSupported).toString()) {
        destItem->setData(us, DestUriSupported);
    }

    int markerChangeTime = printer.markerChangeTime();
    if (markerChangeTime != destItem->data(DestMarkerChangeTime)) {
        destItem->setData(printer.markerChangeTime(), DestMarkerChangeTime);
        const QVariantMap markers{{KCUPS_MARKER_CHANGE_TIME, printer.markerChangeTime()},
                                  {KCUPS_MARKER_COLORS, printer.argument(KCUPS_MARKER_COLORS)},
                                  {KCUPS_MARKER_LEVELS, printer.argument(KCUPS_MARKER_LEVELS)},
                                  {KCUPS_MARKER_NAMES, printer.argument(KCUPS_MARKER_NAMES)},
                                  {KCUPS_MARKER_TYPES, printer.argument(KCUPS_MARKER_TYPES)}};
        destItem->setData(markers, DestMarkers);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the index for the printer
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString()) {
            return i;
        }
    }
    return -1;
}

QString PrinterModel::destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const
{
    switch (state) {
    case KCupsPrinter::Idle:
        if (message.isEmpty()) {
            return isAcceptingJobs ? i18n("Idle") : i18n("Idle, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Idle - '%1'", message) : i18n("Idle, rejecting jobs - '%1'", message);
        }
    case KCupsPrinter::Printing:
        if (message.isEmpty()) {
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case KCupsPrinter::Stopped:
        if (message.isEmpty()) {
            return isAcceptingJobs ? i18n("Paused") : i18n("Paused, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Paused - '%1'", message) : i18n("Paused, rejecting jobs - '%1'", message);
        }
    default:
        if (message.isEmpty()) {
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

void PrinterModel::insertPrinter(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    if (destRow(printerName) == -1) {
        qCDebug(LIBKCUPS) << "CUPS PRINTER INSERT:" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
        getPrinterAttributes(printerName);
    } else {
        qCWarning(LIBKCUPS) << "****** PRINTER ALREADY IN MODEL, EVENT NOT HANDLED:" << text;
    }
}

void PrinterModel::updatePrinter(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    if (destRow(printerName) >= 0) {
        qCDebug(LIBKCUPS) << "CUPS PRINTER UPDATE:" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
        getPrinterAttributes(printerName);
    } else {
        qCWarning(LIBKCUPS) << "****** PRINTER NOT IN MODEL, EVENT NOT HANDLED:" << text;
    }
}

void PrinterModel::getPrinterAttributes(const QString &printer)
{
    auto request = new KCupsRequest(KCupsConnection::global());
    connect(request, &KCupsRequest::deviceMap, this, &PrinterModel::gotDevice);
    connect(request, &KCupsRequest::finished, this, &PrinterModel::getDestsFinished);
    // Always call with class=false because the only thing that change on a class
    // after it's set up is the member list
    request->getPrinterAttributesNotify(printer, false, m_attrs);
}

void PrinterModel::updatePrinterState(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    int ndx = destRow(printerName);
    if (ndx >= 0) {
        qCDebug(LIBKCUPS) << "CUPS PRINTER STATE:" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

        /**
         *  CUPS "state" update does not include the default printer setting
         *  so check each data item for change and if nothing changes, we assume
         *  the default setting changed (or something else).  Therefore, do a
         *  complete printer update.
        */

        const auto i = item(ndx);
        bool change = false;

        KCupsPrinter::Status state = static_cast<KCupsPrinter::Status>(printerState);
        if (i->data(DestState) != state) {
            i->setData(state, DestState);
            change = true;
        }

        const auto status = destStatus(state, QString(), printerIsAcceptingJobs);
        if (i->data(DestStatus) != status) {
            i->setData(status, DestStatus);
            change = true;
        }

        if (i->data(DestIsAcceptingJobs).toBool() != printerIsAcceptingJobs) {
            i->setData(printerIsAcceptingJobs, DestIsAcceptingJobs);
            change = true;
        }

        bool paused = (state == KCupsPrinter::Stopped || !printerIsAcceptingJobs);
        if (i->data(DestIsPaused) != paused) {
            i->setData(paused, DestIsPaused);
            change = true;
        }

        if (!change) {
            getPrinterAttributes(printerName);
        }
    } else {
        qCWarning(LIBKCUPS) << "****** PRINTER NOT IN MODEL, EVENT NOT HANDLED:" << text;
    }

}

void PrinterModel::removePrinter(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        qCDebug(LIBKCUPS) << "CUPS PRINTER REMOVE:" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
        removeRows(dest_row, 1);
        setDisplayLocationHint();
        setPrintersOnly();
    } else {
        qCWarning(LIBKCUPS) << "****** PRINTER NOT IN MODEL, EVENT NOT HANDLED:" << text;
    }
}

void PrinterModel::serverChanged(const QString &text)
{
    qCWarning(LIBKCUPS) << "PrinterModel:" << text;
    clear();
    update();
}

bool PrinterModel::searchIncludeDiscovered() const
{
    return m_searchIncludeDiscovered;
}

void PrinterModel::setSearchIncludeDiscovered(bool newSearchIncludeDiscovered)
{
    if (m_searchIncludeDiscovered == newSearchIncludeDiscovered)
        return;
    m_searchIncludeDiscovered = newSearchIncludeDiscovered;
    emit searchIncludeDiscoveredChanged();
}

int PrinterModel::searchTimeout() const
{
    return m_searchTimeout;
}

void PrinterModel::setSearchTimeout(int newSearchTimeout)
{
    if (m_searchTimeout == newSearchTimeout)
        return;
    m_searchTimeout = newSearchTimeout;
}

#include "moc_PrinterModel.cpp"
