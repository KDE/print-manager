/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MarkerLevelChecker.h"
#include "pmkded_log.h"

#include <KCupsRequest.h>
#include <KLocalizedString>
#include <KNotification>

#include <QDateTime>

using namespace Qt::StringLiterals;

MarkerLevelChecker::MarkerLevelChecker(QObject *parent)
    : QObject(parent)
{
    // TODO: should we also check when the job is finished?
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &MarkerLevelChecker::jobHandler);
    // TODO: remove before merge
    connect(KCupsConnection::global(), &KCupsConnection::printerStateChanged, this, &MarkerLevelChecker::checkMarkerLevels);
}

MarkerLevelChecker::~MarkerLevelChecker()
{
}

void MarkerLevelChecker::checkMarkerLevels(const QString &text,
                                           const QString &printerUri,
                                           const QString &printerName,
                                           uint printerState,
                                           const QString &printerStateReasons,
                                           bool printerIsAcceptingJobs)
{
    static const QStringList attrs({KCUPS_MARKER_LEVELS,
                                    QLatin1String(KCUPS_MARKER_HIGH_LEVELS),
                                    QLatin1String(KCUPS_MARKER_LOW_LEVELS),
                                    KCUPS_MARKER_CHANGE_TIME,
                                    KCUPS_MARKER_TYPES,
                                    KCUPS_PRINTER_INFO,
                                    KCUPS_PRINTER_MAKE_AND_MODEL,
                                    KCUPS_MARKER_NAMES,
                                    KCUPS_PRINTER_TYPE});

    static const int threshold = 15;

    qCDebug(PMKDED) << "checkMarkerLevels" << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    const auto req = new KCupsRequest;
    connect(req, &KCupsRequest::finished, req, &KCupsRequest::deleteLater);
    connect(req, &KCupsRequest::deviceMap, this, [](const QVariantMap &map) {
        qCDebug(PMKDED) << map;

        // Class will not have marker level attributes
        if (map.value(KCUPS_PRINTER_TYPE).toInt() & CUPS_PRINTER_CLASS) {
            return;
        }

        auto notify = new KNotification(QLatin1String("MarkerLevel"));
        notify->setComponentName(QLatin1String("printmanager"));
        notify->setIconName(QLatin1String("printer"));
        notify->setTitle(map.value(KCUPS_PRINTER_INFO).toString());

        const auto ml = map.value(KCUPS_MARKER_LEVELS).toList();
        const auto hl = map.value(QLatin1String(KCUPS_MARKER_HIGH_LEVELS)).toList();
        const auto millis = map.value(KCUPS_MARKER_CHANGE_TIME, 0).toInt() * 1000;
        if (ml.isEmpty() || hl.isEmpty() || millis == 0) {
            // TODO: Demote to (NOOP) qCWarning when merged?
            notify->setText(i18n("Unable to get Ink Levels.  Levels may not be available until after the first print job has finished."));
            notify->sendEvent();
        } else {
            qCDebug(PMKDED) << "Checking Marker Levels against low threshold" << threshold;
            bool foundLow = false;
            int lowIndex = -1;
            int lowValue = 0;
            for (uint i = 0; i < ml.count(); ++i) {
                int level = ml.at(i).value<int>();
                int high = hl.at(i).value<int>();
                auto result = div(level * 100, high);
                if (result.quot <= threshold) {
                    foundLow = true;
                    lowIndex = i;
                    lowValue = result.quot;
                    break;
                }
            }
            if (foundLow) {
                const auto changeTime = QDateTime::fromMSecsSinceEpoch(millis + QDateTime::currentMSecsSinceEpoch());
                const auto name = map.value(KCUPS_MARKER_NAMES).toStringList().at(lowIndex);
                const auto type = map.value(KCUPS_MARKER_TYPES).toStringList().at(lowIndex);
                notify->setText(
                    i18n("At least one cartridge level is low\n%2 (%3)\nLevel: %4\nEst. Change Date: %1", changeTime.toString(), name, type, lowValue));
                notify->sendEvent();
            } else {
                qCDebug(PMKDED) << "All Marker Levels above low threshold" << threshold;
                notify->deleteLater();
            }
        }
    });

    req->getPrinterAttributesNotify(printerName, false, attrs);
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
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    checkMarkerLevels(text, printerUri, printerName, printerState, printerStateReasons, printerIsAcceptingJobs);
}

#include "moc_MarkerLevelChecker.cpp"
