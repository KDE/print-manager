// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>

#include <KIO/CommandLauncherJob>

#include <ProcessRunner.h>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Test calling the KCM for printer config tasks"_s);

    parser.addPositionalArgument(u"printer"_s, u"Printer name (required for configure and open queue, defaults to \"test\")"_s);

    QCommandLineOption addPrinter(u"p"_s, u"Add a new printer"_s);
    parser.addOption(addPrinter);
    QCommandLineOption addClass(u"g"_s, u"Add a new printer group"_s);
    parser.addOption(addClass);

    QCommandLineOption configurePrinter(u"c"_s, u"Configure printer"_s);
    parser.addOption(configurePrinter);
    QCommandLineOption configureMedia(u"m"_s, u"Configure printer media settings"_s);
    parser.addOption(configureMedia);

    QCommandLineOption openQueue(u"q"_s, u"Open printer queue"_s);
    parser.addOption(openQueue);

    QCommandLineOption openKcm(u"k"_s, u"Open printer manager KCM"_s);
    parser.addOption(openKcm);
    QCommandLineOption openKcmWithShell(u"s"_s, u"Open printer manager KCM with the Shell"_s);
    parser.addOption(openKcmWithShell);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QString printerName(u"test"_s);
    if (!args.isEmpty()) {
        printerName = args.first();
    }

    if (parser.isSet(addPrinter)) {
        ProcessRunner::addPrinter();
    } else if (parser.isSet(addClass)) {
        ProcessRunner::addClass();
    } else if (parser.isSet(configurePrinter)) {
        ProcessRunner::kcmConfigurePrinter(printerName);
    } else if (parser.isSet(configureMedia)) {
        ProcessRunner::configurePrinter(printerName);
    } else if (parser.isSet(openQueue)) {
        ProcessRunner::openPrintQueue(printerName);
    } else if (parser.isSet(openKcm)) {
        ProcessRunner::openKCM();
    } else if (parser.isSet(openKcmWithShell)) {
        const auto job = new KIO::CommandLauncherJob("kcmshell6"_L1, {"kcm_printer_manager"_L1});
        job->start();
    } else {
        parser.showHelp();
    }

    return app.exec();
}

#include "callkcmtests.moc"
