// SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <CommandHelpers.h>
#include <JobModel.h>
#include <PPDModel.h>
#include <PrinterModel.h>

#include <KCupsRequest.h>

using namespace Qt::Literals::StringLiterals;

constexpr QLatin1String TEST_PRINTER = "test"_L1;

class ModelTest : public QObject
{
    Q_OBJECT

private:
    bool m_testPrinterFound = false;

private Q_SLOTS:

    void initTestCase()
    {
        const auto req = new KCupsRequest();
        req->getPrinterAttributes(TEST_PRINTER,
                                  false,
                                  {KCUPS_PRINTER_NAME,
                                   KCUPS_PRINTER_STATE,
                                   KCUPS_PRINTER_STATE_MESSAGE,
                                   KCUPS_PRINTER_IS_SHARED,
                                   KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                                   KCUPS_PRINTER_TYPE,
                                   KCUPS_PRINTER_LOCATION,
                                   KCUPS_PRINTER_INFO,
                                   KCUPS_PRINTER_MAKE_AND_MODEL,
                                   KCUPS_PRINTER_COMMANDS,
                                   KCUPS_DEVICE_URI,
                                   KCUPS_PRINTER_URI_SUPPORTED,
                                   KCUPS_MEMBER_NAMES});
        req->waitTillFinished();
        m_testPrinterFound = !req->printers().isEmpty();
        req->deleteLater();
    }

    void testPrinterCommand()
    {
        if (!m_testPrinterFound) {
            QSKIP("Test Printer is not configured");
        }

        QObject::connect(PrinterCommands::instance(), &PrinterCommands::testDone, this, []() {
            qDebug() << "Print Test Page Done";
        });
        QObject::connect(PrinterCommands::instance(), &PrinterCommands::error, this, [](int, const QString &, const QString &msg) {
            qDebug() << "Error from CommandHelpers:" << msg;
        });

        PrinterCommands::instance()->printTestPage(TEST_PRINTER, false);
        QSignalSpy spy(PrinterCommands::instance(), &PrinterCommands::testDone);
        QVERIFY(spy.isValid());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    void testPrinterModel()
    {
        PrinterModel printers;
        QObject::connect(&printers, &PrinterModel::error, this, [](int err, const QString, const QString) {
            QCOMPARE(err, 0); // err == 0 on success
        });

        QSignalSpy spy(&printers, &PrinterModel::error);
        QVERIFY(spy.isValid());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    void testJobModel()
    {
        JobModel jobs;
        bool loaded = false;
        QObject::connect(&jobs, &JobModel::loaded, this, [&loaded]() {
            loaded = true;
        });
        QObject::connect(&jobs, &JobModel::error, this, [](int, const QString &, const QString &msg) {
            qDebug() << "Error from JobModel:" << msg;
        });
        QSignalSpy spy(&jobs, &JobModel::loaded);
        QVERIFY(spy.isValid());
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(loaded);
    }

#ifdef LIBCUPS_VERSION_2
    void testPPDModel()
    {
        PPDModel ppds;
        QObject::connect(&ppds, &PPDModel::error, this, [](const QString &m) {
            qDebug() << m;
            QFAIL("Error from PPDModel");
        });

        ppds.load();
        QSignalSpy spy(&ppds, &PPDModel::error);
        QVERIFY(spy.isValid());
        spy.wait(5000);
    }
#endif
};

QTEST_MAIN(ModelTest)
#include "modeltests.moc"
