/*
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class KCupsPrinter;
/**
 * Perform marker level checking and printer status checking
 * when a job is created.
 *
 * For marker levels, CUPS only updates printers.conf after the first print
 * job is created/sent, so marker-* attributes will not exist until that point.
 * For a newly added printer queue or discovered printer, this will be a noop.
 *
 * Assumes levels and thresholds are percentages.
 * see https://openprinting.github.io/cups/doc/spec-ipp.html#marker-high-levels
 *
 * For printer status checking, when a printer is paused or not accepting jobs
 * no "jobProgress" signals are sent so we can check when the job is created.
 *
 * If the printer is offline or cannot be located, we will know that via the
 * jobProgress signal and we can check status reasons against keywords.
 */
class CupsWatcher : public QObject
{
    Q_OBJECT

public:
    explicit CupsWatcher(QObject *parent);
    ~CupsWatcher() = default;

private:
    void checkMarkerLevels(const KCupsPrinter &printer);
    void notifyPrinterStatus(const QString &printer, uint jobId, const QString reason = QString());

    void jobProgress(const QString &text,
                     const QString &printerUri,
                     const QString &printerName,
                     uint printerState,
                     const QString &printerStateReasons,
                     bool printerIsAcceptingJobs,
                     uint jobId,
                     uint jobState,
                     const QString &jobStateReasons,
                     const QString &jobName,
                     uint jobImpressionsCompleted);
    /**
     * @brief Generic handler for job signals
     */
    void jobHandler(const QString &text,
                    const QString &printerUri,
                    const QString &printerName,
                    uint printerState,
                    const QString &printerStateReasons,
                    bool printerIsAcceptingJobs,
                    uint jobId,
                    uint jobState,
                    const QString &jobStateReasons,
                    const QString &jobName,
                    uint jobImpressionsCompleted);

    QList<uint> m_notifiedJobIds;
};
