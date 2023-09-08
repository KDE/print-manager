/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessRunner.h"
#include <KIO/CommandLauncherJob>

using namespace Qt::StringLiterals;

ProcessRunner::ProcessRunner(QObject* parent)
{
    Q_UNUSED(parent);
}

void ProcessRunner::configurePrinter(const QString& printerName)
{
    exec("configure-printer"_L1, {printerName});
}

void ProcessRunner::openPrintQueue(const QString& printerName)
{
    exec("kde-print-queue"_L1, {printerName});
}

void ProcessRunner::addPrinter()
{
    exec("kde-add-printer"_L1, {"--add-printer"_L1});
}

void ProcessRunner::exec(const QString &cmd, const QStringList &args)
{
    auto job = new KIO::CommandLauncherJob(cmd, args);
    job->start();
}

#include "moc_ProcessRunner.cpp"
