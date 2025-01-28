// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>

using namespace Qt::Literals::StringLiterals;

class KDedTests : public QObject
{
    Q_OBJECT

public:
    KDedTests(QObject *parent)
        : QObject(parent)
    {
    };

public Q_SLOTS:
    void getReady()
    {
        const auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                         u"/com/redhat/NewPrinterNotification"_s,
                                                         u"com.redhat.NewPrinterNotification"_s,
                                                         u"GetReady"_s);
        const auto reply = QDBusConnection::systemBus().call(call);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Failed:" << reply.errorName() << reply.errorMessage();
        }
        Q_EMIT done();
    }

    void goodNewPrinter()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({0, "test"_L1, "HP"_L1, "4520"_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Failed:" << reply.errorName() << reply.errorMessage();
        }
        Q_EMIT done();
    }

    void badNewPrinter()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({2, "test"_L1, "HP"_L1, "4520"_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Failed:" << reply.errorName() << reply.errorMessage();
        }
        Q_EMIT done();
    }

    void queueNotCreated()
    {
        auto call = QDBusMessage::createMethodCall(u"com.redhat.NewPrinterNotification"_s,
                                                   u"/com/redhat/NewPrinterNotification"_s,
                                                   u"com.redhat.NewPrinterNotification"_s,
                                                   u"NewPrinter"_s);
        call.setArguments({0, "ipps://test_printer"_L1, ""_L1, ""_L1, "Test Printer"_L1, ""_L1});

        const auto reply = QDBusConnection::systemBus().call(call);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Failed:" << reply.errorName() << reply.errorMessage();
        }
        Q_EMIT done();
    }

Q_SIGNALS:
    void done();
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Test CUPS DBUS printer notification interfaces"_s);

    QCommandLineOption getReady(u"r"_s, u"Get Ready (new device found)"_s);
    parser.addOption(getReady);
    QCommandLineOption goodNewPrinter(u"g"_s, u"New printer added/configured (good)"_s);
    parser.addOption(goodNewPrinter);
    QCommandLineOption badNewPrinter(u"b"_s, u"New printer added/configured (bad)"_s);
    parser.addOption(badNewPrinter);
    QCommandLineOption failedQueue(u"q"_s, u"Printer queue creation failed"_s);
    parser.addOption(failedQueue);

    parser.process(app);

    const auto kded = new KDedTests(qApp);
    QObject::connect(kded, &KDedTests::done, &app, &QCoreApplication::quit, Qt::QueuedConnection);

    if (parser.isSet(getReady)) {
        kded->getReady();
    } else if (parser.isSet(goodNewPrinter)) {
        kded->goodNewPrinter();
    } else if (parser.isSet(badNewPrinter)) {
        kded->badNewPrinter();
    } else if (parser.isSet(failedQueue)) {
        kded->queueNotCreated();
    } else {
        parser.showHelp();
    }

    return app.exec();
}

#include "kdedtests.moc"
