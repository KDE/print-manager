/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

/**
 * Check for at least one marker level being below the threshold.
 *
 * CUPS appears to only update printers.conf after the first print
 * job is created/sent, so these will be empty until that point for a
 * new printer queue, which will be a noop.
 *
 * Assumes levels and thresholds are percentages.
 *
 * Maintains a \sa KCupsConnection for the lifetime.
 */
class MarkerLevelChecker : public QObject
{
    Q_OBJECT

public:
    explicit MarkerLevelChecker(QObject *parent);
    ~MarkerLevelChecker() override;

private:
    void checkMarkerLevels(const QString &printerName);
    /**
     * Generic handler for job signals
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
    /**
     * Generic handler for printer signals
     */
    void printerHandler(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
};
