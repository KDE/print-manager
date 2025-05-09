/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class MarkerLevelChecker : public QObject
{
    Q_OBJECT

public:
    explicit MarkerLevelChecker(QObject *parent);
    ~MarkerLevelChecker() override;

private:
    void checkMarkerLevels(const QString &text,
                           const QString &printerUri,
                           const QString &printerName,
                           uint printerState,
                           const QString &printerStateReasons,
                           bool printerIsAcceptingJobs);
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
