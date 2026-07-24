/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QTest>

#include <KCupsPrinter.h>

using namespace Qt::Literals::StringLiterals;

class AttributeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPrinterAttributes()
    {
        // An idle, available printer
        KCupsPrinter printer{{{KCUPS_PRINTER_NAME, u"test"_s},
                              {KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL},
                              {KCUPS_PRINTER_STATE, KCupsPrinter::Idle},
                              {KCUPS_PRINTER_STATE_MESSAGE, u"none"_s},
                              {KCUPS_PRINTER_IS_ACCEPTING_JOBS, true},
                              {KCUPS_PRINTER_IS_SHARED, false}}};

        // Test printer name can't be changed
        printer.setAttribute(KCUPS_PRINTER_NAME, u"another-test-name"_s);
        QVERIFY(printer.name() == u"test"_s);

        // Test printer type can't be changed
        printer.setAttribute(KCUPS_PRINTER_TYPE, CUPS_PRINTER_DISCOVERED);
        QVERIFY(printer.type() == CUPS_PRINTER_LOCAL);

        // Test (enum) printer state can be changed
        printer.setAttribute(KCUPS_PRINTER_STATE, KCupsPrinter::Stopped);
        QVERIFY(printer.state() == KCupsPrinter::Stopped);

        // Test (bool) printer shared can be changed
        printer.setAttribute(KCUPS_PRINTER_IS_SHARED, true);
        QVERIFY(printer.isShared());

        // Test (string) printer state msg can be changed
        printer.setAttribute(KCUPS_PRINTER_STATE_MESSAGE, u"A new state message"_s);
        QVERIFY(printer.stateMsg() != u"none"_s);
    }
};

QTEST_MAIN(AttributeTest)
#include "attributetest.moc"
