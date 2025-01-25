// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <JobModel.h>
#include <PPDModel.h>
#include <PrinterModel.h>

using namespace Qt::Literals::StringLiterals;

class ModelTest : public QObject
{
    Q_OBJECT

    // Set true to enable compare/verify
    // false will load the models, but not compare/verify
    bool m_enabled = false;

private Q_SLOTS:

    void testPrinterModel()
    {
        PrinterModel printers;
        QObject::connect(&printers, &PrinterModel::error, this, [this](int err, const QString &m, const QString &m1) {
            if (m_enabled) {
                QCOMPARE(err, 0); // err == 0 on success
            } else {
                qDebug() << "Response from PrinterModel:" << err << m1;
            }
        });

        QSignalSpy spy(&printers, &PrinterModel::error);
        QVERIFY(spy.isValid());
        spy.wait();
        if (m_enabled) {
            QCOMPARE(spy.count(), 1);
        }
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
        if (m_enabled) {
            QCOMPARE(spy.count(), 1);
            QVERIFY(ppds.rowCount() > 0);
        } else {
            qDebug() << "PPDs rowCount:" << ppds.rowCount();
        }
    }
};

QTEST_MAIN(ModelTest)
#include "modeltests.moc"
