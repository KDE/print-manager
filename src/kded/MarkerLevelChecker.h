/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

/**
 * Check for at least one marker level being below the threshold.
 *
 * CUPS only updates printers.conf after the first print
 * job is created/sent, so marker-* attributes will not exist until that point.
 * For a newly added printer queue or discovered printer, this will be a noop.
 *
 * Assumes levels and thresholds are percentages.
 * see https://openprinting.github.io/cups/doc/spec-ipp.html#marker-high-levels
 */
class MarkerLevelChecker : public QObject
{
    Q_OBJECT

public:
    explicit MarkerLevelChecker(QObject *parent);
    ~MarkerLevelChecker() = default;

private:
    void checkMarkerLevels(const QString &printerName);

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
};
