/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QTest>

#include <KCupsPrinter.h>

using namespace Qt::Literals::StringLiterals;

class NotifyStatusTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNotifyStatus()
    {
        // An idle, available printer
        KCupsPrinter printer{{{KCUPS_PRINTER_NAME, u"notifytest"_s},
                              {KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL},
                              {KCUPS_PRINTER_STATE, KCupsPrinter::Idle},
                              {KCUPS_PRINTER_STATE_MESSAGE, u"none"_s},
                              {KCUPS_PRINTER_IS_ACCEPTING_JOBS, true}}};

        auto reason = printer.checkNotAvailable(u"test message"_s);
        // reason empty means the printer is available
        QVERIFY(reason.isEmpty());

        // A paused printer
        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"paused"_s);
        reason = printer.checkNotAvailable(u"test message"_s);
        QVERIFY(!reason.isEmpty());

        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"none"_s);
        printer.setAttribute(KCUPS_PRINTER_STATE, KCupsPrinter::Stopped);
        reason = printer.checkNotAvailable(u"test message"_s);
        QVERIFY(!reason.isEmpty());

        // An offline printer
        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"offline"_s);
        printer.setAttribute(KCUPS_PRINTER_STATE, KCupsPrinter::Idle);
        reason = printer.checkNotAvailable(u"test message"_s);
        QVERIFY(!reason.isEmpty());

        // A disconnected printer
        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"none"_s);
        reason = printer.checkNotAvailable(u"unavailable"_s);
        QVERIFY(!reason.isEmpty());

        // A not found printer
        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"none"_s);
        reason = printer.checkNotAvailable(u"unable to locate"_s);
        QVERIFY(!reason.isEmpty());
    }
};

QTEST_MAIN(NotifyStatusTest)
#include "notifystatustest.moc"
