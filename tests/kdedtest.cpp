// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDBusConnection>
#include <QDBusMessage>
#include <QObject>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class KdedTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGetReady()
    {
        const auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                         u"/com/redhat/NewPrinterNotification"_s,
                                                         u"com.redhat.NewPrinterNotification"_s,
                                                         u"GetReady"_s);
        const auto reply = QDBusConnection::systemBus().call(call);
        QCOMPARE(reply.type(), QDBusMessage::ReplyMessage);
    }

    void testGoodNewPrinter()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({0, "test"_L1, "HP"_L1, "4520"_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        QCOMPARE(reply.type(), QDBusMessage::ReplyMessage);
    }

    void testBadNewPrinter()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({2, "test"_L1, "HP"_L1, "4520"_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        QCOMPARE(reply.type(), QDBusMessage::ReplyMessage);
    }

    void testQueueNotCreated()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({0, "ipps://test_printer"_L1, ""_L1, ""_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        QCOMPARE(reply.type(), QDBusMessage::ReplyMessage);
    }
};

QTEST_MAIN(KdedTest)
#include "kdedtest.moc"
