/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H

#include <QObject>
#include <qqmlregistration.h>
#include <kcupslib_export.h>

class KCUPSLIB_EXPORT ProcessRunner : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit ProcessRunner(QObject *parent = nullptr);

public Q_SLOTS:
    static void configurePrinter(const QString &printerName);
    static void openPrintQueue(const QString &printerName);

    static void addPrinter();
    static void addClass();
    static void changePrinterPPD(const QString &printerName);

private:
    static void exec(const QString &cmd, const QStringList &args);
};

#endif // PROCESS_RUNNER_H
