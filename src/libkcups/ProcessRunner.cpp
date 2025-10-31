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

// call legacy apps
void ProcessRunner::configurePrinter(const QString &printerName)
{
    exec(u"configure-printer"_s, {printerName}, u"org.kde.ConfigurePrinter"_s);
}

void ProcessRunner::openPrintQueue(const QString &printerName)
{
    exec(u"plasma-print-queue"_s, {printerName}, u"org.kde.plasma.printqueue"_s);
}

// call kcm
void ProcessRunner::addPrinter()
{
    openKCM({u"--add-printer"_s});
}

void ProcessRunner::addClass()
{
    openKCM({u"--add-group"_s});
}

void ProcessRunner::kcmConfigurePrinter(const QString &printerName)
{
    openKCM({u"--configure-printer"_s, printerName});
}

void ProcessRunner::openKCM(const QStringList &args)
{
    // The desktop filename is the same as the binary and icon
    const QString systemSettings = u"systemsettings"_s;

    QStringList cmdline{u"kcm_printer_manager"_s};
    if (!args.isEmpty()) {
        cmdline.append(u"--args"_s);
        cmdline.append(args.join(QLatin1Char(' ')));
    }

    // In Plasma, so assume System Settings is available
    exec(systemSettings, cmdline, systemSettings);
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
