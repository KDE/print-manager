// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <KCupsRequest.h>

class CupsServerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCUPSServer()
    {
        const auto request = new KCupsRequest();
        connect(request, &KCupsRequest::finished, this, [](KCupsRequest *r) {
            bool success = false;
            if (r->hasError() && r->error() != IPP_NOT_FOUND) {
                // success = false;
            } else {
                KCupsServer server = r->serverSettings();
                if (!server.arguments().isEmpty()) {
                    success = true;
                }
            }
            r->deleteLater();
            QCOMPARE(success, true);
        });

        request->getServerSettings();

        QSignalSpy spy(request, &KCupsRequest::finished);
        QVERIFY(spy.isValid());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(CupsServerTest)
#include "servertest.moc"
