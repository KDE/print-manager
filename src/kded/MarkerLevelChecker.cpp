/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MarkerLevelChecker.h"
#include "pmkded_log.h"

#include <KLocalizedString>
#include <KNotification>

#include <KCupsRequest.h>
#include <ProcessRunner.h>

using namespace Qt::StringLiterals;

MarkerLevelChecker::MarkerLevelChecker(QObject *parent)
    : QObject(parent)
{
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, [](const QString &msg) {
        qCDebug(PMKDED) << "CUPS Started:" << msg;
    });

    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, [](const QString &msg) {
        qCDebug(PMKDED) << "CUPS Stopped:" << msg;
    });

    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, [](const QString &msg) {
        qCDebug(PMKDED) << "CUPS Restarted:" << msg;
    });

    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &MarkerLevelChecker::jobHandler);
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
        // it's possible attributes from a temporary queue could be empty
        if (req->printers().isEmpty()) {
            qCDebug(PMKDED) << "No printers found for marker level check or CUPS services is not available";
            return;
        }

        const auto printer = req->printers().at(0);

        // QVariant::toList converts an int to a null list
        // So, create a QList<int> if only one entry (int)
        const auto getLevels = [printer](const QString &key) -> QList<QVariant> {
            QList<QVariant> list;
            const auto levels = printer.argument(key);
            if (levels.isValid()) {
                if (levels.canConvert<QList<int>>()) {
                    list = levels.toList();
                    qCDebug(PMKDED) << "Valid levels list" << key << list;
                } else {
                    list = QList<QVariant>{levels};
                    qCDebug(PMKDED) << "Valid level value set to a list" << key << list;
                }
            } else {
                // Valid levels entry not found
                qCDebug(PMKDED) << "Invalid levels entry:" << key;
            }

            return list;
        };

        const auto currentLevels = getLevels(KCUPS_MARKER_LEVELS);
        const auto lowLevels = getLevels(KCUPS_MARKER_LOW_LEVELS);
        const auto highLevels = getLevels(KCUPS_MARKER_HIGH_LEVELS);

        if (currentLevels.isEmpty() || highLevels.isEmpty() || lowLevels.isEmpty()) {
            qCDebug(PMKDED) << "At least one marker level attribute is invalid or not found, aborting level check";
            return;
        }

        int lowIndex = -1;
        int lowValue = 0;
        for (uint i = 0; i < currentLevels.count(); ++i) {
            const int level = currentLevels.at(i).toInt();
            const int low = lowLevels.at(i).toInt();
            const int high = highLevels.at(i).toInt();

            // Because printers, level can be 0 and low boundary can be > zero
            // Also, level can be < low and not zero, ie. low=2, level=1
            // level < 0 is unknown or unavailable, so ignore
            if (level == 0 || (level <= low && level > 0)) {
                lowIndex = i;
                if (level <= low) {
                    lowValue = level;
                }
                break;
            } else {
                // CUPS seems to handle this, but just in case don't divide by zero
                if (high == 0) {
                    qCWarning(PMKDED) << "Found a high boundary == 0, exiting level check";
                    return;
                }
                if (level > low && level <= high) {
                    auto result = div((level - low) * 100, high);
                    if (result.quot <= s_threshold) {
                        lowIndex = i;
                        lowValue = level;
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

            auto notify = new KNotification(u"MarkerLevel"_s, KNotification::Persistent);
            notify->setComponentName(u"printmanager"_s);
            notify->setTitle(printer.info());

            if (lowValue == 0) {
                notify->setText(i18nc("@info:usagetip The name and type of the ink cartridge", "%1 (%2) appears to be empty.", name, type));
            } else {
                notify->setText(i18nc("@info:usagetip The name and type of the ink cartridge and percent ink remaining",
                                      "%1 (%2) appears to be low (%3% remaining).",
                                      name,
                                      type,
                                      lowValue));
            }

            auto checkMarkers = notify->addDefaultAction(i18nc("@action:button check printer ink levels", "Check Levels…"));
            connect(checkMarkers, &KNotificationAction::activated, this, [pn = printer.name()]() {
                ProcessRunner::kcmConfigurePrinter(pn);
            });

            notify->sendEvent();
            qCDebug(PMKDED) << "Found marker-level at/below threshold" << type << name << lowValue;
        }
    });

    // TODO: Use new libkcups apis
    request->getPrinterAttributes(printerName, false, s_attrs);
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
