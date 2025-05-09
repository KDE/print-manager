/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointer>

/**
 * Check for at least one marker level being below the threshold.
 *
 * CUPS only updates printers.conf after the first print
 * job is created/sent, so marker-* attributes will not exist until that point.
 * For a newly added printer queue or discovered printer, this will be a noop.
 *
 * Assumes levels and thresholds are percentages.
 * see https://openprinting.github.io/cups/doc/spec-ipp.html#marker-high-levels
 *
 * Maintains a \sa KCupsConnection for the lifetime.
 */
class KCupsConnection;
class MarkerLevelChecker : public QObject
{
    Q_OBJECT

public:
    explicit MarkerLevelChecker(QObject *parent);
    ~MarkerLevelChecker() override;

private:
    QPointer<KCupsConnection> m_connection;

    void checkMarkerLevels(const QString &printerName);

    void init();
    /**
     * @brief Force a new cups connection.  When the cups scheduler
     * stops the subscriptions for notify appear to be in a unknown
     * state - sometimes working, sometimes not.
     *
     * Forcing a new connection guarantees valid subscriptions and
     * prevents the checker from errantly issuing a request on a
     * potentially stale connection should the CUPS scheduler be
     * stopped.
     *
     * @param reconnect true forces the new connection
     */
    void reset(bool reconnect = true);
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
    /**
     * @brief Generic handler for printer signals
     */
    void printerHandler(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
};
