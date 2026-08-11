/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterModel.h"
#include "CommandHelpers.h"
#include "KCupsRequest.h"
#include "kcupslib_log.h"

#include <KLocalizedString>
#include <QVersionNumber>

using namespace Qt::StringLiterals;

PrinterModel::PrinterModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // This is emitted when a printer is added
    connect(KCupsConnection::global(), &KCupsConnection::printerAdded, this, &PrinterModel::printerAdded);

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(), &KCupsConnection::printerModified, this, &PrinterModel::printerModified);

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(), &KCupsConnection::printerStateChanged, this, &PrinterModel::printerStateChanged);

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(), &KCupsConnection::printerStopped, this, &PrinterModel::printerStopped);

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(), &KCupsConnection::printerRestarted, this, &PrinterModel::printerRestarted);

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(), &KCupsConnection::printerShutdown, this, &PrinterModel::printerShutdown);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::printerDeleted, this, &PrinterModel::printerRemoved);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrinterModel::serverChanged);

    update();
}

void PrinterModel::clear()
{
    beginResetModel();
    m_printers.clear();
    endResetModel();
}

int PrinterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_printers.size();
}

void PrinterModel::getDestsFinished(KCupsRequest *request)
{
    // When there is no printer IPP_NOT_FOUND is returned
    if (request->hasError() && request->error() != IPP_STATUS_ERROR_NOT_FOUND) {
        clear();
        Q_EMIT error(request->error(), request->serverError(), request->errorMsg());
        if (request->error() == IPP_STATUS_ERROR_SERVICE_UNAVAILABLE) {
            setServerState(ServerState::Unavailable);
        }
    } else {
        Q_EMIT error(IPP_STATUS_OK, QString(), QString());
        setServerState(ServerState::Available);
        updateDisplayHints();

        for (auto printer = m_printers.begin(); printer != m_printers.end(); ++printer) {
            if (!printer->isClass()) {
                getIppDirectData(printer);
            }
        }
    }
}

/** Resolve the device uri if discovery type (dnssd://host.../._ipp...)
 * Uris already resolved are left as-is
 * Then, try the direct-ipp attribute query
 */
void PrinterModel::getIppDirectData(QList<KCupsPrinter>::iterator printer)
{
    qWarning() << printer->name() << "Trying Direct-IPP:" << printer->deviceUri();
    const auto resolvedUri = PrinterCommands::resolveToUri(printer->deviceUri());
    if (!resolvedUri.isEmpty()) {
        qCDebug(LIBKCUPS) << printer->name() << "URI Resolved:" << printer->deviceUri() << "->" << resolvedUri;
        printer->setAttribute(KCUPS_DEVICE_URI, resolvedUri);
        const auto idx = index(std::distance(m_printers.begin(), printer));
        Q_EMIT dataChanged(idx, idx, {DestUri});
    }

    QStringList attr = QStringList{KCUPS_PRINTER_MORE_INFO, KCUPS_PRINTER_SUPPLY_INFO_URI};
    if (printer->markers().isEmpty()) {
        attr +=
            QStringList{KCUPS_MARKER_COLORS, KCUPS_MARKER_LEVELS, KCUPS_MARKER_NAMES, KCUPS_MARKER_TYPES, KCUPS_MARKER_LOW_LEVELS, KCUPS_MARKER_HIGH_LEVELS};
        qCDebug(LIBKCUPS) << printer->name() << "CUPS-layer marker levels not found, trying Direct-IPP";
    }

    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, printer, attr](KCupsRequest *req) {
        if (!req->printers().isEmpty()) {
            const auto found = req->printers().at(0);
            for (const auto &key : attr) {
                printer->setAttribute(key, found.argument(key));
            }
            const auto idx = index(std::distance(m_printers.begin(), printer));
            Q_EMIT dataChanged(idx, idx);
            qWarning() << printer->name() << "Got IPP DIRECT!" << printer->deviceUri();
            // for (const auto &key : attr) {
            //     qWarning() << key << it->argument(key);
            // }
        }
        req->deleteLater();
    });
    request->getAttributesDirect(printer->name(), printer->deviceUri(), attr);
}

void PrinterModel::updateDisplayHints()
{
    QStringList locList;
    bool printersOnly = true;

    for (const auto &printer : std::as_const(m_printers)) {
        // Printers Only hint
        if (printer.isClass()) {
            printersOnly = false;
        }

        // Location list hint
        const auto val = printer.location();
        if (!val.isEmpty()) {
            locList.append(val);
        }
    }
    // only show the location if there is more than one printer
    // and at least two distinct locations exist
    locList.removeDuplicates();
    bool displayLocationHint = rowCount() > 1 && locList.count() > 1;
    if (m_showLocations != displayLocationHint) {
        m_showLocations = displayLocationHint;
        Q_EMIT showLocationsChanged();
    }

    if (m_hasOnlyPrinters != printersOnly) {
        m_hasOnlyPrinters = printersOnly;
        Q_EMIT hasOnlyPrintersChanged();
    }
}

bool PrinterModel::hasOnlyPrinters() const
{
    return m_hasOnlyPrinters;
}

bool PrinterModel::showLocations() const
{
    return m_showLocations;
}

KCupsRequest *PrinterModel::setupRequest(RequestFunc func)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, func](KCupsRequest *r) {
        if (r->hasError()) {
            qCDebug(LIBKCUPS) << r->error() << r->serverError() << r->errorMsg();
            Q_EMIT error(r->error(), r->serverError(), r->errorMsg());
        } else {
            if (func)
                (this->*func)(r);
        }
        r->deleteLater();
    });

    return request;
}

ServerState::State PrinterModel::serverState() const
{
    return m_serverState;
}

void PrinterModel::setServerState(ServerState::State state)
{
    if (m_serverState == state) {
        return;
    }

    m_serverState = state;
    Q_EMIT serverStateChanged();
}

QHash<int, QByteArray> PrinterModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{DestStatus, "stateMessage"},
                                              {DestName, "printerName"},
                                              {DestState, "printerState"},
                                              {DestIsDefault, "isDefault"},
                                              {DestIsShared, "isShared"},
                                              {DestIsAcceptingJobs, "isAcceptingJobs"},
                                              {DestIsPaused, "isPaused"},
                                              {DestIsClass, "isClass"},
                                              {DestLocation, "location"},
                                              {DestDescription, "info"},
                                              {DestKind, "kind"},
                                              {DestType, "type"},
                                              {DestCommands, "commands"},
                                              {DestMarkerChangeTime, "markerChangeTime"},
                                              {DestMarkers, "markers"},
                                              {DestIconName, "iconName"},
                                              {DestRemote, "remote"},
                                              {DestUri, "printerUri"},
                                              {DestUriSupported, "uriSupported"},
                                              {DestMemberNames, "memberNames"},
                                              {DestIsDiscovered, "isDiscovered"},
                                              {DestMoreInfo, "moreInfo"},
                                              {DestSupplyInfoUri, "supplyInfoUri"}};
    return roles;
}

bool PrinterModel::includeDiscovered() const
{
    return m_includeDiscovered;
}

void PrinterModel::setIncludeDiscovered(bool newIncludeDiscovered)
{
    if (m_includeDiscovered == newIncludeDiscovered)
        return;

    m_includeDiscovered = newIncludeDiscovered;
    setFilterMask(m_includeDiscovered ? 0 : KCUPS_PRINTER_DISCOVERED);
    clear();
    update();
    Q_EMIT includeDiscoveredChanged();
}

void PrinterModel::insertUpdateFinished(KCupsRequest *request)
{
    if (request->hasError() && request->error() != IPP_STATUS_ERROR_NOT_FOUND) {
        Q_EMIT error(request->error(), request->serverError(), request->errorMsg());
        return;
    }

    if (request->printers().isEmpty()) {
        return;
    }

    const auto printer = request->printers().at(0);
    setModelItem(printer);
    updateDisplayHints();
    if (!printer.isClass()) {
        getIppDirectData(std::ranges::find(m_printers, printer.name(), &KCupsPrinter::name));
    }
}

QVariant PrinterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_printers.size())
        return {};

    const auto p = m_printers.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return p.name();
    case Qt::DecorationRole:
        return p.iconName();
    case DestStatus:
        return destStatus(p.state(), p.stateMsg(), p.isAcceptingJobs());
    case DestState:
        return p.state();
    case DestName:
        return p.name();
    case DestIsDefault:
        return p.isDefault();
    case DestIsShared:
        return p.isShared();
    case DestIsAcceptingJobs:
        return p.isAcceptingJobs();
    case DestIsPaused:
        return p.state() == KCupsPrinter::Stopped;
    case DestIsClass:
        return p.isClass();
    case DestLocation:
        return p.location();
    case DestDescription:
        return p.info();
    case DestKind:
        return p.makeAndModel();
    case DestType:
        return p.type();
    case DestCommands:
        return p.commands();
    case DestMarkerChangeTime:
        return p.markerChangeTime();
    case DestMarkers:
        return p.markers();
    case DestIconName:
        return p.iconName();
    case DestRemote:
        return p.type() & KCUPS_PRINTER_REMOTE;
    case DestIsDiscovered:
        return p.isDiscovered();
    case DestUri:
        return p.deviceUri();
    case DestMoreInfo:
        return p.argument(KCUPS_PRINTER_MORE_INFO);
    case DestUriSupported:
        return p.uriSupported();
    case DestMemberNames:
        return p.memberNames();
    case DestSupplyInfoUri:
        return p.argument(KCUPS_PRINTER_SUPPLY_INFO_URI);
    default:
        return {};
    }
}

void PrinterModel::setModelItem(const KCupsPrinter &printer)
{
    // if item found, update it, otherwise add to model
    if (const auto ndx = findIndex(printer.name()); ndx == -1) {
        qCDebug(LIBKCUPS) << "Model ADDING" << printer.name();
        const auto row = m_printers.size();
        beginInsertRows(QModelIndex(), row, row);
        m_printers.append(printer);
        endInsertRows();
    } else {
        qCDebug(LIBKCUPS) << "Model UPDATING" << printer.name() << ndx;
        m_printers[ndx] = printer;
        const auto idx = index(ndx);
        Q_EMIT dataChanged(idx, idx);
    }
}

void PrinterModel::update()
{
    const auto request = setupRequest(&PrinterModel::getDestsFinished);
    connect(request, &KCupsRequest::destination, this, &PrinterModel::setModelItem);
    request->getDestinations(m_searchTimeout, m_filterType, m_filterMask);
}

int PrinterModel::findIndex(const QString &destName)
{
    auto it = std::ranges::find_if(m_printers, [destName](const KCupsPrinter &printer) {
        return destName == printer.name();
    });
    if (it != m_printers.end()) {
        return std::distance(m_printers.begin(), it);
    }
    return -1;
}

QString PrinterModel::destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const
{
    switch (state) {
    case KCupsPrinter::Idle:
        if (message.isEmpty() || message.toLower() == QStringLiteral("none")) {
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
        if (message.isEmpty() || message.toLower() == QStringLiteral("paused")) {
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

void PrinterModel::insertUpdatePrinter(const QString &printerName)
{
    static const QStringList s_attr = {KCUPS_PRINTER_NAME,
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
                                       KCUPS_MEMBER_NAMES};

    const auto request = setupRequest(&PrinterModel::insertUpdateFinished);
    request->getPrinterAttributes(printerName, false, s_attr);
}

void PrinterModel::printerRemoved(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerRemoved" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    if (const auto ndx = findIndex(printerName); ndx != -1) {
        beginRemoveRows(QModelIndex(), ndx, ndx);
        m_printers.removeAt(ndx);
        endRemoveRows();
    }
    updateDisplayHints();
}

void PrinterModel::printerStateChanged(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerStateChanged" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    // WORKAROUND: CUPS Issues #1235/#1246 (https://github.com/OpenPrinting/cups/issues/1235)
    // Fixed in 2.4.13+/2.5 (N/A in CUPS 3.x)
    if (QVersionNumber(CUPS_VERSION_MAJOR, CUPS_VERSION_MINOR, CUPS_VERSION_PATCH) < QVersionNumber(2, 4, 13)) {
        insertUpdatePrinter(printerName);
    } else {
        auto it = std::ranges::find(m_printers, printerName, &KCupsPrinter::name);
        if (it != m_printers.end()) {
            it->setAttribute(KCUPS_PRINTER_STATE, printerState);
            it->setAttribute(KCUPS_PRINTER_IS_ACCEPTING_JOBS, printerIsAcceptingJobs);
            it->setAttribute(KCUPS_PRINTER_STATE_MESSAGE, printerStateReasons);
            const auto idx = index(std::distance(m_printers.begin(), it));
            Q_EMIT dataChanged(idx, idx, {DestState, DestIsAcceptingJobs, DestIsPaused, DestStatus});
        } else {
            qCDebug(LIBKCUPS) << "Unable to set State, printer not in model:" << printerName;
        }
    }
}

void PrinterModel::printerStopped(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerStopped" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    printerStateChanged(text, printerUri, printerName, printerState, printerStateReasons, printerIsAcceptingJobs);
}

void PrinterModel::printerRestarted(const QString &text,
                                    const QString &printerUri,
                                    const QString &printerName,
                                    uint printerState,
                                    const QString &printerStateReasons,
                                    bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerRestarted" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    printerStateChanged(text, printerUri, printerName, printerState, printerStateReasons, printerIsAcceptingJobs);
}

void PrinterModel::printerShutdown(const QString &text,
                                   const QString &printerUri,
                                   const QString &printerName,
                                   uint printerState,
                                   const QString &printerStateReasons,
                                   bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerShutdown" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    printerStateChanged(text, printerUri, printerName, printerState, printerStateReasons, printerIsAcceptingJobs);
}

void PrinterModel::printerModified(const QString &text,
                                   const QString &printerUri,
                                   const QString &printerName,
                                   uint printerState,
                                   const QString &printerStateReasons,
                                   bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerModified" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    insertUpdatePrinter(printerName);
}

void PrinterModel::printerAdded(const QString &text,
                                const QString &printerUri,
                                const QString &printerName,
                                uint printerState,
                                const QString &printerStateReasons,
                                bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerAdded" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    insertUpdatePrinter(printerName);
}

void PrinterModel::serverChanged(const QString &text)
{
    qCDebug(LIBKCUPS) << "serverChanged" << text;
    update();
}

uint PrinterModel::filterMask() const
{
    return m_filterMask;
}

void PrinterModel::setFilterMask(uint filterMask)
{
    if (filterMask == m_filterMask) {
        return;
    }
    m_filterMask = filterMask;
    Q_EMIT filterMaskChanged();
}

uint PrinterModel::filterType() const
{
    return m_filterType;
}

void PrinterModel::setFilterType(uint filterType)
{
    if (filterType == m_filterType) {
        return;
    }
    m_filterType = filterType;
    Q_EMIT filterTypeChanged();
}

uint PrinterModel::searchTimeout() const
{
    return m_searchTimeout;
}

void PrinterModel::setSearchTimeout(uint timeout)
{
    if (timeout == m_searchTimeout) {
        return;
    }
    m_searchTimeout = timeout;
    Q_EMIT searchTimeoutChanged();
}

#include "moc_PrinterModel.cpp"
