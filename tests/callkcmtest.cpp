// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QTest>

#include <KIO/CommandLauncherJob>

#include <ProcessRunner.h>

using namespace Qt::Literals::StringLiterals;

class CallKCMTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAddPrinter()
    {
        ProcessRunner::addPrinter();
    }

    void testAddClass()
    {
        ProcessRunner::addClass();
    }

    void testConfigurePrinter()
    {
        ProcessRunner::configurePrinter("test"_L1);
    }

    void testOpenQueue()
    {
        ProcessRunner::openPrintQueue("test"_L1);
    }

    void testOpenKCM()
    {
        // ProcessRunner::openKCM();
    }

    void testOpenKCMWithKCMShell()
    {
        const auto job = new KIO::CommandLauncherJob("kcmshell6"_L1, {"kcm_printer_manager"_L1});
        job->start();
    }
};

QTEST_MAIN(CallKCMTest)
#include "callkcmtest.moc"
