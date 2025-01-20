/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterModel.h"
#include "KCupsRequest.h"
#include "kcupslib_log.h"

#include <KLocalizedString>
#include <QVersionNumber>

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
    m_roles[DestIsDiscovered] = "isDiscovered";

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

void PrinterModel::getDestsFinished(KCupsRequest *request)
{
    // When there is no printer IPP_NOT_FOUND is returned
    if (request->hasError() && request->error() != IPP_NOT_FOUND) {
        // clear the model after so that the proper widget can be shown
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
        updateDisplayHints();
        Q_EMIT error(IPP_OK, QString(), QString());
    }
    request->deleteLater();
}

void PrinterModel::updateDisplayHints()
{
    QStringList locList;
    bool printersOnly = true;

    for (int i = 0; i < rowCount(); i++) {
        // Printers Only hint
        if (item(i)->data(DestIsClass).toBool()) {
            printersOnly = false;
        }

        // Location list hint
        const auto val = item(i)->data(DestLocation).toString();
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

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

KCupsRequest *PrinterModel::setupRequest(RequestFunc func)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, func](KCupsRequest *r) {
        if (r->hasError()) {
            Q_EMIT error(r->error(), r->serverError(), r->errorMsg());
        } else {
            if (func)
                func(this, r);
        }
        r->deleteLater();
    });

    return request;
}

bool PrinterModel::serverUnavailable() const
{
    return m_unavailable;
}

QHash<int, QByteArray> PrinterModel::roleNames() const
{
    return m_roles;
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
    m_filterMask = (m_includeDiscovered ? 0 : CUPS_PRINTER_DISCOVERED);
    clear();
    update();
    Q_EMIT includeDiscoveredChanged();
}

void PrinterModel::pausePrinter(const QString &printerName)
{
    const auto request = setupRequest();
    request->pausePrinter(printerName);
}

void PrinterModel::resumePrinter(const QString &printerName)
{
    const auto request = setupRequest();
    request->resumePrinter(printerName);
}

void PrinterModel::newDestination(const QVariantMap &destination)
{
    const KCupsPrinter printer(std::move(destination));

    // if item found, update it, otherwise add to model
    int row = destRow(printer.name());
    if (row == -1) {
        qCDebug(LIBKCUPS) << "Model ADDING" << printer.name() << "uriSupported:" << printer.uriSupported();
        // Create the printer item
        auto stdItem = new QStandardItem(printer.icon(), printer.name());
        stdItem->setData(printer.name(), DestName);
        updateDest(stdItem, printer);
        insertRow(0, stdItem);
    } else {
        qCDebug(LIBKCUPS) << "Model UPDATING" << printer.name() << row;
        updateDest(item(row), printer);
    }
}

void PrinterModel::update()
{
    const auto request = setupRequest(&PrinterModel::getDestsFinished);
    connect(request, &KCupsRequest::deviceMap, this, &PrinterModel::newDestination);
    request->getDestinations(m_searchTimeout, m_filterType, m_filterMask);
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

    bool paused = (state == KCupsPrinter::Stopped);
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
        destItem->setData(printerType & CUPS_PRINTER_DISCOVERED, DestIsDiscovered);
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

    // printer member names for type=class
    const auto members = printer.memberNames();
    if (members != destItem->data(DestMemberNames)) {
        destItem->setData(members, DestMemberNames);
    }

    int markerChangeTime = printer.markerChangeTime();
    if (markerChangeTime != destItem->data(DestMarkerChangeTime)) {
        destItem->setData(printer.markerChangeTime(), DestMarkerChangeTime);

        QVariantMap markers{{KCUPS_MARKER_CHANGE_TIME, printer.markerChangeTime()},
                            {KCUPS_MARKER_COLORS, printer.attribute(KCUPS_MARKER_COLORS).toStringList()},
                            {KCUPS_MARKER_NAMES, printer.attribute(KCUPS_MARKER_NAMES).toStringList()},
                            {KCUPS_MARKER_TYPES, printer.attribute(KCUPS_MARKER_TYPES).toStringList()}};

        // Levels needs to be a list of ints.  QVariant::toList converts an int to a null list
        // So, create a QList<int> if only one entry (int)
        const auto levels = printer.attribute(KCUPS_MARKER_LEVELS);
        if (levels.canConvert<QList<int>>()) {
            markers.insert(KCUPS_MARKER_LEVELS, levels);
        } else {
            QList<int> list;
            list << levels.toInt();
            markers.insert(KCUPS_MARKER_LEVELS, QVariant::fromValue(list));
        }

        destItem->setData(markers, DestMarkers);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // get the index row based on name
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString()) {
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

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PrinterModel::insertUpdatePrinter(const QString &printerName)
{
    qCDebug(LIBKCUPS) << "InsertUpdatePrinter" << printerName;
    const auto request = setupRequest(&PrinterModel::getDestsFinished);
    connect(request, &KCupsRequest::deviceMap, this, &PrinterModel::newDestination);
    request->getPrinterAttributesNotify(printerName, false, m_attrs);
}

void PrinterModel::printerRemoved(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << "printerRemoved" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    // remove item from model
    int row = destRow(printerName);
    if (row != -1) {
        removeRow(row);
    } else {
        qCDebug(LIBKCUPS) << "Unable to remove printer: not in model" << printerName;
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
        // Set "state" fields
        int row = destRow(printerName);
        if (row != -1) {
            item(row)->setData(printerState, DestState);
            item(row)->setData(printerIsAcceptingJobs, DestIsAcceptingJobs);
            item(row)->setData(printerState == KCupsPrinter::Stopped, DestIsPaused);
            item(row)->setData(destStatus(static_cast<KCupsPrinter::Status>(printerState), printerStateReasons, printerIsAcceptingJobs), DestStatus);
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

#include "moc_PrinterModel.cpp"
