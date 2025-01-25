// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <DevicesModel.h>

class ModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDevicesModel()
    {
        DevicesModel devices;
        devices.update();

        QSignalSpy spy(&devices, &DevicesModel::loaded);
        QVERIFY(spy.isValid());
        spy.wait(20000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(devices.rowCount() > 0);
    }
};

QTEST_MAIN(ModelTest)
#include "devicemodeltests.moc"
