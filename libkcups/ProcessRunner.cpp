/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessRunner.h"

#include <QProcess>

ProcessRunner::ProcessRunner(QObject* parent)
{
    Q_UNUSED(parent);
}

void ProcessRunner::configurePrinter(const QString& printerName)
{
    QProcess::startDetached(QLatin1String("configure-printer"), {printerName});
}

void ProcessRunner::openPrintQueue(const QString& printerName)
{
    QProcess::startDetached(QLatin1String("kde-print-queue"), {printerName});
}

void ProcessRunner::openPrintKCM()
{
    QProcess::startDetached(QLatin1String("kcmshell5"), {QLatin1String("kcm_printer_manager")});
}

#include "moc_ProcessRunner.cpp"
