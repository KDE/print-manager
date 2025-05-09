/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MarkerLevelChecker.h"
#include "pmkded_log.h"

#include <KCupsRequest.h>
#include <KLocalizedString>
#include <KNotification>
#include <ProcessRunner.h>

using namespace Qt::StringLiterals;

MarkerLevelChecker::MarkerLevelChecker(QObject *parent)
    : QObject(parent)
{
    qCDebug(PMKDED) << "print-manager daemon, starting" << Q_FUNC_INFO;
    init();
}

void MarkerLevelChecker::reset(bool reconnect)
{
    if (m_connection) {
        delete m_connection;
        m_connection.clear();
    }
    if (reconnect) {
        init();
    }
}
void MarkerLevelChecker::init()
{
    if (m_connection.isNull()) {
        m_connection = KCupsConnection::global();

        connect(m_connection, &KCupsConnection::serverStarted, this, [this](const QString &msg) {
            qCWarning(PMKDED) << "SERVER STARTED, connecting" << msg;
            reset();
        });

        connect(m_connection, &KCupsConnection::serverStopped, this, [this](const QString &msg) {
            qCWarning(PMKDED) << "SERVER STOPPED, disconnecting" << msg;
            reset(false);
        });

        connect(m_connection, &KCupsConnection::serverRestarted, this, [this](const QString &msg) {
            qCWarning(PMKDED) << "SERVER RE-STARTED, connecting" << msg;
            reset();
        });

        connect(m_connection, &KCupsConnection::jobCreated, this, &MarkerLevelChecker::jobHandler);
        connect(m_connection, &KCupsConnection::jobCompleted, this, &MarkerLevelChecker::jobHandler);
        connect(m_connection, &KCupsConnection::printerStateChanged, this, &MarkerLevelChecker::printerHandler);
    }
}

MarkerLevelChecker::~MarkerLevelChecker()
{
}

void MarkerLevelChecker::checkMarkerLevels(const QString &printerName)
{
    static const QStringList s_attrs({KCUPS_MARKER_NAMES,
                                      KCUPS_MARKER_LEVELS,
                                      KCUPS_MARKER_HIGH_LEVELS,
                                      KCUPS_MARKER_LOW_LEVELS,
                                      KCUPS_MARKER_TYPES,
                                      KCUPS_PRINTER_INFO,
                                      KCUPS_PRINTER_TYPE});

    /**
     *  Marker low level indicates the near empty warning value, but it can be 0.
     *  Let's be more conservative and warn if actual level is at a higher pct
     *  https://openprinting.github.io/cups/doc/spec-ipp.html#marker-low-levels
     */
    static const int s_threshold = 3;

    qCDebug(PMKDED) << "checkMarkerLevels: getting printer attributes" << printerName;

    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this](KCupsRequest *req) {
        req->deleteLater();
        const auto printer = req->printers().at(0);

        const auto ml = printer.argument(KCUPS_MARKER_LEVELS).toList(); // current levels
        const auto hl = printer.argument(KCUPS_MARKER_HIGH_LEVELS).toList(); // high boundary values
        const auto ll = printer.argument(KCUPS_MARKER_LOW_LEVELS).toList(); // low boundary values
        if (ml.isEmpty() || hl.isEmpty() || ll.isEmpty()) {
            qCDebug(PMKDED) << "Marker level attributes not found, nothing to check";
            return;
        }

        qCDebug(PMKDED) << "Checking Marker Levels against low threshold" << s_threshold;
        Q_ASSERT_X(ml.count() == hl.count() && ml.count() == ll.count(), "marker levels", "marker level lists have different lengths");

        int lowIndex = -1;
        int lowValue = 0;
        for (uint i = 0; i < ml.count(); ++i) {
            const int level = ml.at(i).toInt();
            const int low = ll.at(i).toInt();
            const int high = hl.at(i).toInt();

            // Because printers, level can be 0 and low boundary can be > zero
            // Also, level can be < low and not zero, ie. low=2, level=1
            if (level == 0 || level <= low) {
                lowIndex = i;
                break;
            } else {
                // CUPS seems to handle this, but just in case don't divide by zero
                if (high == 0) {
                    qCWarning(PMKDED) << "Found a high boundary == 0, exiting level check";
                    return;
                }
                if (level > low && level <= high) {
                    auto result = div(level * 100, high);
                    if (result.quot <= s_threshold) {
                        lowIndex = i;
                        lowValue = result.quot;
                        break;
                    }
                }
            }
        }
        // found a marker level at or below the threshold pct
        if (lowIndex >= 0) {
            // Make sure name/type lists are valid
            QString name(u"<unknown>"_s);
            QString type(u"<unknown>"_s);
            auto list = printer.argument(KCUPS_MARKER_NAMES).toStringList();
            if (lowIndex < list.count()) {
                name = list.at(lowIndex);
            }
            list = printer.argument(KCUPS_MARKER_TYPES).toStringList();
            if (lowIndex < list.count()) {
                type = list.at(lowIndex);
            }

            auto notify = new KNotification(u"MarkerLevel"_s);
            notify->setComponentName(u"printmanager"_s);
            notify->setTitle(printer.info());

            if (lowValue == 0) {
                notify->setText(i18nc("@info:usagetip", "%1 (%2) appears to be empty.", name, type));
            } else {
                notify->setText(i18nc("@info:usagetip", "%1 (%2) appears to be low (%3% remaining).", name, type, lowValue));
            }

            auto checkMarkers = notify->addDefaultAction(i18n("Check Levels…"));
            connect(checkMarkers, &KNotificationAction::activated, this, [pn = printer.name()]() {
                ProcessRunner::kcmConfigurePrinter(pn);
            });

            notify->sendEvent();
            qCDebug(PMKDED) << "Found marker-level at/below threshold" << type << name << lowValue << "<=" << s_threshold;
        } else {
            qCDebug(PMKDED) << "All marker-levels above low threshold" << s_threshold;
        }
    });

    // TODO: Use new libkcups apis
    request->getPrinterAttributes(printerName, false, s_attrs);
}

void MarkerLevelChecker::printerHandler(const QString &text,
                                        const QString &printerUri,
                                        const QString &printerName,
                                        uint printerState,
                                        const QString &printerStateReasons,
                                        bool printerIsAcceptingJobs)
{
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerIsAcceptingJobs)

    if (printerStateReasons.contains(u"marker-supply-low-warning"_s)) {
        checkMarkerLevels(printerName);
    }
}

void MarkerLevelChecker::jobHandler(const QString &text,
                                    const QString &printerUri,
                                    const QString &printerName,
                                    uint printerState,
                                    const QString &printerStateReasons,
                                    bool printerIsAcceptingJobs,
                                    uint jobId,
                                    uint jobState,
                                    const QString &jobStateReasons,
                                    const QString &jobName,
                                    uint jobImpressionsCompleted)
{
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    checkMarkerLevels(printerName);
}

#include "moc_MarkerLevelChecker.cpp"
