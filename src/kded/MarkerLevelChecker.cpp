/*
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

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

void MarkerLevelChecker::checkMarkerLevels(KCupsRequest *request)
{
    // let's make sure to not leave dangling requests
    auto cleanupReq = qScopeGuard([request] {
        request->deleteLater();
    });

    // it's possible attributes from a temporary queue could be empty
    if (request->printers().isEmpty()) {
        qCDebug(PMKDED) << "No printers found for marker level check";
        return;
    }

    const auto printer = request->printers().at(0);
    const auto msgs = printer.checkMarkerLevels();

    if (!msgs.isEmpty()) {
        auto notify = new KNotification(u"MarkerLevel"_s, KNotification::Persistent);
        notify->setComponentName(u"printmanager"_s);
        notify->setTitle(printer.info());
        notify->setText(msgs.join(u"\n"_s));

        auto checkMarkers = notify->addDefaultAction(i18nc("@action:button check printer ink levels", "Check Levels…"));
        connect(checkMarkers, &KNotificationAction::activated, this, [pn = printer.name(), token = notify->xdgActivationToken().toUtf8()]() {
            ProcessRunner::kcmConfigurePrinter(pn, token);
        });

        notify->sendEvent();
    }
}

void MarkerLevelChecker::jobHandler([[maybe_unused]] const QString &text,
                                    [[maybe_unused]] const QString &printerUri,
                                    [[maybe_unused]] const QString &printerName,
                                    [[maybe_unused]] uint printerState,
                                    [[maybe_unused]] const QString &printerStateReasons,
                                    [[maybe_unused]] bool printerIsAcceptingJobs,
                                    [[maybe_unused]] uint jobId,
                                    [[maybe_unused]] uint jobState,
                                    [[maybe_unused]] const QString &jobStateReasons,
                                    [[maybe_unused]] const QString &jobName,
                                    [[maybe_unused]] uint jobImpressionsCompleted)
{
    qCDebug(PMKDED) << text << printerName << jobId << jobStateReasons;
    static const QStringList s_attrs({KCUPS_MARKER_NAMES,
                                      KCUPS_MARKER_LEVELS,
                                      KCUPS_MARKER_HIGH_LEVELS,
                                      KCUPS_MARKER_LOW_LEVELS,
                                      KCUPS_MARKER_TYPES,
                                      KCUPS_PRINTER_INFO,
                                      KCUPS_PRINTER_TYPE});

    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &MarkerLevelChecker::checkMarkerLevels);
    request->getPrinterAttributes(printerName, false, s_attrs);
}

#include "moc_MarkerLevelChecker.cpp"
