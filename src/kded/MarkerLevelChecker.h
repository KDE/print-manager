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

    /**
     * @brief If CUPS stops/starts, the notify subscriptions will become stale.
     * CUPS scheduler stop/start is rare in a session, but can happen if server
     * side settings are changed or if the service is stopped/started manually.
     *
     * Therefore, we need to release the old connection and get a new one on a
     * CUPS start/restart.
     */
    void init();
    void setConnections();

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
