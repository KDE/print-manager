/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QTest>

#include <KCupsConnection.h>

using namespace Qt::Literals::StringLiterals;

class SubscriptionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testConnection()
    {
        const auto conn = KCupsConnection::global();
        QVERIFY(conn);
        const auto handler = []() {
            return;
        };

#ifdef LIBCUPS_VERSION_2
        QVERIFY(connect(conn, &KCupsConnection::serverAudit, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::serverRestarted, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::serverStarted, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::serverStopped, this, handler));
#endif
        QVERIFY(connect(conn, &KCupsConnection::jobState, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::jobCreated, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::jobStopped, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::jobCompleted, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::jobConfigChanged, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::jobProgress, this, handler));

        QVERIFY(connect(conn, &KCupsConnection::printerFinishingsChanged, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerMediaChanged, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerStateChanged, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerAdded, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerDeleted, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerModified, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerShutdown, this, handler));
        QVERIFY(connect(conn, &KCupsConnection::printerRestarted, this, handler));
    }
};

QTEST_MAIN(SubscriptionTest)
#include "subscriptiontest.moc"
