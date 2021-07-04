/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H

#include <QObject>

class Q_DECL_EXPORT ProcessRunner : public QObject
{
    Q_OBJECT
public:
    explicit ProcessRunner(QObject *parent = nullptr);

public Q_SLOTS:
    void configurePrinter(const QString &printerName);
    void openPrintQueue(const QString &printerName);
    void openPrintKCM();
};

#endif // PROCESS_RUNNER_H
