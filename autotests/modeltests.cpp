// SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <DevicesModel.h>
#include <JobModel.h>
#include <PPDModel.h>
#include <PrinterModel.h>

using namespace Qt::Literals::StringLiterals;

class ModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

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
        JobModel m_jobs;
        m_jobs.init("test"_L1);
    }

    void testPPDModel()
    {
        PPDModel ppds;
        QObject::connect(&ppds, &PPDModel::error, this, [](const QString &m) {
            qDebug() << "Error from PPDModel:" << m;
        });

        ppds.load();
        QSignalSpy spy(&ppds, &PPDModel::loaded);
        QVERIFY(spy.isValid());
        spy.wait(20000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(ppds.rowCount() > 0);
    }

    // void testDevicesModel()
    // {
    //     DevicesModel devices;
    //     devices.update();

    //     QSignalSpy spy(&devices, &DevicesModel::loaded);
    //     QVERIFY(spy.isValid());
    //     spy.wait(20000);
    //     QCOMPARE(spy.count(), 1);
    //     QVERIFY(devices.rowCount() > 0);
    // }
};

QTEST_MAIN(ModelTest)
#include "modeltests.moc"
