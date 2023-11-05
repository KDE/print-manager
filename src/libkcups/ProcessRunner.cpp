/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessRunner.h"
#include <KIO/CommandLauncherJob>

using namespace Qt::StringLiterals;

ProcessRunner::ProcessRunner(QObject* parent) : QObject(parent)
{
}

void ProcessRunner::configurePrinter(const QString& printerName)
{
    exec(u"configure-printer"_s, {printerName});
}

void ProcessRunner::openPrintQueue(const QString& printerName)
{
    exec(u"kde-print-queue"_s, {printerName});
}

void ProcessRunner::addPrinter()
{
    exec(u"kde-add-printer"_s, {u"--add-printer"_s});
}

void ProcessRunner::addClass()
{
    exec(u"kde-add-printer"_s, {u"--add-class"_s});
}

void ProcessRunner::changePrinterPPD(const QString &printerName)
{
    exec(u"kde-add-printer"_s, {u"--change-ppd"_s, printerName});
}

void ProcessRunner::exec(const QString &cmd, const QStringList &args)
{
    auto job = new KIO::CommandLauncherJob(cmd, args);
    job->start();
}

#include "moc_ProcessRunner.cpp"
