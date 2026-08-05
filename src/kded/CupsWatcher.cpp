/*
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CupsWatcher.h"
#include "pmkded_log.h"

#include <KLocalizedString>
#include <KNotification>

#include <CommandHelpers.h>
#include <KCupsRequest.h>
#include <ProcessRunner.h>

using namespace Qt::StringLiterals;

CupsWatcher::CupsWatcher(QObject *parent)
    : QObject(parent)
{
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &CupsWatcher::jobCreated);
    connect(KCupsConnection::global(), &KCupsConnection::jobProgress, this, &CupsWatcher::jobProgress);

    /**
     * Handler for when a job completes successfully.  Even if the job is cancelled
     * we'll receive this signal.  Either way, remove the job from the notified list
     * if it was notified previously.
     */
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
                const auto cnt = m_notifiedJobIds.removeAll(jobId);
                qCDebug(PMKDED) << "jobCompleted:" << jobStateReasons << jobId << "Removed from notify list:" << cnt;
            });
}

void CupsWatcher::notifyPrinterStatus(const KCupsPrinter &printer, const QString reason)
{
    auto notify = new KNotification(u"StatusWarning"_s);
    notify->setComponentName(u"printmanager"_s);
    notify->setTitle(printer.info());
    notify->setText(
        i18nc("@info:status %1 is the reason message for the status", "%1\nThe print job will complete when the printer becomes available.", reason));

    // We can allow the user to resume the printer if it is paused/stopped
    if (printer.state() == KCupsPrinter::Stopped) {
        auto defAction = notify->addAction(i18nc("@action:button Prompt to resume printer", "Resume Printer"));
        connect(defAction, &KNotificationAction::activated, this, [printer]() {
            PrinterCommands printerCmd;
            printerCmd.resumePrinter(printer.name());
        });
    }
    notify->sendEvent();
    qCDebug(PMKDED) << "Sending status event notification:" << printer.name() << reason;
}

void CupsWatcher::checkMarkerLevels(const KCupsPrinter &printer)
{
    // Check if a marker notification is open for the printer
    if (m_openNotifications.contains(printer.name())) {
        qCDebug(PMKDED) << "Marker notification currently open for printer:" << printer.name();
        return;
    }

    const auto msgs = printer.checkMarkerLevels();
    if (!msgs.isEmpty()) {
        auto notify = new KNotification(u"MarkerLevel"_s, KNotification::Persistent);
        notify->setComponentName(u"printmanager"_s);
        notify->setTitle(printer.info());
        notify->setText(msgs.join(u"\n"_s));
        // When notification closes, remove the list entry
        QObject::connect(notify, &KNotification::closed, this, [this, printer]() {
            m_openNotifications.removeAll(printer.name());
            qCDebug(PMKDED) << "Marker notification closed for printer:" << printer.name();
        });

        auto checkMarkers = notify->addDefaultAction(i18nc("@action:button check printer ink levels", "Check Levels…"));
        connect(checkMarkers, &KNotificationAction::activated, this, [printer, notify]() {
            ProcessRunner::kcmConfigurePrinter(printer.name(), notify->xdgActivationToken().toUtf8());
        });

        notify->sendEvent();
        // Log this printer has an open notification
        m_openNotifications << printer.name();
    }
}

void CupsWatcher::jobCreated([[maybe_unused]] const QString &text,
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
    qCDebug(PMKDED) << "jobCreated:" << text << printerName << jobId << jobStateReasons;
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
    connect(request, &KCupsRequest::finished, this, [this, text](KCupsRequest *req) {
        if (!req->printers().isEmpty()) {
            const auto printer = req->printers().constFirst();
            /**
             * If a job is created the printer is most likely available and
             * accepting print job requests; however, it could be simply paused/stopped.
             */
            const auto reason = printer.checkNotAvailable(text);
            if (!reason.isEmpty()) {
                notifyPrinterStatus(printer, reason);
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

/**
 * This signal implies CUPS is trying to complete a print job and will be received
 * if the printer is available or not, possibly many times per job.
 *
 * If we've already notified on a job, don't notify again, otherwise, check available.
 */
void CupsWatcher::jobProgress(const QString &text,
                              [[maybe_unused]] const QString &printerUri,
                              const QString &printerName,
                              uint printerState,
                              const QString &printerStateReasons,
                              bool printerIsAcceptingJobs,
                              uint jobId,
                              [[maybe_unused]] uint jobState,
                              const QString &jobStateReasons,
                              [[maybe_unused]] const QString &jobName,
                              [[maybe_unused]] uint jobImpressionsCompleted)
{
    qCDebug(PMKDED) << "jobProgress:" << text << printerName << jobId << printerState << printerStateReasons << jobState << jobStateReasons;

    if (m_notifiedJobIds.contains(jobId)) {
        qCDebug(PMKDED) << jobId << "JobID has already triggered a notify event, ignoring";
        return;
    }

    // Create a mock printer based on the params from the signal
    KCupsPrinter printer{{{KCUPS_PRINTER_INFO, printerName},
                          {KCUPS_PRINTER_NAME, printerName},
                          {KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL},
                          {KCUPS_PRINTER_STATE, printerState},
                          {KCUPS_PRINTER_STATE_MESSAGE, printerStateReasons},
                          {KCUPS_PRINTER_IS_ACCEPTING_JOBS, printerIsAcceptingJobs}}};

    const auto reason = printer.checkNotAvailable(text);
    if (!reason.isEmpty()) {
        notifyPrinterStatus(printer, reason);
        m_notifiedJobIds << jobId;
        qCDebug(PMKDED) << jobId << "Setting jobID listed as notified";
    }
}

#include "moc_CupsWatcher.cpp"
