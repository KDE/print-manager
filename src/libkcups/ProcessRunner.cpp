/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProcessRunner.h"
#include <KIO/CommandLauncherJob>

using namespace Qt::StringLiterals;

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent)
{
}

void ProcessRunner::configurePrinter(const QString &printerName)
{
    exec(u"configure-printer"_s, {printerName}, u"org.kde.ConfigurePrinter"_s);
}

void ProcessRunner::openPrintQueue(const QString &printerName)
{
    exec(u"kde-print-queue"_s, {printerName}, u"org.kde.PrintQueue"_s);
}

void ProcessRunner::exec(const QString &cmd, const QStringList &args, const QString &desktopFile)
{
    auto job = new KIO::CommandLauncherJob(cmd, args);
    if (!desktopFile.isEmpty()) {
        job->setDesktopName(desktopFile);
    }
    job->start();
}

#include "moc_ProcessRunner.cpp"
