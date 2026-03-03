/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

private:
    bool m_needAuth = true;

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

    void testDevicesModel()
    {
        if (!m_needAuth) {
            DevicesModel devices;
            QObject::connect(&devices, &DevicesModel::loaded, this, [&devices]() {
                if (devices.rowCount() <= 0) {
                    qDebug() << "No rows in Devices destinations model";
                }
            });

            devices.update();
            QSignalSpy spy(&devices, &DevicesModel::loaded);
            QVERIFY(spy.isValid());
            spy.wait(10000);
            QCOMPARE(spy.count(), 1);
        }
    }
#endif
};

QTEST_MAIN(ModelTest)
#include "modeltest.moc"
