/*
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CupsWatcher.h"
#include "pmkded_log.h"

#include <KLocalizedString>
#include <KNotification>

#include <KCupsRequest.h>
#include <ProcessRunner.h>

using namespace Qt::StringLiterals;

CupsWatcher::CupsWatcher(QObject *parent)
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

    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &CupsWatcher::jobHandler);
    connect(KCupsConnection::global(), &KCupsConnection::jobProgress, this, &CupsWatcher::jobProgress);
    // connect(KCupsConnection::global(),
    //         &KCupsConnection::printerStateChanged,
    //         this,
    //         [this]([[maybe_unused]] const QString &text,
    //                [[maybe_unused]] const QString &printerUri,
    //                [[maybe_unused]] const QString &printerName,
    //                [[maybe_unused]] uint printerState,
    //                [[maybe_unused]] const QString &printerStateReasons,
    //                [[maybe_unused]] bool printerIsAcceptingJobs) {
    //             // A printer going to idle implies that it's probably available.  In a multi-printer
    //             // setup, even with one printer changing status to idle, clearing the job ids list
    //             // for all will trigger a notification for each job CUPS is re-trying, which should
    //             // be the first job in each queue that's not available.
    //             if (printerState == KCupsPrinter::Idle) {
    //                 qCDebug(PMKDED) << "Clearing failed/paused jobs list:" << text << printerName << printerState << printerStateReasons;
    //                 m_notifiedJobIds.clear();
    //             }
    //         });

    connect(KCupsConnection::global(),
            &KCupsConnection::jobCompleted,
            this,
            [this]([[maybe_unused]] const QString &text,
                   [[maybe_unused]] const QString &printerUri,
                   [[maybe_unused]] const QString &printerName,
                   [[maybe_unused]] uint printerState,
                   [[maybe_unused]] const QString &printerStateReasons,
                   [[maybe_unused]] bool printerIsAcceptingJobs,
                   [[maybe_unused]] uint jobId,
                   [[maybe_unused]] uint jobState,
                   [[maybe_unused]] const QString &jobStateReasons,
                   [[maybe_unused]] const QString &jobName,
                   [[maybe_unused]] uint jobImpressionsCompleted) {
                // clear the jobid from the list
                const auto cnt = m_notifiedJobIds.removeAll(jobId);
                qWarning() << "JOBCOMPLETED:" << text << jobId << "Removed:" << cnt;
            });
}

void CupsWatcher::notifyPrinterStatus(const QString &printer, uint jobId, const QString reason)
{
    if (m_notifiedJobIds.contains(jobId)) {
        qCDebug(PMKDED) << jobId << "JobID has already triggered a notify event, ignoring";
        return;
    }

    auto notify = new KNotification(u"StatusWarning"_s);
    notify->setComponentName(u"printmanager"_s);
    notify->setTitle(printer);
    notify->setText(
        i18nc("@info:status %1 is the reason message for the status", "%1\nThe print job will complete when the printer becomes available.", reason));
    notify->sendEvent();
    m_notifiedJobIds << jobId;
}

void CupsWatcher::checkMarkerLevels(const KCupsPrinter &printer)
{
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

void CupsWatcher::jobHandler([[maybe_unused]] const QString &text,
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
                                      KCUPS_PRINTER_NAME,
                                      KCUPS_PRINTER_STATE,
                                      KCUPS_PRINTER_STATE_MESSAGE,
                                      KCUPS_PRINTER_INFO,
                                      KCUPS_PRINTER_TYPE});

    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, jobId, text](KCupsRequest *req) {
        if (!req->printers().isEmpty()) {
            const auto printer = req->printers().at(0);
            // Catch a paused or stopped printer (the job is created)
            const auto reason = printer.checkNotAvailable(text);
            if (!reason.isEmpty()) {
                notifyPrinterStatus(printer.info(), jobId, reason);
            }
            // Check markers
            checkMarkerLevels(printer);
        } else {
            // it's possible attributes from a temporary queue could be empty
            qCDebug(PMKDED) << "No printers found for checking";
        }
        req->deleteLater();
    });

    request->getPrinterAttributes(printerName, false, s_attrs);
}

void CupsWatcher::jobProgress(const QString &text,
                              [[maybe_unused]] const QString &printerUri,
                              const QString &printerName,
                              [[maybe_unused]] uint printerState,
                              const QString &printerStateReasons,
                              [[maybe_unused]] bool printerIsAcceptingJobs,
                              uint jobId,
                              [[maybe_unused]] uint jobState,
                              const QString &jobStateReasons,
                              [[maybe_unused]] const QString &jobName,
                              [[maybe_unused]] uint jobImpressionsCompleted)
{
    qCDebug(PMKDED) << "checkStatus:" << text << printerName << jobId << printerState << printerStateReasons << jobState << jobStateReasons;

    KCupsPrinter printer{{{KCUPS_PRINTER_NAME, printerName},
                          {KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL},
                          {KCUPS_PRINTER_STATE, printerState},
                          {KCUPS_PRINTER_STATE_MESSAGE, printerStateReasons},
                          {KCUPS_PRINTER_IS_ACCEPTING_JOBS, printerIsAcceptingJobs}}};

    const auto reason = printer.checkNotAvailable(text);
    if (!reason.isEmpty()) {
        notifyPrinterStatus(printerName, jobId, reason);
    }
}

#include "moc_CupsWatcher.cpp"
