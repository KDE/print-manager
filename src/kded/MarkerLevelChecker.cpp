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
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &MarkerLevelChecker::jobHandler);
    connect(KCupsConnection::global(), &KCupsConnection::jobCompleted, this, &MarkerLevelChecker::jobHandler);
}

MarkerLevelChecker::~MarkerLevelChecker()
{
}

void MarkerLevelChecker::checkMarkerLevels(const QString &printerName, const QString &printerUri)
{
    static const QStringList s_attrs({KCUPS_MARKER_NAMES,
                                      KCUPS_MARKER_LEVELS,
                                      KCUPS_MARKER_HIGH_LEVELS,
                                      KCUPS_MARKER_LOW_LEVELS,
                                      KCUPS_MARKER_TYPES,
                                      KCUPS_PRINTER_INFO,
                                      KCUPS_PRINTER_TYPE});
    static const int s_threshold = 5;

    qCDebug(PMKDED) << "checkMarkerLevels" << printerName << printerUri;

    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this](KCupsRequest *req) {
        req->deleteLater();
        const auto printer = req->printers().at(0);

        // Class will not have marker level attributes
        if (printer.type() & CUPS_PRINTER_CLASS) {
            return;
        }

        const auto ml = printer.argument(KCUPS_MARKER_LEVELS).toList(); // current levels
        const auto hl = printer.argument(KCUPS_MARKER_HIGH_LEVELS).toList(); // high boundary values
        if (ml.isEmpty() || hl.isEmpty()) {
            // levels, etc., not found, noop
            qCDebug(PMKDED) << "Marker attributes not found, nothing to check";
            return;
        }

        qCDebug(PMKDED) << "Checking Marker Levels against low threshold" << s_threshold;
        const auto ll = printer.argument(KCUPS_MARKER_LOW_LEVELS).toList(); // low boundary values
        int lowIndex = -1;
        int lowValue = 0;
        Q_ASSERT_X(ml.count() == hl.count() && ml.count() == ll.count(), "marker levels", "marker level lists have different lengths");
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
            const auto name = printer.argument(KCUPS_MARKER_NAMES).toStringList().at(lowIndex);
            const auto type = printer.argument(KCUPS_MARKER_TYPES).toStringList().at(lowIndex);
            auto notify = new KNotification(u"MarkerLevel"_s);
            notify->setComponentName(u"printmanager"_s);
            notify->setTitle(printer.info());

            if (lowValue == 0) {
                notify->setText(i18nc("@info:usagetip", "%1 (%2) appears to be empty.", name, type, lowValue));
            } else {
                notify->setText(i18nc("@info:usagetip", "%1 (%2) appears to be low (%3% remaining).", name, type, lowValue));
            }

            auto checkMarkers = notify->addDefaultAction(i18n("Check Levels…"));
            connect(checkMarkers, &KNotificationAction::activated, this, [printer]() {
                ProcessRunner::kcmConfigurePrinter(printer.name());
            });

            notify->sendEvent();
            qCDebug(PMKDED) << "Found Marker Level below threshold" << name << lowValue << "<=" << s_threshold;
        } else {
            qCDebug(PMKDED) << "All Marker Levels above low threshold" << s_threshold;
        }
    });

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
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    // We don't care about the job, we just want to check marker levels
    checkMarkerLevels(printerName, printerUri);
}

#include "moc_MarkerLevelChecker.cpp"
