/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "TestHelpers.h"
#include <CommandHelpers.h>

using namespace Qt::Literals::StringLiterals;

class CommandTest : public QObject
{
    Q_OBJECT

private:
    TestHelpers helpers;
    PrinterCommands commands;
    QString m_testPrinterName;
    bool m_printersExist = false;

private Q_SLOTS:

    void initTestCase()
    {
        // try to find the test printer
        const auto request = helpers.getPrinterRequest();
        connect(request, &KCupsRequest::finished, this, [this](KCupsRequest *req) {
            if (!req->printers().isEmpty()) {
                m_testPrinterName = req->printers().at(0).name();
                qDebug() << "Test printer found:" << m_testPrinterName;
            }
            req->deleteLater();
        });

        QSignalSpy spy(request, &KCupsRequest::finished);
        QVERIFY(spy.isValid());
        spy.wait();
        QCOMPARE(spy.count(), 1);

        // see if we have any available printers configured
        const auto request2 = helpers.getPrinterListRequest();
        connect(request2, &KCupsRequest::finished, this, [this](KCupsRequest *req) {
            if (req->hasError()) {
                qDebug() << req->error() << req->serverError() << req->errorMsg();
            } else {
                qDebug() << "Printers exist and are availble";
                m_printersExist = true;
            }
            req->deleteLater();
        });

        QSignalSpy spy2(request2, &KCupsRequest::finished);
        QVERIFY(spy2.isValid());
        spy2.wait();
        QCOMPARE(spy2.count(), 1);
    }

    void testPrintTestPage()
    {
        if (m_testPrinterName.isEmpty()) {
            QSKIP("Test Printer is not configured");
        } else {
            QObject::connect(&commands, &PrinterCommands::testDone, this, []() {
                qDebug() << "Print Test Page Done";
            });
            QObject::connect(&commands, &PrinterCommands::error, this, [](int, const QString &, const QString &msg) {
                qDebug() << "Error from CommandHelpers:" << msg;
            });

            commands.printTestPage(m_testPrinterName, false);
            QSignalSpy spy(&commands, &PrinterCommands::testDone);
            QVERIFY(spy.isValid());
            spy.wait();
            QCOMPARE(spy.count(), 1);
        }
    }

    void testJobsList()
    {
        if (!m_printersExist) {
            QSKIP("There are no printers configured for jobs list testing");
        } else {
            const auto request = new KCupsRequest;
            connect(request, &KCupsRequest::finished, this, [](KCupsRequest *req) {
                req->deleteLater();
                if (req->hasError()) {
                    qDebug() << req->error() << req->serverError() << req->errorMsg();
                    QFAIL("Unable to get Jobs List");
                }
            });
            request->getJobs(QString(),
                             false,
                             -1,
                             {KCUPS_JOB_ID,
                              KCUPS_JOB_NAME,
                              KCUPS_JOB_K_OCTETS,
                              KCUPS_JOB_K_OCTETS_PROCESSED,
                              KCUPS_JOB_STATE,
                              KCUPS_JOB_STATE_REASONS,
                              KCUPS_JOB_HOLD_UNTIL,
                              KCUPS_TIME_AT_COMPLETED,
                              KCUPS_TIME_AT_CREATION,
                              KCUPS_TIME_AT_PROCESSING,
                              KCUPS_JOB_PRINTER_URI,
                              KCUPS_JOB_ORIGINATING_USER_NAME,
                              KCUPS_JOB_ORIGINATING_HOST_NAME,
                              KCUPS_JOB_MEDIA_PROGRESS,
                              KCUPS_JOB_MEDIA_SHEETS,
                              KCUPS_JOB_MEDIA_SHEETS_COMPLETED,
                              KCUPS_JOB_PRINTER_STATE_MESSAGE,
                              KCUPS_JOB_PRESERVED});

            QSignalSpy spy(request, &KCupsRequest::finished);
            QVERIFY(spy.isValid());
            spy.wait();
            QCOMPARE(spy.count(), 1);
        }
    }
};

QTEST_MAIN(CommandTest)
#include "commandtest.moc"
